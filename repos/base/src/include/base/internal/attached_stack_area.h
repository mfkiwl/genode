/*
 * \brief  Stack area attached to the local address space
 * \author Norman Feske
 * \date   2013-09-25
 */

/*
 * Copyright (C) 2006-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__BASE__INTERNAL__ATTACHED_STACK_AREA_H_
#define _INCLUDE__BASE__INTERNAL__ATTACHED_STACK_AREA_H_

/* Genode includes */
#include <parent/client.h>
#include <region_map/client.h>
#include <pd_session/client.h>

/* base-internal includes */
#include <base/internal/stack_area.h>
#include <base/internal/expanding_region_map_client.h>

namespace Genode { struct Attached_stack_area; }


struct Genode::Attached_stack_area : Expanding_region_map_client
{
	Attached_stack_area(Parent &parent, Pd_session_capability pd)
	:
		Expanding_region_map_client(parent, pd, Pd_session_client(pd).stack_area(),
		                            Parent::Env::pd())
	{
		Region_map_client local_rm(Pd_session_client(pd).address_space());

		if (local_rm.attach(Expanding_region_map_client::dataspace(), Region_map::Attr {
			.size       = stack_area_virtual_size(),
			.offset     = { },
			.use_at     = true,
			.at         = stack_area_virtual_base(),
			.executable = false,
			.writeable  = true
		}).failed())
			warning("unable to attach stack area to local address space");
	}
};

#endif /* _INCLUDE__BASE__INTERNAL__ATTACHED_STACK_AREA_H_ */
