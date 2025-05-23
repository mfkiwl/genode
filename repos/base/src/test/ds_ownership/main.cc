/*
 * \brief  Testing the distinction between user and owner of a RAM dataspace
 * \author Martin Stein
 * \author Norman Feske
 * \date   2012-04-19
 */

/*
 * Copyright (C) 2008-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <pd_session/connection.h>
#include <base/log.h>
#include <base/component.h>


void Component::construct(Genode::Env &env)
{
	using namespace Genode;

	log("--- dataspace ownership test ---");

	static Pd_connection pd_1 { env };
	static Pd_connection pd_2 { env };

	Pd_ram_allocator ram_1 { pd_1 }, ram_2 { pd_2 };

	log("allocate dataspace from one RAM session");
	pd_1.ref_account(env.pd_session_cap());
	env.pd().transfer_quota(pd_1.cap(), Ram_quota{8*1024});
	Ram_dataspace_capability ds = ram_1.alloc(sizeof(unsigned));

	log("attempt to free dataspace from foreign RAM session");
	ram_2.free(ds);

	log("try to attach dataspace to see if it still exists");
	if (env.rm().attach(ds, { }).failed())
		warning("dataspace unexpectedly vanished");

	log("attach operation succeeded");

	log("free dataspace from legitimate RAM session");
	Ram_quota const quota_before_free { pd_1.avail_ram() };
	ram_1.free(ds);
	Ram_quota const quota_after_free { pd_1.avail_ram() };

	if (quota_after_free.value > quota_before_free.value)
		log("test succeeded");
	else
		error("test failed");
}

