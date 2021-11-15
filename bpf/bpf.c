/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stdbool.h>
#include "assert.h"
#include "bpf.h"
#include "bpf/store.h"
#include <debug_progmem.h>

extern int bpf_run(bpf_t *bpf, const void *ctx, int64_t *result);

void* bpf_get_mem(const bpf_t *bpf, uint8_t size, const intptr_t addr, uint8_t type)
{
    const intptr_t end = addr + size;
    for (const bpf_mem_region_t *region = &bpf->stack_region; region; region = region->next) {
        if (addr >= (intptr_t)region->start && end <= (intptr_t)(region->start + region->len)) {
            if ((region->flag & type) == 0) {
                debug_d("Denied access to 0x%x with len %u\n", (void*)addr, size);
                return NULL;
            }
            return (void*)(region->phys_start + addr - region->start);
        }
    }

    debug_d("Attempt to access invalid memory at 0x%x with len %u\n", (void*)addr, size);
    return NULL;
}

int bpf_store_allowed(const bpf_t *bpf, void *addr, size_t size)
{
    return bpf_get_mem(bpf, size, (intptr_t)addr, BPF_MEM_REGION_WRITE) ? 0 : -1;
}

int bpf_load_allowed(const bpf_t *bpf, void *addr, size_t size)
{
    return bpf_get_mem(bpf, size, (intptr_t)addr, BPF_MEM_REGION_READ) ? 0 : -1;
}

static int _execute(bpf_t *bpf, void *ctx, int64_t *result)
{
    assert(bpf->flags & BPF_FLAG_SETUP_DONE);
    return bpf_run(bpf, ctx, result);
}

int bpf_execute(bpf_t *bpf, void *ctx, size_t ctx_len, int64_t *result)
{
    bpf->arg_region.start = NULL;
    bpf->arg_region.len = 0;

    return _execute(bpf, ctx, result);
}

int bpf_execute_ctx(bpf_t *bpf, void *ctx, size_t ctx_len, int64_t *result)
{
    bpf->arg_region.start = bpf->arg_region.phys_start = ctx;
    bpf->arg_region.len = ctx_len;
    bpf->arg_region.flag = (BPF_MEM_REGION_READ | BPF_MEM_REGION_WRITE);

    return _execute(bpf, ctx, result);
}

int bpf_setup(bpf_t *bpf)
{
    if(bpf == NULL) {
        return -1;
    }

    bpf_mem_region_t stack_region = {
        .start = bpf->stack,
        .phys_start = bpf->stack,
        .len = bpf->stack_size,
        .flag = BPF_MEM_REGION_READ | BPF_MEM_REGION_WRITE,
        .next = &bpf->data_region,
    };
    bpf->stack_region = stack_region;

    rbpf_header_t hdr = rbpf_header(bpf);

    size_t len = ALIGNUP4(hdr.data_len + hdr.bss_len);
    if(len == 0) {
        bpf_mem_region_t data_region = {
            .next = &bpf->rodata_region,
        };
        bpf->data_region = data_region;
    } else {
        const void* data = rbpf_data(bpf);
        uint8_t* ptr = malloc(len);
        if(ptr == NULL) {
            return -1;
        }
        memcpy(ptr, data, ALIGNUP4(hdr.data_len));
        memset(ptr + hdr.data_len, 0, hdr.bss_len);

        bpf_mem_region_t data_region = {
            .start = data,
            .phys_start = ptr,
            .len = len,
            .flag = BPF_MEM_REGION_READ | BPF_MEM_REGION_WRITE,
            .next = &bpf->rodata_region,
        };
        bpf->data_region = data_region;
    }

    const void* rodata = rbpf_rodata(bpf);
    bpf_mem_region_t rodata_region = {
        .start =  rodata,
        .phys_start = rodata,
        .len = hdr.rodata_len,
        .flag = BPF_MEM_REGION_READ,
        .next = &bpf->arg_region,
    };
    bpf->rodata_region = rodata_region;

    bpf_mem_region_t arg_region = {};
    bpf->arg_region = arg_region;

    bpf->flags |= BPF_FLAG_SETUP_DONE;

    return 0;
}

void bpf_destroy(bpf_t* bpf)
{
    if(bpf == NULL) {
        return;
    }
    free((void*)bpf->data_region.phys_start);
    memset(bpf, 0, sizeof(bpf_t));
}

void bpf_add_region(bpf_t *bpf, bpf_mem_region_t *region,
                    void *start, size_t len, uint8_t flags)
{
    bpf_mem_region_t r = {
        .next = bpf->arg_region.next,
        .start = start,
        .phys_start = start,
        .len = len,
        .flag = flags,
    };
    *region = r;
    bpf->arg_region.next = region;
}

void bpf_init(void)
{
    bpf_store_init();
}
