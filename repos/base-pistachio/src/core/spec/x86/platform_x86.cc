/*
 * \brief  Platform support specific to x86
 * \author Christian Helmuth
 * \date   2006-04-11
 */

/*
 * Copyright (C) 2006-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* base-internal includes */
#include <base/internal/pistachio.h>

/* core includes */
#include <kip.h>
#include <platform.h>
#include <util.h>

using namespace Core;


void Platform::_setup_io_port_alloc()
{
	/* setup allocator */
	enum { IO_PORT_RANGE_SIZE = 0x10000 };
	if (_io_port_alloc.add_range(0, IO_PORT_RANGE_SIZE).failed())
		warning("unable to initialize default I/O-port range");
}
