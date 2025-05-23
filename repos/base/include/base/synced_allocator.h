/*
 * \brief  Mutex-guarded allocator interface
 * \author Norman Feske
 * \author Stefan Kalkowski
 * \date   2008-08-05
 */

/*
 * Copyright (C) 2008-2020 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__BASE__SYNCED_ALLOCATOR_H_
#define _INCLUDE__BASE__SYNCED_ALLOCATOR_H_

#include <base/allocator.h>
#include <base/synced_interface.h>

namespace Genode {
	template <typename> class Synced_allocator;
}


/**
 * Mutex-guarded allocator
 *
 * This class wraps the complete 'Allocator' interface while
 * preventing concurrent calls to the wrapped allocator implementation.
 *
 * \param ALLOC  class implementing the 'Allocator' interface
 */
template <typename ALLOC>
class Genode::Synced_allocator : public Allocator
{
	private:

		Mutex                          _mutex { };
		ALLOC                          _alloc;
		Synced_interface<ALLOC, Mutex> _synced_object;

	public:

		using Guard = typename Synced_interface<ALLOC, Mutex>::Guard;

		Synced_allocator(auto &&... args)
		: _alloc(args...), _synced_object(_mutex, &_alloc) { }

		Guard operator () ()       { return _synced_object(); }
		Guard operator () () const { return _synced_object(); }


		/*********************************
		 ** Memory::Allocator interface **
		 *********************************/

		Alloc_result try_alloc(size_t size) override {
			return _synced_object()->try_alloc(size); }

		void _free(Allocation &a) override { free(a.ptr, a.num_bytes); }


		/****************************************
		 ** Legacy Genode::Allocator interface **
		 ****************************************/

		void free(void *addr, size_t size) override {
			_synced_object()->free(addr, size); }

		size_t consumed() const override {
			return _synced_object()->consumed(); }

		size_t overhead(size_t size) const override {
			return _synced_object()->overhead(size); }

		bool need_size_for_free() const override {
			return _synced_object()->need_size_for_free(); }
};

#endif /* _INCLUDE__BASE__SYNCED_ALLOCATOR_H_ */
