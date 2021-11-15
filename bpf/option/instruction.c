/*
 * Copyright (C) 2020 Inria
 * Copyright (C) 2020 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <debug_progmem.h>

#include "bpf.h"
#include "bpf/instruction.h"
#include "bpf/call.h"

/* ALU type instructions */
static int _alu64(uint8_t opcode, uint64_t *src, uint64_t *dst)
{
    uint8_t instruction = opcode & BPF_INSTRUCTION_ALU_OP_MASK;

    switch (instruction) {
        case BPF_INSTRUCTION_ALU_ADD:
            *dst += *src;
            break;
        case BPF_INSTRUCTION_ALU_SUB:
            *dst -= *src;
            break;
        case BPF_INSTRUCTION_ALU_MUL:
            *dst *= *src;
            break;
        case BPF_INSTRUCTION_ALU_DIV:
            *dst /= *src;
            break;
        case BPF_INSTRUCTION_ALU_OR:
            *dst |= *src;
            break;
        case BPF_INSTRUCTION_ALU_AND:
            *dst &= *src;
            break;
        case BPF_INSTRUCTION_ALU_LSH:
            *dst <<= *src;
            break;
        case BPF_INSTRUCTION_ALU_RSH:
            *dst >>= *src;
            break;
        case BPF_INSTRUCTION_ALU_NEG:
            *dst = -*dst;
            break;
        case BPF_INSTRUCTION_ALU_MOD:
            *dst %= *src;
            break;
        case BPF_INSTRUCTION_ALU_XOR:
            *dst ^= *src;
            break;
        case BPF_INSTRUCTION_ALU_MOV:
            *dst = *src;
            break;
        case BPF_INSTRUCTION_ALU_ARSH:
            (*(int64_t*)dst) >>= *src;
            break;
        default:
            return BPF_ILLEGAL_INSTRUCTION;
    }

    return BPF_OK;
}

static int _alu32(uint8_t opcode, uint64_t *src, uint64_t *dst)
{
    int res = _alu64(opcode, src, dst);
    *dst &= UINT32_MAX;
    return res;
}

/* Load instructions */
static int _ld(bpf_t* bpf, const bpf_instruction_t **pc, uint64_t *src, uint64_t *dst)
{
    (void)src;
    bpf_instruction_t inst = GET_INSTRUCTION(*pc);
    uint64_t imm = (uint64_t)inst.immediate + ((uint64_t)GET_INSTRUCTION(*pc + 1).immediate << 32);

    switch(inst.opcode) {
        case 0x18: /* LDDW */
            break;
        case 0xB8: /* LDDWD */
            imm += (intptr_t)rbpf_data(bpf);
            break;
        case 0xD8: /* LDDWR */
            imm += (intptr_t)rbpf_rodata(bpf);
            break;
        /* Other BPF instructions are Linux socket/filter specific */
        default:
            return BPF_ILLEGAL_INSTRUCTION;
    }

    *dst = imm;
    (*pc)++;

    return BPF_OK;
}

/* Returns 1 if the code should jump, zero on no jump, negative on error */
static int _jump_cond(uint8_t opcode, uint64_t *src, uint64_t *dst)
{
    uint8_t instruction = opcode & BPF_INSTRUCTION_ALU_OP_MASK;

    switch (instruction) {
        case BPF_INSTRUCTION_BRANCH_JA:
            return 1;
        case BPF_INSTRUCTION_BRANCH_JEQ:
            return (*dst == *src);
        case BPF_INSTRUCTION_BRANCH_JGT:
            return (*dst > *src);
        case BPF_INSTRUCTION_BRANCH_JGE:
            return (*dst >= *src);
        case BPF_INSTRUCTION_BRANCH_JLT:
            return (*dst < *src);
        case BPF_INSTRUCTION_BRANCH_JLE:
            return (*dst <= *src);
        case BPF_INSTRUCTION_BRANCH_JSET:
            return (*dst & *src);
        case BPF_INSTRUCTION_BRANCH_JNE:
            return (*dst != *src);
        case BPF_INSTRUCTION_BRANCH_JSGT:
            return (*(int64_t*)dst > *(int64_t*)src);
        case BPF_INSTRUCTION_BRANCH_JSGE:
            return (*(int64_t*)dst >= *(int64_t*)src);
        case BPF_INSTRUCTION_BRANCH_JSLT:
            return (*(int64_t*)dst < *(int64_t*)src);
        case BPF_INSTRUCTION_BRANCH_JSLE:
            return (*(int64_t*)dst <= *(int64_t*)src);
        default:
            return BPF_ILLEGAL_INSTRUCTION;
    }
}

static int _jump(const bpf_instruction_t **pc, uint64_t *src, uint64_t *dst)
{
    bpf_instruction_t instruction = GET_INSTRUCTION(*pc);

    int res = _jump_cond(instruction.opcode, src, dst);
    if (res < 0) {
        return res;
    }
    if (res > 0) {
        *pc += instruction.offset;
    }
    return BPF_OK;
}

static int _load_x(const bpf_t *bpf, const bpf_instruction_t *instruction, uint64_t *regmap)
{
    intptr_t addr = regmap[instruction->src] + instruction->offset;
    void* memptr;

#define LOAD_X(DATATYPE) \
    memptr = bpf_get_mem(bpf, sizeof(DATATYPE), addr, BPF_MEM_REGION_READ); \
    if(memptr == NULL) { \
        return BPF_ILLEGAL_MEM; \
    } \
    regmap[instruction->dst] = *(const DATATYPE*)memptr;

    switch(instruction->opcode) {
        case 0x79:
            LOAD_X(uint64_t)
            break;
        case 0x61:
            LOAD_X(uint32_t)
            break;
        case 0x69:
            LOAD_X(uint16_t)
            break;
        case 0x71:
            LOAD_X(uint8_t)
            break;
        default:
            return BPF_ILLEGAL_INSTRUCTION;
    }
    return BPF_OK;
}

