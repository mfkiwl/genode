/*
 * \brief  Generic implementation parts of the signaling framework
 * \author Norman Feske
 * \date   2017-05-10
 */

/*
 * Copyright (C) 2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/env.h>
#include <base/rpc_server.h>
#include <base/trace/events.h>

/* core includes */
#include <signal_source_component.h>
#include <signal_transmitter.h>
#include <signal_delivery_proxy.h>

/* base-internal includes */
#include <base/internal/globals.h>

using namespace Core;


static Constructible<Signal_delivery_proxy_component> delivery_proxy;


void Core::init_core_signal_transmitter(Rpc_entrypoint &ep)
{
	if (!delivery_proxy.constructed())
		delivery_proxy.construct(ep);
}


void Signal_transmitter::submit(unsigned cnt)
{
	{
		Trace::Signal_submit trace_event(cnt);
	}
	delivery_proxy->submit(_context, cnt);
}


Rpc_entrypoint &Core::core_signal_ep(Rpc_entrypoint &)
{
	size_t const STACK_SIZE = 20 * 1024;
	static Rpc_entrypoint ep(nullptr, STACK_SIZE,
	                         "signal_entrypoint", Affinity::Location());
	return ep;
}
