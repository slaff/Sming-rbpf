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
#include <string.h>
#include <assert.h>
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

typedef enum {
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
} bpf_error_t;

typedef enum {
    BPF_MEM_REGION_READ     = 0x01,
    BPF_MEM_REGION_WRITE    = 0x02,
    BPF_MEM_REGION_EXE      = 0x04, ///< Not currently used
} bpf_access_type_t;

typedef struct bpf_mem_region {
    struct bpf_mem_region *next;
    const uint8_t *start;       ///< Address within application image
    const uint8_t *phys_start;  ///< System memory start (to allow segment relocation)
    size_t len;
    uint8_t flag;               ///< bpf_access_type_t
} bpf_mem_region_t;

typedef enum {
    BPF_FLAG_SETUP_DONE     = 0x01,
    BPF_FLAG_PREFLIGHT_DONE = 0x02,
    BPF_CONFIG_NO_RETURN    = 0x0100, ///< Script doesn't need to have a return
} bpf_instance_flag_t;

typedef struct bpf_s {
    /* Initialised by application */
    const uint8_t *application;     ///< Application bytecode
    size_t application_len;         ///< Application length
    uint8_t *stack;                 ///< VM stack, must be a multiple of 8 bytes and aligned
    size_t stack_size;              ///< VM stack size in bytes
    /* Initialised by bpf_setup() */
    bpf_mem_region_t stack_region;
    bpf_mem_region_t rodata_region;
    bpf_mem_region_t data_region;
    bpf_mem_region_t arg_region;
    btree_t btree;                  ///< Local btree
    uint16_t flags;                 ///< bpf_instance_flag_t
    uint32_t branches_remaining;    ///< Number of allowed branch instructions remaining
} bpf_t;

/**
 * @brief Initialise global bpf data
 * Call once at startup before using any other bpf calls.
 */
void bpf_init(void);

/**
 * @brief Setup a BPF container
 * @param bpf Container to initialise. Caller must set application and stack fields and
 * zero-initialise the remainder.
 */
int bpf_setup(bpf_t *bpf);

/**
 * @brief Release any resources allocated for a container
 * @param bpf
 * 
 * DATA and BSS sections are copied to RAM which is dynamically allocated by `bpf_setup()`.
 * This ensures container code is not modified and permits multiple instances.
 */
void bpf_destroy(bpf_t *bpf);

/**
 * @brief Validate container before first execution
 * @param bpf
 * 
 * Called internally by execute functions
 */
int bpf_verify_preflight(bpf_t *bpf);

/**
 * @brief Execute a container
 * @param bpf
 * @param ctx Passed as parameter to container entry point function
 * @param ctx_size Size of context data in bytes
 * @param result OUT Result code returned from entry point function
 * Context may be modified by the container.
 */
int bpf_execute(bpf_t *bpf, void *ctx, size_t ctx_size, int64_t *result);

/**
 * @brief Execute a container and map provided context as memory region
 * @param bpf
 * @param ctx Passed as parameter to container entry point function,
 *            and mapped as memory region.
 * @param ctx_size Size of context data in bytes
 * @param result OUT Result code returned from entry point function
 */
int bpf_execute_ctx(bpf_t *bpf, void *ctx, size_t ctx_size, int64_t *result);

/**
 * @brief Add an additional memory region to the container
 * @param bpf
 * @param region Pointer to an uninitialised memory region structure
 * @param start Pointer to start of system memory region
 * @param len Size of region in bytes
 * @param flags Indicates whether read/write, etc.
 */
void bpf_add_region(bpf_t *bpf, bpf_mem_region_t *region,
                    void *start, size_t len, uint8_t flags);

/**
 * @brief Get pointer to memory with requested access
 * @param bpf
 * @param size Length of block to access in bytes
 * @param addr Start of block to access
 * @param type Type of access required as per bpf_access_type_t
 * @retval void* On success, points to system memory address.
 * Returns NULL if block is invalid or write access requested for read-only region.
 */
void* bpf_get_mem(const bpf_t *bpf, uint8_t size, const intptr_t addr, uint8_t type);

/*
 * @brief Check whether WRITE access is permitted for given memory block
 */
int bpf_store_allowed(const bpf_t *bpf, void *addr, size_t size);

/*
 * @brief Check whether READ access is permitted for given memory block
 */
int bpf_load_allowed(const bpf_t *bpf, void *addr, size_t size);

/**
 * @brief Read header structure from current application
 */
static inline rbpf_header_t rbpf_header(const bpf_t *bpf)
{
    assert(bpf->application != NULL);
    rbpf_header_t hdr;
    memcpy(&hdr, bpf->application, sizeof(hdr));
    return hdr;
}

/**
 * @brief Get pointer to RODATA application section
 */
static inline const void *rbpf_rodata(const bpf_t *bpf)
{
    return bpf->application + sizeof(rbpf_header_t) + rbpf_header(bpf).data_len;
}

/**
 * @brief Get pointer to DATA application section
 */
static inline const void *rbpf_data(const bpf_t *bpf)
{
    return bpf->application + sizeof(rbpf_header_t);
}

/**
 * @brief Get pointer to TEXT (code) application section
 */
static inline const void *rbpf_text(const bpf_t *bpf)
{
    rbpf_header_t header = rbpf_header(bpf);
    return bpf->application + sizeof(rbpf_header_t) + header.data_len + header.rodata_len;
}

#ifdef __cplusplus
}
#endif
#endif /* BPF_H */
/** @} */
