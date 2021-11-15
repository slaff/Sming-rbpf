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
/**
 * @brief All calls are declared using a macro with the following form:
 *   XX(function_code, function_name, return_type, ...)
 *      function_code   32-bit system call code
 *      function_name   Name used to invoke function within VM
 *      return_type     Function return type (void, int, etc)
 *      ...             Parameter types and names
 *
 */
#define XX(function_code, function_name, return_type, ...) return_type function_name(bpf_t*, ##__VA_ARGS__);
BPF_SYSCALL_MAP(XX)
#undef XX
} // namespace VM
} // namespace rBPF

extern "C" {
#endif

/**
 * @brief System call implementation prototype
 * 
 * All calls must accept bpf parameter and from 0 to 5 parameters.
 */
typedef uint32_t (*bpf_call_t)(bpf_t* bpf, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5);

/**
 * @brief Map function code to system call
 * @param num Functdion code
 * @retval bpf_call_t Pointer to implementation, or NULL if not available
 */
bpf_call_t bpf_get_call(uint32_t num);

#ifdef __cplusplus
}
#endif

#endif /* BPF_CALL_H */
