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

#include <stdint.h>
#include "bpf/shared.h"
#include "phydat.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef signed ssize_t;

#define DEFINE_SYSCALL(syscall_id, ResultType, name, ...) \
    static ResultType (*name)(__VA_ARGS__) = (ResultType(*)(__VA_ARGS__))syscall_id;


/**
 * Opaque dummy type saul registration
 */
typedef void bpf_saul_reg_t;

DEFINE_SYSCALL(BPF_FUNC_BPF_PRINTF, void, bpf_printf, const char *, ...)

DEFINE_SYSCALL(BPF_FUNC_BPF_STORE_GLOBAL, int, bpf_store_global, uint32_t key, uint32_t value)
DEFINE_SYSCALL(BPF_FUNC_BPF_STORE_LOCAL, int, bpf_store_local, uint32_t key, uint32_t value)
DEFINE_SYSCALL(BPF_FUNC_BPF_FETCH_GLOBAL, int, bpf_fetch_global, uint32_t key, uint32_t *value)
DEFINE_SYSCALL(BPF_FUNC_BPF_FETCH_LOCAL, int, bpf_fetch_local, uint32_t key, uint32_t *value)
DEFINE_SYSCALL(BPF_FUNC_BPF_NOW_MS, uint32_t, bpf_now_ms, void)

/* STDLIB */
DEFINE_SYSCALL(BPF_FUNC_BPF_MEMCPY, void, bpf_memcpy, void *dest, const void *src, size_t n)

/* SAUL calls */
DEFINE_SYSCALL(BPF_FUNC_BPF_SAUL_REG_FIND_NTH, bpf_saul_reg_t *, bpf_saul_reg_find_nth, int pos)
DEFINE_SYSCALL(BPF_FUNC_BPF_SAUL_REG_FIND_TYPE, bpf_saul_reg_t *, bpf_saul_reg_find_type, uint8_t type)
DEFINE_SYSCALL(BPF_FUNC_BPF_SAUL_REG_READ, int , bpf_saul_reg_read, bpf_saul_reg_t *dev, phydat_t *data)

/* CoAP calls */
DEFINE_SYSCALL(BPF_FUNC_BPF_GCOAP_RESP_INIT, void , bpf_gcoap_resp_init, bpf_coap_ctx_t *ctx, unsigned resp_code)
DEFINE_SYSCALL(BPF_FUNC_BPF_COAP_OPT_FINISH, ssize_t , bpf_coap_opt_finish, bpf_coap_ctx_t *ctx, unsigned opt)
DEFINE_SYSCALL(BPF_FUNC_BPF_COAP_ADD_FORMAT, void , bpf_coap_add_format, bpf_coap_ctx_t *ctx, uint32_t format)
DEFINE_SYSCALL(BPF_FUNC_BPF_COAP_GET_PDU, uint8_t *, bpf_coap_get_pdu, bpf_coap_ctx_t *ctx)

/* FMT calls */
DEFINE_SYSCALL(BPF_FUNC_BPF_FMT_S16_DFP, size_t , bpf_fmt_s16_dfp, char *out, int16_t val, int fp_digits)
DEFINE_SYSCALL(BPF_FUNC_BPF_FMT_U32_DEC, size_t , bpf_fmt_u32_dec, char *out, uint32_t val)

/* ZTIMER calls */
DEFINE_SYSCALL(BPF_FUNC_BPF_ZTIMER_NOW, uint32_t , bpf_ztimer_now, void)
DEFINE_SYSCALL(BPF_FUNC_BPF_ZTIMER_PERIODIC_WAKEUP, void , bpf_ztimer_periodic_wakeup, uint32_t *last_wakeup, uint32_t period)

#ifdef __cplusplus

}
#endif
#endif /* BPF_APPLICATION_CALL_H */
