/*
 * \brief  Test for the repeated child creation in a dynamic init
 * \author Norman Feske
 * \date   2018-06-07
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#include <base/heap.h>
#include <base/component.h>
#include <base/attached_rom_dataspace.h>
#include <base/session_state.h>
#include <log_session/log_session.h>
#include <os/reporter.h>

namespace Test {
	struct Main;
	using namespace Genode;
}


struct Test::Main
{
	Env &_env;

	bool _client_starting = false;

	Expanding_reporter _init_config_reporter { _env, "config",  "init.config" };

	void _gen_log_server_start_content(Xml_generator &xml) const
	{
		xml.attribute("name", "server");
		xml.attribute("caps", "100");
		xml.node("resource", [&] () {
			xml.attribute("name", "RAM");
			xml.attribute("quantum", "1M"); });

		xml.node("binary", [&] () {
			xml.attribute("name", "dummy"); });

		xml.node("config", [&] () {
			xml.node("log_service", [&] () {}); });

		xml.node("provides", [&] () {
			xml.node("service", [&] () {
				xml.attribute("name", Log_session::service_name()); }); });

		xml.node("route", [&] () {
			xml.node("any-service", [&] () {
				xml.node("parent", [&] () {}); }); });
	}

	void _gen_log_client_start_content(Xml_generator &xml) const
	{
		xml.attribute("name", "client");
		xml.attribute("caps", "200");
		xml.node("resource", [&] () {
			xml.attribute("name", "RAM");
			xml.attribute("quantum", "1M"); });

		xml.node("binary", [&] () {
			xml.attribute("name", "dummy"); });

		xml.node("config", [&] () {
			xml.node("create_log_connections", [&] () {
				xml.attribute("count", 1);
				xml.attribute("ram_upgrade", "64K");
			});
			xml.node("log", [&] () {
				xml.attribute("string", "client started"); }); });

		xml.node("route", [&] () {

			xml.node("service", [&] () {
				xml.attribute("name", Log_session::service_name());
				xml.node("child", [&] () {
					xml.attribute("name", "server"); });
			});

			xml.node("any-service", [&] () {
				xml.node("parent", [&] () {}); });
		});
	}

	void _gen_init_config(Xml_generator &xml) const
	{
		xml.node("report", [&] () {
			xml.attribute("requested",  "yes");
			xml.attribute("provided",   "yes");
			xml.attribute("init_ram",   "yes");
			xml.attribute("init_caps",  "yes");
			xml.attribute("child_ram",  "yes");
			xml.attribute("child_caps", "yes");
			xml.attribute("delay_ms",    100);
		});

		auto gen_service = [&] (char const *name) {
			xml.node("service", [&] () { xml.attribute("name", name); }); };

		xml.node("parent-provides", [&] () {
			gen_service("ROM");
			gen_service("CPU");
			gen_service("PD");
			gen_service("RM");
			gen_service("LOG");
		});

		xml.node("start", [&] () {
			_gen_log_server_start_content(xml); });

		if (_client_starting)
			xml.node("start", [&] () {
				_gen_log_client_start_content(xml); });
	}

	void generate_init_config()
	{
		_init_config_reporter.generate([&] (Xml_generator &xml) {
			_gen_init_config(xml); });
	}

	/*
	 * Handling of state reports generated by init
	 */
	Attached_rom_dataspace _init_state { _env, "state" };

	Signal_handler<Main> _init_state_handler {
		_env.ep(), *this, &Main::_handle_init_state };

	/* counter for iterations */
	unsigned _cnt = 0;

	struct Ram_tracker
	{
		using Name = String<32>;

		Name const _name;

		/* RAM quota of previous iteration */
		size_t _previous = 0;

		size_t total_loss = 0;

		void update(Xml_node const &ram)
		{
			size_t const current = ram.attribute_value("quota", Number_of_bytes());

			log(_name, " RAM: ", Number_of_bytes(current));
			if (_previous) {

				if (current < _previous) {
					total_loss += _previous - current;
					log(_name, " lost ", _previous - current, " bytes", " "
					    "(total ", total_loss, " bytes)");
				}

				if (current > _previous)
					log(_name, " gained ", current - _previous, " bytes");
			}
			_previous = current;
		}

		Ram_tracker(Name const &name) : _name(name) { }
	};

	Ram_tracker _init_ram_tracker   { "init" };
	Ram_tracker _server_ram_tracker { "server" };

	static Number_of_bytes _init_ram(Xml_node const &state)
	{
		/* \throw Nonexistent_sub_node */
		return state.sub_node("ram").attribute_value("quota", Number_of_bytes());
	}

	using Name = String<32>;

	template <typename FN>
	void _apply_child(Xml_node const &state, Name const &name, FN const &fn)
	{
		state.for_each_sub_node("child", [&] (Xml_node const &child) {
			if (child.attribute_value("name", Name()) == name)
				fn(child); });
	}

	void _handle_init_state()
	{
		_init_state.update();

		Xml_node const &state = _init_state.xml();

		/*
		 * Detect state where the client is running and has established a
		 * session to the LOG server.
		 */
		bool client_present  = false;
		bool client_complete = false;
		_apply_child(state, "client", [&] (Xml_node const &child) {
			client_present = true;
			child.for_each_sub_node("requested", [&] (Xml_node const &requested) {
				requested.for_each_sub_node("session", [&] (Xml_node const &session) {
					if (session.attribute_value("service", String<16>()) == "LOG"
					 && session.attribute_value("state", String<16>()) == "CAP_HANDED_OUT")
						client_complete = true; }); }); });

		bool client_connected = false;
		_apply_child(state, "server", [&] (Xml_node const &child) {
			child.for_each_sub_node("provided", [&] (Xml_node const &provided) {
				client_connected |= (provided.num_sub_nodes() > 0); }); });

		if (_client_starting) {

			/* kill client as soon as it started up completely */
			if (client_complete)
				_client_starting = false;

		} else {

			/* restart client as soon as it vanished */
			if (!client_present && !client_connected) {

				_cnt++;
				log("iteration ", _cnt);

				if (state.has_sub_node("ram"))
					_init_ram_tracker.update(state.sub_node("ram"));

				_apply_child(state, "server", [&] (Xml_node const &child) {
					_server_ram_tracker.update(child.sub_node("ram")); });

				_client_starting = true;

				if (_init_ram_tracker.total_loss   > 16*1024
				 || _server_ram_tracker.total_loss > 16*1024) {

					error("unexpected quota distribution");
					_env.parent().exit(1);
				}
			}
		}

		/* success after 50 iterations without any accounting issues */
		if (_cnt == 50)
			_env.parent().exit(0);

		generate_init_config();
	}

	Main(Env &env) : _env(env)
	{
		_init_state.sigh(_init_state_handler);

		_handle_init_state();

		generate_init_config();
	}
};


void Component::construct(Genode::Env &env) { static Test::Main main(env); }

