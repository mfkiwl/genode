/*
 * \brief  Layout rules
 * \author Norman Feske
 * \date   2018-09-26
 */

/*
 * Copyright (C) 2018 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _LAYOUT_RULES_H_
#define _LAYOUT_RULES_H_

/* Genode includes */
#include <os/buffered_xml.h>

/* local includes */
#include <types.h>
#include <window.h>

namespace Window_layouter { class Layout_rules; }


class Window_layouter::Layout_rules : Noncopyable
{
	public:

		struct Action : Interface
		{
			virtual void layout_rules_changed() = 0;
		};

	private:

		Env &_env;

		Allocator &_alloc;

		Action &_action;

		Constructible<Buffered_xml> _config_rules { };

		struct Rom_rules
		{
			static char const *node_type() { return "rules"; }

			Attached_rom_dataspace rom;

			Action &_action;

			void _handle()
			{
				rom.update();
				_action.layout_rules_changed();
			}

			Signal_handler<Rom_rules> _handler;

			Rom_rules(Env &env, Action &action)
			:
				rom(env, node_type()), _action(action),
				_handler(env.ep(), *this, &Rom_rules::_handle)
			{ 
				rom.sigh(_handler);
				_handle();
			}
		};

		Constructible<Rom_rules> _rom_rules { };

	public:

		Layout_rules(Env &env, Allocator &alloc, Action &action)
		:
			_env(env), _alloc(alloc), _action(action)
		{ }

		void update_config(Xml_node const &config)
		{
			bool const use_rules_from_rom =
				(config.attribute_value(Rom_rules::node_type(), String<10>()) == "rom");

			_rom_rules.conditional(use_rules_from_rom, _env, _action);

			_config_rules.destruct();

			if (config.has_sub_node(Rom_rules::node_type()))
				_config_rules.construct(_alloc, config.sub_node(Rom_rules::node_type()));

			_action.layout_rules_changed();
		}

		/**
		 * Call 'fn' with XML node of active layout rules as argument
		 *
		 * The rules are either provided as a dedicated "rules" ROM module
		 * or as '<rules>' sub node of the configuration. The former is
		 * enabled via the 'rules="rom"' config attribute. If both are
		 * definitions are present, the rules ROM - if valid - takes
		 * precedence over the configuration's '<rules>' node.
		 */
		void with_rules(auto const &fn) const
		{
			if (_rom_rules.constructed()) {
				Xml_node const &rules = _rom_rules->rom.xml();
				if (rules.type() == Rom_rules::node_type()) {
					fn(rules);
					return;
				}
			}

			if (_config_rules.constructed()) {
				fn(_config_rules->xml);
				return;
			}

			fn(Xml_node("<rules/>"));
		}
};

#endif /* _LAYOUT_RULES_H_ */
