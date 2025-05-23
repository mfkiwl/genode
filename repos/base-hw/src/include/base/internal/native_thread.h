/*
 * \brief  Kernel-specific thread meta data
 * \author Norman Feske
 * \date   2016-03-11
 */

/*
 * Copyright (C) 2016-2017 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

#ifndef _INCLUDE__BASE__INTERNAL__NATIVE_THREAD_H_
#define _INCLUDE__BASE__INTERNAL__NATIVE_THREAD_H_

#include <util/noncopyable.h>
#include <base/native_capability.h>

namespace Genode { struct Native_thread; }

namespace Core { class  Platform_thread; }


struct Genode::Native_thread : Noncopyable
{
	Native_capability cap { };

	struct { Core::Platform_thread *platform_thread; };

	Native_thread() { }
};

#endif /* _INCLUDE__BASE__INTERNAL__NATIVE_THREAD_H_ */
