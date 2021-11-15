/*
 * Copyright (C) 2021 Inria
 * Copyright (C) 2021 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_bpf (Extended) Berkeley Packet Filter compliant virtual machine
 * @ingroup     sys
 * @brief       API for eBPF-based scripts
 *
 *
 * @{
 *
 * @file
 * @brief       [eBPF](https://www.kernel.org/doc/html/latest/bpf/index.html)
 *
 * @author      Koen Zandberg <koen@bergzand.net>
 */

#ifndef BPF_H
#define BPF_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bpf/btree.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_BPF_ENABLE_ALU32
#define CONFIG_BPF_ENABLE_ALU32 (0)
#endif

#ifndef CONFIG_BPF_BRANCHES_ALLOWED
#define CONFIG_BPF_BRANCHES_ALLOWED 200
#endif

#define BPF_STACK_SIZE  512

#define RBPF_MAGIC_NO 0x72425046 /**< Magic header number: "rBPF" */

typedef struct __attribute__((packed)) {
    uint32_t magic;      /**< Magic number */
    uint32_t version;    /**< Version of the application */
    uint32_t flags;
    uint32_t data_len;   /**< Length of the data section */
    uint32_t bss_len;    /**< Length of the BSS section */
    uint32_t rodata_len; /**< Length of the rodata section */
    uint32_t text_len;   /**< Length of the text section */
    uint32_t functions;  /**< Number of functions available */
} rbpf_header_t;

typedef struct __attribute__((packed)) {
    uint16_t name_offset; /**< Offset in the rodata for the name */
    uint16_t flags;       /**< Flags for this function */
    uint16_t location_offset; /**< Location in the text section where the function starts */
} rbpf_function_t;

typedef enum {
    BPF_POLICY_CONTINUE,            /**< Always execute next hook */
    BPF_POLICY_ABORT_ON_NEGATIVE,   /**< Execute next script unless result is negative */
    BPF_POLICY_ABORT_ON_POSITIVE,   /**< Execute next script unless result is non-zero positive */
    BPF_POLICY_SINGLE,              /**< Always stop after this execution */
} bpf_hook_policy_t;

enum {
    BPF_OK = 0,
    BPF_ILLEGAL_INSTRUCTION = -1,
    BPF_ILLEGAL_MEM         = -2,
    BPF_ILLEGAL_JUMP        = -3,
    BPF_ILLEGAL_CALL        = -4,
    BPF_ILLEGAL_LEN         = -5,
    BPF_ILLEGAL_REGISTER    = -6,
    BPF_NO_RETURN           = -7,
    BPF_OUT_OF_BRANCHES     = -8,
    BPF_ILLEGAL_DIV         = -9,
};

typedef struct bpf_mem_region bpf_mem_region_t;

#define BPF_MEM_REGION_READ     0x01
#define BPF_MEM_REGION_WRITE    0x02
#define BPF_MEM_REGION_EXEC     0x04


struct bpf_mem_region {
    bpf_mem_region_t *next;
    const uint8_t *start;
    const uint8_t *phys_start;
    size_t len;
    uint8_t flag;
};

#define BPF_FLAG_SETUP_DONE        0x01
#define BPF_FLAG_PREFLIGHT_DONE    0x02
#define BPF_CONFIG_NO_RETURN       0x0100 /**< Script doesn't need to have a return */

typedef struct bpf_s {
    // Initialised by application
    const uint8_t *application; /**< Application bytecode */
    size_t application_len;     /**< Application length */
    uint8_t *stack;             /**< VM stack, must be a multiple of 8 bytes and aligned */
    size_t stack_size;          /**< VM stack size in bytes */
    // Initialised by bpf_setup()
    bpf_mem_region_t stack_region;
    bpf_mem_region_t rodata_region;
    bpf_mem_region_t data_region;
    bpf_mem_region_t arg_region;
    btree_t btree;              /**< Local btree */
    uint16_t flags;
    uint32_t branches_remaining; /**< Number of allowed branch instructions remaining */
} bpf_t;

void bpf_init(void);
int bpf_setup(bpf_t *bpf);
void bpf_destroy(bpf_t *bpf);

int bpf_verify_preflight(bpf_t *bpf);

int bpf_execute(bpf_t *bpf, void *ctx, size_t ctx_size, int64_t *result);
int bpf_execute_ctx(bpf_t *bpf, void *ctx, size_t ctx_size, int64_t *result);


void bpf_add_region(bpf_t *bpf, bpf_mem_region_t *region,
                    void *start, size_t len, uint8_t flags);

void* bpf_get_mem(const bpf_t *bpf, uint8_t size, const intptr_t addr, uint8_t type);
int bpf_store_allowed(const bpf_t *bpf, void *addr, size_t size);
int bpf_load_allowed(const bpf_t *bpf, void *addr, size_t size);

static inline rbpf_header_t rbpf_header(const bpf_t *bpf)
{
    rbpf_header_t hdr;
    memcpy(&hdr, bpf->application, sizeof(hdr));
    return hdr;
}

static inline const void *rbpf_rodata(const bpf_t *bpf)
{
    return bpf->application + sizeof(rbpf_header_t) + rbpf_header(bpf).data_len;
}

static inline const void *rbpf_data(const bpf_t *bpf)
{
    return bpf->application + sizeof(rbpf_header_t);
}

static inline const void *rbpf_text(const bpf_t *bpf)
{
    rbpf_header_t header = rbpf_header(bpf);
    return bpf->application + sizeof(rbpf_header_t) + header.data_len + header.rodata_len;
}

static inline size_t rbpf_text_len(const bpf_t *bpf)
{
    return rbpf_header(bpf).text_len;
}

#ifdef __cplusplus
}
#endif
#endif /* BPF_H */
/** @} */
