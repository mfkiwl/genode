/*
 * \brief  Platform specific services for x86
 * \author Stefan Kalkowski
 * \date   2012-10-26
 */

/*
 * Copyright (C) 2012-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <base/service.h>
#include <base/heap.h>

/* core includes */
#include <platform.h>
#include <platform_services.h>
#include <io_port_root.h>


/*
 * Add x86 specific ioport service
 */
void Core::platform_add_local_services(Rpc_entrypoint         &,
                                       Sliced_heap            &sliced_heap,
                                       Registry<Service>      &local_services,
                                       Trace::Source_registry &,
                                       Ram_allocator          &,
                                       Local_rm               &,
                                       Range_allocator        &io_port_ranges)
{
	static Io_port_root io_port_root(io_port_ranges, sliced_heap);

	static Core_service<Io_port_session_component> io_port_ls(local_services,
	                                                          io_port_root);
}
