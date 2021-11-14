/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef BPF_CALL_H
#define BPF_CALL_H

#include "bpf/shared.h"

#ifdef __cplusplus

namespace rBPF
{
namespace VM
{
#define XX(function_code, function_name, return_type, ...) return_type function_name(bpf_t*, ##__VA_ARGS__);
BPF_SYSCALL_MAP(XX)
#undef XX
} // namespace VM
} // namespace rBPF

extern "C" {
#endif

typedef uint32_t (*bpf_call_t)(bpf_t* bpf, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

bpf_call_t bpf_get_call(uint32_t num);

#ifdef __cplusplus
}
#endif
#endif /* BPF_CALL_H */
