/*
 * \brief  Cpu_thread_component interface
 * \author Christian Prochaska
 * \date   2016-01-19
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _CPU_THREAD_COMPONENT_H_
#define _CPU_THREAD_COMPONENT_H_

/* Genode includes */
#include <base/thread.h>
#include <cpu_thread/client.h>
#include <log_session/connection.h>

/* local includes */
#include "cpu_session_component.h"

namespace Cpu_sampler {
	using namespace Genode;
	class Cpu_thread_component;
	class Cpu_session_component;
}

class Cpu_sampler::Cpu_thread_component : public Rpc_object<Cpu_thread>
{
	private:

		enum { SAMPLE_BUF_SIZE = 1024 };

		Cpu_session_component &_cpu_session_component;
		Env                   &_env;

		Allocator             &_md_alloc;

		Cpu_session::Create_thread_result _parent_cpu_thread;
		Constructible<Cpu_thread_client>  _parent_cpu_client;

		bool                   _started = false;

		Session_label          _label;
		Session_label          _log_session_label;

		Genode::addr_t         _sample_buf[SAMPLE_BUF_SIZE];
		unsigned int           _sample_buf_index = 0;

		Constructible<Log_connection> _log;

	public:

		Cpu_thread_component(Cpu_session_component   &cpu_session_component,
		                     Env                     &env,
		                     Allocator               &md_alloc,
		                     Pd_session_capability    pd,
		                     Cpu_session::Name const &name,
		                     Affinity::Location       affinity,
		                     Cpu_session::Weight      weight,
		                     addr_t                   utcb,
		                     char              const *thread_name,
		                     unsigned int             thread_id);
		~Cpu_thread_component();

		Cpu_session_component const *cpu_session_component() const
		{ return &_cpu_session_component; }

		Cpu_session::Create_thread_result result() { return _parent_cpu_thread; }

		Thread_capability parent_thread() { return _parent_cpu_client->rpc_cap(); }
		Session_label &label() { return _label; }

		void take_sample();
		void reset();
		void flush();

		/**************************
		 ** CPU thread interface **
		 *************************/

		Dataspace_capability utcb() override;
		void start(addr_t, addr_t) override;
		void pause() override;
		void resume() override;
		void single_step(bool) override;
		Thread_state state() override;
		void state(Thread_state const &) override;
		void exception_sigh(Signal_context_capability) override;
		void affinity(Affinity::Location) override;
		unsigned trace_control_index() override;
		Dataspace_capability trace_buffer() override;
		Dataspace_capability trace_policy() override;
};

#endif /* _CPU_THREAD_COMPONENT_H_ */
