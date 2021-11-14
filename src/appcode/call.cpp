/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <debug_progmem.h>
#include <bpf.h>
#include "bpf/store.h"
#include "bpf/call.h"

bpf_call_t bpf_get_call(uint32_t num)
{
	switch(num) {
#define XX(function_code, function_name, return_type, ...)                                                             \
	case function_code:                                                                                                \
		return reinterpret_cast<bpf_call_t>(rBPF::VM::function_name);
		BPF_SYSCALL_MAP(XX)
	default:
		return nullptr;
	}
}
