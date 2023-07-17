#ifndef _REG_H_
#define _REG_H_

#include "types.h"

// general purpose registers in RISCV
enum gp_reg_type_t {
    zero, ra, sp, gp, tp,
    t0, t1, t2,
    s0, s1,
    a0, a1, a2, a3, a4, a5, a6, a7,
    s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
    t3, t4, t5, t6,
    num_gp_regs,
};

// floating purpose registers in RISCV
enum fp_reg_type_t {
    f0, f1, f2, f3, f4, f5, f6, f7,
    f8, f9, f10, f11, f12, f13, f14, f15,
    f16, f17, f18, f19, f20, f21, f22, f23,
    f24, f25, f26, f27, f28, f29, f30, f31,
    num_fp_regs,
};

typedef union {
    u32 w;
    u64 v;
    f64 d;
    f32 f;
} fp_reg_t;

#endif
