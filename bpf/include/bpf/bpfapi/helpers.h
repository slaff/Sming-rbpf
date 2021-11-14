/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_BPFAPI_HELPERS_H
#define BPF_BPFAPI_HELPERS_H

#include "bpf/shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XX(function_code, function_name, return_type, ...)                                                             \
	static return_type (*function_name)(__VA_ARGS__) = (return_type(*)(__VA_ARGS__))function_code;
BPF_SYSCALL_MAP(XX)
#undef XX

#ifdef __cplusplus
}
#endif

#endif /* BPF_APPLICATION_CALL_H */
