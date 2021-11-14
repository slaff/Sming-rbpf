/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_SHARED_H
#define BPF_SHARED_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Aux helper functions (stdlib) */
#define BPF_SYSCALL_STD(XX)                                                                                            \
	XX(0x01, bpf_printf, int, const char*, ...)                                                                        \
	XX(0x02, bpf_memcpy, void, void* dest, const void* src, size_t n)

/* Key/value store functions */
#define BPF_SYSCALL_STORE(XX)                                                                                          \
	XX(0x10, bpf_store_global, int, uint32_t key, uint32_t value)                                                      \
	XX(0x11, bpf_store_local, int, uint32_t key, uint32_t value)                                                       \
	XX(0x12, bpf_fetch_global, int, uint32_t key, uint32_t* value)                                                     \
	XX(0x13, bpf_fetch_local, int, uint32_t key, uint32_t* value)

/* Time(r) functions */
#define BPF_SYSCALL_TIMER(XX) XX(0x20, bpf_now_ms, uint32_t)

#ifndef BPF_SYSCALL_APP
#define BPF_SYSCALL_APP(XX)
#endif

#define BPF_SYSCALL_MAP(XX)                                                                                            \
	BPF_SYSCALL_STD(XX)                                                                                                \
	BPF_SYSCALL_STORE(XX)                                                                                              \
	BPF_SYSCALL_TIMER(XX)                                                                                              \
	BPF_SYSCALL_APP(XX)

#ifdef __cplusplus
}
#endif

#endif /* BPF_SHARED_H */
