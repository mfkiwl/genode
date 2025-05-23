/*
 * \brief  Instance of the core-local (Genode) capability space
 * \author Norman Feske
 * \date   2015-05-08
 */

/*
 * Copyright (C) 2015-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* base includes */
#include <base/capability.h>

/* base-internal includes */
#include <base/internal/capability_data.h>
#include <base/internal/capability_space_sel4.h>

/* core includes */
#include <core_capability_space.h>
#include <platform.h>


/**
 * Core-specific supplement of the capability meta data
 */
class Genode::Native_capability::Data : public Capability_data
{
	public:

		Data(Rpc_obj_key key)
		:
			Capability_data(key)
		{ }

		Data() { }
};


using namespace Core;


/**
 * Singleton instance of core-specific capability space
 */
namespace {

	struct Local_capability_space
	:
		Capability_space_sel4<1UL << Core_cspace::NUM_CORE_SEL_LOG2, 0UL,
		                      Native_capability::Data>
	{ };

	static Local_capability_space &local_capability_space()
	{
		static Local_capability_space capability_space;
		return capability_space;
	}
}


/********************************************************************
 ** Implementation of the core-specific Capability_space interface **
 ********************************************************************/

Native_capability
Capability_space::create_rpc_obj_cap(Native_capability ep_cap,
                                     Rpc_obj_key rpc_obj_key)
{
	if (!ep_cap.valid())
		return { };

	/* allocate core-local selector for RPC object */
	Cap_sel const rpc_obj_sel = platform_specific().core_sel_alloc().alloc();

	/* create Genode capability */
	auto & data = local_capability_space().create_capability(rpc_obj_sel,
	                                                         rpc_obj_key);

	Cap_sel const ep_sel(local_capability_space().sel(*ep_cap.data()));

	/* mint endpoint capability into RPC object capability */
	{
		seL4_CNode       const service    = seL4_CapInitThreadCNode;
		seL4_Word        const dest_index = rpc_obj_sel.value();
		uint8_t          const dest_depth = 32;
		seL4_CNode       const src_root   = seL4_CapInitThreadCNode;
		seL4_Word        const src_index  = ep_sel.value();
		uint8_t          const src_depth  = 32;
		seL4_CapRights_t const rights     = seL4_AllRights;
		seL4_Word        const badge      = rpc_obj_key.value();

		int const ret = seL4_CNode_Mint(service,
		                                dest_index,
		                                dest_depth,
		                                src_root,
		                                src_index,
		                                src_depth,
		                                rights,
		                                badge);
		ASSERT(ret == seL4_NoError);
	}

	return Native_capability(&data);
}


void Capability_space::destroy_rpc_obj_cap(Native_capability &cap)
{
	if (!cap.valid() || !cap.data())
		return;

	Cap_sel const sel(local_capability_space().sel(*cap.data()));
	seL4_CNode_Revoke(seL4_CapInitThreadCNode, sel.value(), 32);
}


template <unsigned A, unsigned B, typename C>
void Capability_space_sel4<A, B, C>::_cleanup_last_ref(Native_capability::Data & data)
{
	Cap_sel const sel(local_capability_space().sel(data));

	/*
	 * Continues cleanup of destroy_rpc_obj_cap() and reverts the allocation
	 * of create_rpc_obj_cap () since the ref count dropped now to zero.
	 */
	seL4_CNode_Revoke(seL4_CapInitThreadCNode, sel.value(), 32);
	seL4_CNode_Delete(seL4_CapInitThreadCNode, sel.value(), 32);
	platform_specific().core_sel_alloc().free(sel);
}


/******************************************************
 ** Implementation of the Capability_space interface **
 ******************************************************/

Native_capability Capability_space::create_ep_cap(Thread &ep_thread)
{
	return ep_thread.with_native_thread(
		[&] (Native_thread &nt) {
			Cap_sel const ep_sel(nt.attr.ep_sel);

			/* entrypoint capabilities are not allocated from a PD session */
			auto &data = local_capability_space().create_capability(ep_sel, Rpc_obj_key());
			return Native_capability(&data);
		},
		[&] { return Native_capability(); });
}


void Capability_space::dec_ref(Native_capability::Data &data)
{
	local_capability_space().dec_ref(data);
}


void Capability_space::inc_ref(Native_capability::Data &data)
{
	local_capability_space().inc_ref(data);
}


Rpc_obj_key Capability_space::rpc_obj_key(Native_capability::Data const &data)
{
	return local_capability_space().rpc_obj_key(data);
}


Capability_space::Ipc_cap_data Capability_space::ipc_cap_data(Native_capability const &cap)
{
	return local_capability_space().ipc_cap_data(*cap.data());
}


Native_capability Capability_space::lookup(Rpc_obj_key rpc_obj_key)
{
	Native_capability::Data *data = local_capability_space().lookup(rpc_obj_key);

	return data ? Native_capability(data) : Native_capability();
}


unsigned Capability_space::alloc_rcv_sel()
{
	return platform_specific().alloc_core_rcv_sel();
}


void Capability_space::reset_sel(unsigned sel)
{
	return platform_specific().reset_sel(sel);
}


Native_capability Capability_space::import(Ipc_cap_data ipc_data)
{
	auto &data = local_capability_space().create_capability(ipc_data.sel,
	                                                        ipc_data.rpc_obj_key);

	return Native_capability(&data);
}


Native_capability
Capability_space::create_notification_cap(Cap_sel &notify_cap)
{
	auto &data = local_capability_space().create_capability(notify_cap,
	                                                        Rpc_obj_key());

	return Native_capability(&data);
}
