/*
 * Copyright (C) 2021 Inria
 * Copyright (C) 2021 Koen Zandberg <koen@bergzand.net>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "bpf.h"
#include "bpf/instruction.h"
#include "bpf/call.h"

int bpf_verify_preflight(bpf_t *bpf)
{
    const bpf_instruction_t *application = rbpf_text(bpf);
    size_t length = rbpf_header(bpf).text_len;
    if (bpf->flags & BPF_FLAG_PREFLIGHT_DONE) {
        return BPF_OK;
    }

    if (length & 0x7) {
        return BPF_ILLEGAL_LEN;
    }


    for (const bpf_instruction_t *i= application;
            i < (bpf_instruction_t*)((uint8_t*)application + length); i++) {
        bpf_instruction_t inst = GET_INSTRUCTION(i);
        /* Check if register values are valid */
        if (inst.dst >= 11 || inst.src >= 11) {
            return BPF_ILLEGAL_REGISTER;
        }

        /* Double length instruction */
        if (inst.opcode == 0x18) {
            i++;
            continue;
        }

        /* Only instruction-specific checks here */
        if ((inst.opcode & BPF_INSTRUCTION_CLS_MASK) == BPF_INSTRUCTION_CLS_BRANCH) {
            intptr_t target = (intptr_t)(i + inst.offset);
            /* Check if the jump target is within bounds. The address is
             * incremented after the jump by the regular PC increase */
            if ((target >= (intptr_t)((uint8_t*)application + length))
                || (target < (intptr_t)application)) {
                return BPF_ILLEGAL_JUMP;
            }
        }

        if (inst.opcode == (BPF_INSTRUCTION_BRANCH_CALL | BPF_INSTRUCTION_CLS_BRANCH)) {
            if (!bpf_get_call(inst.immediate)) {
                return BPF_ILLEGAL_CALL;
            }
        }
    }

    size_t num_instructions = length/sizeof(bpf_instruction_t);

    /* Check if the last instruction is a return instruction */
    bpf_instruction_t inst = GET_INSTRUCTION(&application[num_instructions - 1]);
    if (inst.opcode != 0x95 && !(bpf->flags & BPF_CONFIG_NO_RETURN)) {
        return BPF_NO_RETURN;
    }
    bpf->flags |= BPF_FLAG_PREFLIGHT_DONE;
    return BPF_OK;
}
