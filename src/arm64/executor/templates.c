/// File: Measurement templates for various threat models
///
// Copyright (C) Microsoft Corporation
// SPDX-License-Identifier: MIT

// -----------------------------------------------------------------------------------------------
// Note on registers.
// Some of the registers are reserved for a specific purpose and should never be overwritten.
// These include:
//   * X15 - hardware trace
//

#include "main.h"
#include <linux/string.h>

#define TEMPLATE_ENTER 0x00001111
#define TEMPLATE_INSERT_TC 0x00002222
#define TEMPLATE_RETURN 0x00003333

#define xstr(s) _str(s)
#define _str(s) str(s)
#define str(s) #s

int load_template(size_t tc_size)
{
    unsigned template_pos = 0;
    unsigned code_pos = 0;

    // skip until the beginning of the template
    for (;; template_pos++)
    {
        if (template_pos >= MAX_MEASUREMENT_CODE_SIZE)
            return -1;

        if (*(uint32_t *)&measurement_template[template_pos] == TEMPLATE_ENTER)
        {
            template_pos += 4;
            break;
        }
    }

    // copy the first part of the template
    for (;; template_pos++, code_pos++)
    {
        if (template_pos >= MAX_MEASUREMENT_CODE_SIZE)
            return -1;

        if (*(uint32_t *)&measurement_template[template_pos] == TEMPLATE_INSERT_TC)
        {
            template_pos += 4;
            break;
        }

        measurement_code[code_pos] = measurement_template[template_pos];
    }

    // copy the test case into the template
    memcpy(&measurement_code[code_pos], test_case, tc_size);
    code_pos += tc_size;

    // write the rest of the template
    for (;; template_pos++, code_pos++)
    {
        if (template_pos >= MAX_MEASUREMENT_CODE_SIZE)
            return -2;

        if (*(uint32_t *)&measurement_template[template_pos] == TEMPLATE_INSERT_TC)
            return -3;

        if (*(uint32_t *)&measurement_template[template_pos] == TEMPLATE_RETURN)
            break;

        measurement_code[code_pos] = measurement_template[template_pos];
    }

    // RET
    measurement_code[code_pos + 0] = '\xc0';
    measurement_code[code_pos + 1] = '\x03';
    measurement_code[code_pos + 2] = '\x5f';
    measurement_code[code_pos + 3] = '\xd6';
    return code_pos + 4;
}

// =================================================================================================
// Template building blocks
// =================================================================================================
// clang-format off
inline void prologue(void)
{
    // As we don't use a compiler to track clobbering,
    // we have to save the callee-saved regs
    asm volatile("" \
        "stp x16, x17, [sp, #-16]!\n"
        "stp x18, x19, [sp, #-16]!\n"
        "stp x20, x21, [sp, #-16]!\n"
        "stp x22, x23, [sp, #-16]!\n"
        "stp x24, x25, [sp, #-16]!\n"
        "stp x26, x27, [sp, #-16]!\n"
        "stp x28, x29, [sp, #-16]!\n"
        "str x30, [sp, #-16]!\n"

        // x30 <- input base address (stored in x0, the first argument of measurement_code)
        "mov x30, x0\n"

        // stored_rsp <- sp
        // "str sp, [x30, #"xstr(RSP_OFFSET)"]\n"
        "mov x0, sp\n"
        "str x0, [x30, #"xstr(RSP_OFFSET)"]\n"
    );
}

inline void epilogue(void) {
    asm volatile("" \
        // store the hardware trace (x15)
        "str x15, [x30, #"xstr(MEASUREMENT_OFFSET)"]\n"

        // rsp <- stored_rsp
        "ldr x0, [x30, #"xstr(RSP_OFFSET)"]\n"
        "mov sp, x0\n"

        // restore registers
        "ldr x30, [sp], #16\n"
        "ldp x28, x29, [sp], #16\n"
        "ldp x26, x27, [sp], #16\n"
        "ldp x24, x25, [sp], #16\n"
        "ldp x22, x23, [sp], #16\n"
        "ldp x20, x21, [sp], #16\n"
        "ldp x18, x19, [sp], #16\n"
        "ldp x16, x17, [sp], #16\n"
    );
}