static int _store(const bpf_t *bpf, const bpf_instruction_t *instruction, uint64_t *regmap)
{
    intptr_t addr = regmap[instruction->dst] + instruction->offset;
    void* memptr;

#define STORE_IMM(DATATYPE) \
    memptr = bpf_get_mem(bpf, sizeof(DATATYPE), addr, BPF_MEM_REGION_WRITE); \
    if(memptr == NULL) { \
        return BPF_ILLEGAL_MEM; \
    } \
    *(DATATYPE*)memptr = instruction->immediate;

    switch(instruction->opcode) {
        case 0x7a:
            STORE_IMM(uint64_t)
            break;
        case 0x62:
            STORE_IMM(uint32_t)
            break;
        case 0x6a:
            STORE_IMM(uint16_t)
            break;
        case 0x72:
            STORE_IMM(uint8_t)
            break;
        default:
            return BPF_ILLEGAL_INSTRUCTION;
    }
    return BPF_OK;
}

static int _store_x(const bpf_t *bpf, const bpf_instruction_t *instruction, uint64_t *regmap)
{
    intptr_t addr = regmap[instruction->dst] + instruction->offset;
    void* memptr;

#define STORE_X(DATATYPE) \
    memptr = bpf_get_mem(bpf, sizeof(DATATYPE), addr, BPF_MEM_REGION_WRITE); \
    if(memptr == NULL) { \
        return BPF_ILLEGAL_MEM; \
    } \
    *(DATATYPE*)memptr = regmap[instruction->src];

    switch(instruction->opcode) {
        case 0x7b:
            STORE_X(uint64_t)
            break;
        case 0x63:
            STORE_X(uint32_t)
            break;
        case 0x6b:
            STORE_X(uint16_t)
            break;
        case 0x73:
            STORE_X(uint8_t)
            break;
        default:
            return BPF_ILLEGAL_INSTRUCTION;
    }
    return BPF_OK;
}

static int _instruction(bpf_t *bpf, uint64_t *regmap,
                        const bpf_instruction_t **pc)
{
    bpf_instruction_t instruction = GET_INSTRUCTION(*pc);

    /* Setup values for alu-based instructions */
    int64_t immediate = instruction.immediate;
    uint64_t *dst = &regmap[instruction.dst];
    uint64_t *src = (instruction.opcode & BPF_INSTRUCTION_ALU_S_MASK) ?
        &regmap[instruction.src] :
        (uint64_t*)&immediate;

    switch (instruction.opcode & BPF_INSTRUCTION_CLS_MASK) {
        case BPF_INSTRUCTION_CLS_ALU64:
            return _alu64(instruction.opcode, src, dst);
        case BPF_INSTRUCTION_CLS_ALU32:
            return _alu32(instruction.opcode, src, dst);
        case BPF_INSTRUCTION_CLS_BRANCH:
            return _jump(pc, src, dst);
        case BPF_INSTRUCTION_CLS_LD:
            return _ld(bpf, pc, src, dst);
        case BPF_INSTRUCTION_CLS_ST:
            return _store(bpf, &instruction, regmap);
        case BPF_INSTRUCTION_CLS_STX:
            return _store_x(bpf, &instruction, regmap);
        case BPF_INSTRUCTION_CLS_LDX:
            return _load_x(bpf, &instruction, regmap);
        default:
            return BPF_ILLEGAL_INSTRUCTION;
    }
}

int bpf_run(bpf_t *bpf, const void *ctx, int64_t *result)
{
    // bpf->instruction_count = 0;
    uint64_t regmap[11] = { 0 };
    regmap[1] = (uint64_t)(uintptr_t)ctx;
    regmap[10] = (uint64_t)(uintptr_t)(bpf->stack + bpf->stack_size);

    int res = bpf_verify_preflight(bpf);
    if (res < 0) {
        return res;
    }

    const bpf_instruction_t *pc = (const bpf_instruction_t*)rbpf_text(bpf);

    do {
        int res = _instruction(bpf, regmap, &pc);
        // bpf->instruction_count++;
        if (res < 0) {
            bpf_instruction_t inst = GET_INSTRUCTION(pc);
            if (inst.opcode == 0x85) {
                bpf_call_t call = bpf_get_call(inst.immediate);
                if (call) {
                    regmap[0] = (*(call))(bpf,
                                          regmap[1],
                                          regmap[2],
                                          regmap[3],
                                          regmap[4],
                                          regmap[5]);
                }
                else {
                    return BPF_ILLEGAL_CALL;
                }
            }
            else if (inst.opcode == 0x95) {
                break;
            }
            else {
#if DEBUG_VERBOSE_LEVEL == DBG
                intptr_t instruction = (uint8_t*)pc - bpf->application;
                (void)instruction;
                debug_d("Illegal %d, access PC: 0x%x, loc: %u", res, instruction, instruction/8 );
#endif
                return res;
            }
        }
        pc++;
    } while ((uint8_t*)pc < bpf->application + bpf->application_len);

    // debug_d("Number of instructions: %d\n", bpf->instruction_count);
    *result = regmap[0];
    return BPF_OK;
}
