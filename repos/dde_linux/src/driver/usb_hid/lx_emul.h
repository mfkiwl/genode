/*
 * \brief  Dummy definitions of Linux Kernel functions
 * \author Sebastian Sumpf
 * \date   2023-07-11
 */

/*
 * Copyright (C) 2023 Genode Labs GmbH
 *
 * This file is distributed under the terms of the GNU General Public License
 * version 2 or later.
 */

#include <lx_emul/debug.h>
#include <linux/init.h>
#include <linux/sched/debug.h>
#include <linux/usb.h>

struct input_dev_poller;
struct input_event;
struct usb_hub;