#define SET_REGISTER_FROM_INPUT() asm volatile("" \
    "add sp, x30, #"xstr(REG_INIT_OFFSET)"\n" \
    "ldp x0, x1, [sp], #16\n" \
    "ldp x2, x3, [sp], #16\n" \
    "ldp x4, x5, [sp], #16\n" \
    "ldp x6, x7, [sp], #16\n" \
    "msr nzcv, x6\n" \
    "mov sp, x7\n");

// =================================================================================================
// L1D Prime+Probe
// =================================================================================================
#if L1D_ASSOCIATIVITY == 2

// clobber:
#define PRIME(BASE, OFFSET, TMP, ACC, COUNTER, REPS) asm volatile("" \
    "isb\n" \
    "mov "COUNTER", "REPS"\n" \
    "_arm64_executor_prime_outer:\n" \
    "mov "OFFSET", 0\n" \
    \
    "_arm64_executor_prime_inner:\n" \
    "isb\n" \
    "add "TMP", "BASE", "OFFSET"\n" \
    "ldr "ACC", ["TMP", #0]\n" \
    "isb\n" \
    "ldr "ACC", ["TMP", #4096]\n" \
    "isb\n" \
    "add "OFFSET", "OFFSET", #64\n" \
    \
    "mov "ACC", #4096\n" \
    "cmp "ACC", "OFFSET"\n" \
    "b.gt _arm64_executor_prime_inner\n" \
    \
    "sub "COUNTER", "COUNTER", #1\n" \
    "cmp "COUNTER", xzr\n" \
    "b.ne _arm64_executor_prime_outer\n" \
    \
    "isb\n" \
)

// clobber:
/*
#define PROBE(BASE, OFFSET, TMP, TMP2, ACC, DEST) asm volatile("" \
    "eor "DEST", "DEST", "DEST"\n" \
    "eor "OFFSET", "OFFSET", "OFFSET"\n" \
    "_arm64_executor_probe_loop:\n" \
    \
    "isb\n" \
    "eor "TMP", "TMP", "TMP"\n" \
    "mrs "TMP", pmevcntr0_el0\n" \
    "mov "ACC", "TMP"\n" \
    \
    "add "TMP", "BASE", "OFFSET"\n" \
    "ldr "TMP2", ["TMP", #0]\n" \
    "isb\n" \
    "ldr "TMP2", ["TMP", #4096]\n" \
    "isb\n" \
    \
    "mrs "TMP", pmevcntr0_el0\n" \
    "subs "ACC", "TMP", "ACC"\n" \
    "b.eq _arm64_executor_probe_failed\n" \
    "_arm64_executor_probe_success:\n" \
    "mov "DEST", "DEST", lsl #1\n" \
    "orr "DEST", "DEST", #1\n" \
    "b _arm64_executor_probe_loop_check\n" \
    \
    "_arm64_executor_probe_failed:\n" \
    "mov "DEST", "DEST", lsl #1\n" \
    \
    "_arm64_executor_probe_loop_check:\n" \
    "add "OFFSET", "OFFSET", #64\n" \
    "mov "TMP", #4096\n" \
    "cmp "TMP", "OFFSET"\n" \
    "b.gt _arm64_executor_probe_loop\n" \
)
*/
#define PROBE(BASE, OFFSET, TMP, TMP2, ACC, DEST) asm volatile("" \
    "mrs "TMP", pmevcntr0_el0\n" \
    "ldr "ACC", ["BASE", #8192]\n" \
    "isb\n" \
    "dsb SY\n" \
    "mrs "TMP2", pmevcntr0_el0\n" \
    "sub "DEST", "TMP2", "TMP"\n" \
)

#endif

void template_l1d_prime_probe(void) {
    asm volatile(".long "xstr(TEMPLATE_ENTER));

    // ensure that we don't crash because of BTI
    asm volatile("bti c");

    prologue();

    PRIME("x30", "x1", "x2", "x3", "x4", "32");

    // Initialize registers
    SET_REGISTER_FROM_INPUT();

    // Execute the test case
    asm("\nisb\n"
        ".long "xstr(TEMPLATE_INSERT_TC)" \n"
        "isb\n");

    // Probe and store the resulting eviction bitmap map into x?
    PROBE("x30", "x0", "x1", "x2", "x3", "x15");

    epilogue();
    asm volatile(".long "xstr(TEMPLATE_RETURN));
}
