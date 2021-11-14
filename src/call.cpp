/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <cstdint>
#include <cstdlib>
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

namespace rBPF
{
namespace VM
{
int bpf_printf(bpf_t* bpf, const char* fmt, ...)
{
	(void)bpf;
	va_list args;
	va_start(args, fmt);
	int n;
	if(isFlashPtr(fmt)) {
		size_t len = ALIGNUP4(strlen_P(fmt) + 1);
		char fmtbuf[len];
		memcpy(fmtbuf, fmt, len);
		n = m_vprintf(fmtbuf, args);
	} else {
		n = m_vprintf(fmt, args);
	}

	va_end(args);
	return n;
}

int bpf_store_local(bpf_t* bpf, uint32_t key, uint32_t value)
{
	return bpf_store_update_local(bpf, key, value);
}

int bpf_store_global(bpf_t* bpf, uint32_t key, uint32_t value)
{
	return bpf_store_update_global(key, value);
}

int bpf_fetch_local(bpf_t* bpf, uint32_t key, uint32_t* value)
{
	if(bpf_store_allowed(bpf, value, sizeof(*value)) < 0) {
		return -1;
	}
	return bpf_store_fetch_local(bpf, key, value);
}

int bpf_fetch_global(bpf_t* bpf, uint32_t key, uint32_t* value)
{
	if(bpf_store_allowed(bpf, value, sizeof(*value)) < 0) {
		return -1;
	}
	return bpf_store_fetch_global(key, value);
}

void bpf_memcpy(bpf_t* bpf, void* dest, const void* src, size_t size)
{
	if(bpf_store_allowed(bpf, dest, size) < 0) {
		return;
	}
	if(bpf_load_allowed(bpf, const_cast<void*>(src), size) < 0) {
		return;
	}
	memcpy(dest, src, size);
}

uint32_t bpf_now_ms(bpf_t* bpf)
{
	return system_get_time() / 1000;
}

} // namespace VM
} // namespace rBPF
