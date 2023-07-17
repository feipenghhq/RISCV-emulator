#include <math.h>
#include "rvemu.h"


typedef void (func_t)(state_t *, inst_t *);

/////////////////////////////////////////
// debug related helper macro/functions
/////////////////////////////////////////
#ifdef DEBUG

#define _printreg(name, reg) printf("DEBUG: register %s = %lx\n", name, state->gp_regs[reg])
#define _printimm() printf("DEBUG: imm = %x\n", inst.imm)

const char *gp_reg_name[] = {
    "zero", "ra", "sp", "gp", "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};

#define printreg(pcval, reg) if (state->pc == (pcval)) _printreg(gp_reg_name[reg], reg);
#define printimm(pcval) if (state->pc == (pcval)) _printimm();

#endif

/////////////////////////////////////////
// Functions to execute instruction
/////////////////////////////////////////

    /////////////////////////////////////////
    // Reg / Imm type instructions
    /////////////////////////////////////////

    #define FUNC(expr) \
        i64 rs1 = state->gp_regs[inst->rs1]; \
        i64 imm = (i64) inst->imm; \
        state->gp_regs[inst->rd] = (expr); \

    // addi instruction
    static void exec_addi(state_t *state, inst_t *inst) {
        FUNC(rs1 + imm);
    }

    // slti instruction
    static void exec_slti(state_t *state, inst_t *inst) {
        FUNC(rs1 < imm);
    }

    // sltiu instruction
    static void exec_sltiu(state_t *state, inst_t *inst) {
        FUNC((u64) rs1 < (u64) imm);
    }

    // andi instruction
    static void exec_andi(state_t *state, inst_t *inst) {
        FUNC(rs1 & imm);
    }

    // ori instruction
    static void exec_ori(state_t *state, inst_t *inst) {
        FUNC(rs1 | imm);
    }

    // xori instruction
    static void exec_xori(state_t *state, inst_t *inst) {
        FUNC(rs1 ^ imm);
    }

    // addiw instruction
    static void exec_addiw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)(rs1 + imm));
    }

    // slli instruction
    static void exec_slli(state_t *state, inst_t *inst) {
        FUNC(rs1 << (imm & 0x3F));
    }

    // srli instruction
    static void exec_srli(state_t *state, inst_t *inst) {
        FUNC((u64) rs1 >> (imm & 0x3F));
    }

    // srai instruction
    static void exec_srai(state_t *state, inst_t *inst) {
        FUNC(rs1 >> (imm & 0x3F));
    }

    // slliw instruction
    static void exec_slliw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)(rs1 << (imm & 0x1F)));
    }

    // srliw instruction
    static void exec_srliw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)((u32) rs1 >> (imm & 0x1F)));
    }

    // sraiw instruction
    static void exec_sraiw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)((i32) rs1 >> (imm & 0x1F)));
    }

    #undef FUNC

    /////////////////////////////////////////
    // Reg / Reg type instructions
    /////////////////////////////////////////

    // Macros for r-type instruction

    #define FUNC(expr) \
        i64 rs1 = state->gp_regs[inst->rs1]; \
        i64 rs2 = state->gp_regs[inst->rs2]; \
        state->gp_regs[inst->rd] = (i64) (expr); \

    // add instruction
    static void exec_add(state_t *state, inst_t *inst) {
        FUNC(rs1 + rs2);
    }

    // slt instruction
    static void exec_slt(state_t *state, inst_t *inst) {
        FUNC(rs1 < rs2);
    }

    // sltu instruction
    static void exec_sltu(state_t *state, inst_t *inst) {
        FUNC((u64) rs1 < (u64) rs2);
    }

    // and instruction
    static void exec_and(state_t *state, inst_t *inst) {
        FUNC(rs1 & rs2);
    }

    // or instruction
    static void exec_or(state_t *state, inst_t *inst) {
        FUNC(rs1 | rs2);
    }

    // xor instruction
    static void exec_xor(state_t *state, inst_t *inst) {
        FUNC(rs1 ^ rs2);
    }

    // sub instruction
    static void exec_sub(state_t *state, inst_t *inst) {
        FUNC(rs1 - rs2);
    }

    // addw instruction
    static void exec_addw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)(rs1 + rs2));
    }

    // subw instruction
    static void exec_subw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)(rs1 - rs2));
    }

    // sll instruction
    static void exec_sll(state_t *state, inst_t *inst) {
        FUNC(rs1 << (rs2 & 0x3F));
    }

    // srl instruction
    static void exec_srl(state_t *state, inst_t *inst) {
        FUNC((u64) rs1 >> (rs2 & 0x3F));
    }

    // sra instruction
    static void exec_sra(state_t *state, inst_t *inst) {
        FUNC(rs1 >> (rs2 & 0x3F));
    }

    // sllw instruction
    static void exec_sllw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)((u32) rs1 << (rs2 & 0x1F)));
    }

    // srlw instruction
    static void exec_srlw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)((u32) rs1 >> (rs2 & 0x1F)));
    }

    // sraw instruction
    static void exec_sraw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)((i32) rs1 >> (rs2 & 0x1F)));
    }

    // mul instruction
    static void exec_mul(state_t *state, inst_t *inst) {
        FUNC(rs1 * rs2);
    }

    // mulh instruction
    static void exec_mulh(state_t *state, inst_t *inst) {
        fatal("unimplemented MULH instructions");
    }

    // mulhsu instruction
    static void exec_mulhsu(state_t *state, inst_t *inst) {
        fatal("unimplemented MULHSU instructions");
    }

    // mulhu instruction
    static void exec_mulhu(state_t *state, inst_t *inst) {
        fatal("unimplemented MULHU instructions");
    }

    // div instruction
    static void exec_div(state_t *state, inst_t *inst) {
        FUNC(rs1 / rs2);
    }

    // divu instruction
    static void exec_divu(state_t *state, inst_t *inst) {
        FUNC((u64) rs1 / (u64) rs2);
    }

    // rem instruction
    static void exec_rem(state_t *state, inst_t *inst) {
        FUNC(rs1 % rs2);
    }

    // remu instruction
    static void exec_remu(state_t *state, inst_t *inst) {
        FUNC((u64) rs1 % (u64) rs2);
    }

    // mulw instruction
    static void exec_mulw(state_t *state, inst_t *inst) {
        FUNC((i64)(i32)(rs1 * rs2));
    }

    // divw instruction
    static void exec_divw(state_t *state, inst_t *inst) {
        FUNC((i32) rs1 / (i32) rs2);
    }

    // divuw instruction
    static void exec_divuw(state_t *state, inst_t *inst) {
        FUNC((u32) rs1 / (u32) rs2);
    }

    // remw instruction
    static void exec_remw(state_t *state, inst_t *inst) {
        FUNC((i32) rs1 % (i32) rs2);
    }

    // remuw instruction
    static void exec_remuw(state_t *state, inst_t *inst) {
        FUNC((u32) rs1 % (u32) rs2);
    }

    #undef FUNC

    /////////////////////////////////////////
    // Other type instructions
    /////////////////////////////////////////

    // lui instruction
    static void exec_lui(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) inst->imm;
    }

    // auipc instruction
    static void exec_auipc(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) inst->imm + state->pc;
    }


    /////////////////////////////////////////
    // Jump type instructions
    /////////////////////////////////////////

    #define FUNC(expr, inst) \
        state->gp_regs[inst->rd] = state->pc + ((inst->rvc) ? 2 : 4); \
        state->exit_reason = direct_branch; \
        state->reenter_pc = (expr); \
        if ((state->reenter_pc & 0x3) != 0) { \
            fatal("instruction_address_misaligned"); \
            state->raise_exception = true; \
            state->exception_code = instruction_address_misaligned; \
        } \


    static void exec_jal(state_t *state, inst_t *inst) {
        FUNC(state->pc + (i64) inst->imm, inst);
    }

    static void exec_jalr(state_t *state, inst_t *inst) {
        u64 rs1 = state->gp_regs[inst->rs1];
        FUNC((rs1 + (i64) inst->imm) & ~(u64)1, inst);
    }

    #undef FUNC

    /////////////////////////////////////////
    // Branch type instructions
    /////////////////////////////////////////

    #define FUNC(type, op) \
        bool result = ((type) state->gp_regs[inst->rs1]) op ((type) state->gp_regs[inst->rs2]); \
        if (result) { \
            state->exit_reason = indirect_branch; \
            state->reenter_pc = state->pc + (i64) inst->imm; \
            inst->cont = true; \
            if ((state->reenter_pc & 0x3) != 0) { \
                state->raise_exception = true; \
                state->exception_code = instruction_address_misaligned; \
                warning("Exception: instruction_address_misaligned"); \
            } \
        } \


    static void exec_beq(state_t *state, inst_t *inst) {
        FUNC(i64, ==);
    }

    static void exec_bne(state_t *state, inst_t *inst) {
        FUNC(i64, !=);
    }

    static void exec_blt(state_t *state, inst_t *inst) {
        FUNC(i64, <);
    }

    static void exec_bltu(state_t *state, inst_t *inst) {
        FUNC(u64, <);
    }

    static void exec_bge(state_t *state, inst_t *inst) {
        FUNC(i64, >=);
    }

    static void exec_bgeu(state_t *state, inst_t *inst) {
        FUNC(u64, >=);
    }

    #undef FUNC

    /////////////////////////////////////////
    // Load type instructions
    /////////////////////////////////////////

    #define FUNC(type) \
        u64 address = state->gp_regs[inst->rs1] + (i64) inst->imm; \
        state->gp_regs[inst->rd] =  *((type *) TO_HOST(address)); \


    static void exec_lb(state_t *state, inst_t *inst) {
        FUNC(i8);
    }

    static void exec_lh(state_t *state, inst_t *inst) {
        FUNC(i16);
    }

    static void exec_lw(state_t *state, inst_t *inst) {
        FUNC(i32);
    }

    static void exec_ld(state_t *state, inst_t *inst) {
        FUNC(i64);
    }

    static void exec_lbu(state_t *state, inst_t *inst) {
        FUNC(u8);
    }

    static void exec_lhu(state_t *state, inst_t *inst) {
        FUNC(u16);
    }

    static void exec_lwu(state_t *state, inst_t *inst) {
        FUNC(u32);
    }

    #undef FUNC

    /////////////////////////////////////////
    // Store type instructions
    /////////////////////////////////////////

    #define FUNC(type) \
        u64 address = state->gp_regs[inst->rs1] + (i64) inst->imm; \
        *((type *) TO_HOST(address)) = state->gp_regs[inst->rs2]; \


    static void exec_sb(state_t *state, inst_t *inst) {
        FUNC(i8);
    }

    static void exec_sh(state_t *state, inst_t *inst) {
        FUNC(i16);
    }

    static void exec_sw(state_t *state, inst_t *inst) {
        FUNC(i32);
    }

    static void exec_sd(state_t *state, inst_t *inst) {
        FUNC(i64);
    }

    #undef FUNC

    /////////////////////////////////////////
    // Zicsr instructions
    /////////////////////////////////////////

    static void exec_csrrw(state_t *state, inst_t *inst) {
        i64 rs1 = state->gp_regs[inst->rs1];
        u64 csr = (u64) state->csr[inst->csr];
        if (inst->rd) state->gp_regs[inst->rd] = csr;
        state->csr[inst->csr] = rs1;
    }

    static void exec_csrrs(state_t *state, inst_t *inst) {
        i64 rs1 = state->gp_regs[inst->rs1];
        u64 csr = (u64) state->csr[inst->csr];
        state->gp_regs[inst->rd] = csr;
        if (inst->rs1 != 0) {
            state->csr[inst->csr] = csr | rs1;
        }
    }

    static void exec_csrrc(state_t *state, inst_t *inst) {
        i64 rs1 = state->gp_regs[inst->rs1];
        u64 csr = (u64) state->csr[inst->csr];
        state->gp_regs[inst->rd] = csr;
        if (inst->rs1 != 0) state->csr[inst->csr] = csr & ~rs1;
    }

    static void exec_csrrwi(state_t *state, inst_t *inst) {
        u64 csr = (u64) state->csr[inst->csr];
        u32 imm = inst->imm;
        if (inst->rd != 0) state->gp_regs[inst->rd] = csr;
        state->csr[inst->csr] = imm;
    }

    static void exec_csrrsi(state_t *state, inst_t *inst) {
        u64 csr = (u64) state->csr[inst->csr];
        u32 imm = inst->imm;
        if (imm != 0) state->csr[inst->csr] = csr | imm;
        state->gp_regs[inst->rd] = csr;
    }

    static void exec_csrrci(state_t *state, inst_t *inst) {
        u64 csr = (u64) state->csr[inst->csr];
        u32 imm = inst->imm;
        if (imm != 0) state->csr[inst->csr] = csr & ~imm;
        state->gp_regs[inst->rd] = csr;
    }

    /////////////////////////////////////////
    // RVC Instructions
    /////////////////////////////////////////

    // Stack Point Based Load type instructions

    #define FUNC(type) \
        u64 address = state->gp_regs[2] + (u64) inst->imm; \
        state->gp_regs[inst->rd] =  *((type *) TO_HOST(address)); \


    static void exec_clwsp(state_t *state, inst_t *inst) {
        FUNC(i32);
    }

    static void exec_cldsp(state_t *state, inst_t *inst) {
        FUNC(i64);
    }

    #undef FUNC

    // Stack Point Based Store type instructions

    #define FUNC(type) \
        u64 address = state->gp_regs[2] + (u64) inst->imm; \
        *((type *) TO_HOST(address)) = state->gp_regs[inst->rs2]; \


    static void exec_cswsp(state_t *state, inst_t *inst) {
        FUNC(i32);
    }

    static void exec_csdsp(state_t *state, inst_t *inst) {
        FUNC(i64);
    }

    #undef FUNC

    // Register Based Load type instructions

    #define FUNC(type) \
        u64 address = state->gp_regs[inst->rs1] + (u64) inst->imm; \
        state->gp_regs[inst->rd] =  *((type *) TO_HOST(address)); \


    static void exec_clw(state_t *state, inst_t *inst) {
        FUNC(i32);
    }

    static void exec_cld(state_t *state, inst_t *inst) {
        FUNC(i64);
    }

    #undef FUNC

    // Register Based Store type instructions

    #define FUNC(type) \
        u64 address = state->gp_regs[inst->rs1] + (u64) inst->imm; \
        *((type *) TO_HOST(address)) = state->gp_regs[inst->rs2]; \


    static void exec_csw(state_t *state, inst_t *inst) {
        FUNC(i32);
    }

    static void exec_csd(state_t *state, inst_t *inst) {
        FUNC(i64);
    }

    #undef FUNC

    // Control Transfer instruction

    static void exec_cj(state_t *state, inst_t *inst) {
        state->exit_reason = direct_branch;
        state->reenter_pc = state->pc + (i64) inst->imm;
    }

    static void exec_cjr(state_t *state, inst_t *inst) {
        state->exit_reason = direct_branch;
        state->reenter_pc = state->pc + state->gp_regs[inst->rs1];
    }

    static void exec_cjalr(state_t *state, inst_t *inst) {
        state->exit_reason = direct_branch;
        state->reenter_pc = state->pc + state->gp_regs[inst->rs1];
        state->gp_regs[1] = state->pc + 2;
    }

    static void exec_cbeqz(state_t *state, inst_t *inst) {
        if (state->gp_regs[inst->rs1] == 0) {
            state->exit_reason = indirect_branch;
            state->reenter_pc = state->pc + (i64) inst->imm;
        }
    }

    static void exec_cbnez(state_t *state, inst_t *inst) {
        if (state->gp_regs[inst->rs1] != 0) {
            state->exit_reason = indirect_branch;
            state->reenter_pc = state->pc + (i64) inst->imm;
        }
    }

    // Integer Constant-Generation Instructions
    static void exec_cli(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) inst->imm;
    }

    static void exec_clui(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) inst->imm;
    }

    // Integer Register-Immediate Operations
    static void exec_caddi(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rs1] + (i64) inst->imm;
    }

    static void exec_caddiw(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) ((i32) state->gp_regs[inst->rs1] + (i32) inst->imm);
    }

    static void exec_caddi16sp(state_t *state, inst_t *inst) {
        state->gp_regs[2] = state->gp_regs[2] + (i64) inst->imm;
    }

    static void exec_caddi4spn(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[2] + (i64) inst->imm;
    }

    static void exec_cslli(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rd] << (inst->imm & 0x3F);
    }

    static void exec_csrli(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (u64) state->gp_regs[inst->rd] >> (inst->imm & 0x3F);
    }

    static void exec_csrai(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) state->gp_regs[inst->rd] >> (inst->imm & 0x3F);
    }

    static void exec_candi(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rd] & (i64) inst->imm;
    }

    // Integer Register-Register Instructions

    static void exec_cmv(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rs2];
    }

    static void exec_cadd(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rd] + state->gp_regs[inst->rs2];
    }

    static void exec_cand(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rd] & state->gp_regs[inst->rs2];
    }

    static void exec_cor(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rd] | state->gp_regs[inst->rs2];
    }

    static void exec_cxor(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rd] ^ state->gp_regs[inst->rs2];
    }

    static void exec_csub(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rd] ^ state->gp_regs[inst->rs2];
    }

    static void exec_caddw(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) ((i32) state->gp_regs[inst->rd] + (i32) state->gp_regs[inst->rs2]);
    }

    static void exec_csubw(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) ((i32) state->gp_regs[inst->rd] - (i32) state->gp_regs[inst->rs2]);
    }

    static void exec_cnop(state_t *state, inst_t *inst) {
        // Do nothing
    }

    /////////////////////////////////////////
    // Trap-Return instructions
    /////////////////////////////////////////

    static void exec_mret(state_t *state, inst_t *inst) {
        state->exit_reason = mret;
        state->reenter_pc = state->csr[mepc_id];
        csr_set(mstatus, mpv, state->csr[mstatus_id], 0x0);
        csr_set(mstatus, mpp, state->csr[mstatus_id], 0x0);
        u8 mpie = csr_get(mstatus, mpie, state->csr[mstatus_id]);
        csr_set(mstatus, mie, state->csr[mstatus_id], mpie);
        csr_set(mstatus, mpie, state->csr[mstatus_id], 0x1);
    }

    /////////////////////////////////////////
    // RV32F Instructions
    /////////////////////////////////////////

    static void exec_flw(state_t *state, inst_t *inst) {
        u64 addr = state->gp_regs[inst->rs1] + inst->imm;
        state->fp_regs[inst->rd].w = *((u32 *) TO_HOST(addr));

    }

    static void exec_fsw(state_t *state, inst_t *inst) {
        u64 addr = state->gp_regs[inst->rs1] + inst->imm;
        *((u32 *) TO_HOST(addr)) = state->fp_regs[inst->rs2].w;
    }

    #define FUNC(expr) \
        f32 rs1 = state->fp_regs[inst->rs1].f; \
        __attribute__((unused)) f32 rs2 = state->fp_regs[inst->rs2].f; \
        state->fp_regs[inst->rd].f = (expr);

    static void exec_fadd_s(state_t *state, inst_t *inst) {
        FUNC(rs1 + rs2);
    }

    static void exec_fsub_s(state_t *state, inst_t *inst) {
        FUNC(rs1 - rs2);
    }

    static void exec_fmul_s(state_t *state, inst_t *inst) {
        FUNC(rs1 * rs2);
    }

    static void exec_fdiv_s(state_t *state, inst_t *inst) {
        FUNC(rs1 / rs2);
    }

    static void exec_fsqrt_s(state_t *state, inst_t *inst) {
        FUNC(sqrt(rs1));
    }

    static void exec_fmin_s(state_t *state, inst_t *inst) {
        FUNC(sqrt(rs1 < rs2 ? rs1 : rs2));
    }

    static void exec_fmax_s(state_t *state, inst_t *inst) {
        FUNC(sqrt(rs1 > rs2 ? rs1 : rs2));
    }

    #undef FUNC


    #define FUNC(expr) \
        f32 rs1 = state->fp_regs[inst->rs1].f; \
        f32 rs2 = state->fp_regs[inst->rs2].f; \
        f32 rs3 = state->fp_regs[inst->rs3].f; \
        state->fp_regs[inst->rd].f = (expr);

    static void exec_fmadd_s(state_t *state, inst_t *inst) {
        FUNC(sqrt((rs1 * rs2) + rs3));
    }

    static void exec_fmsub_s(state_t *state, inst_t *inst) {
        FUNC(sqrt((rs1 * rs2) - rs3));
    }

    static void exec_fnmsub_s(state_t *state, inst_t *inst) {
        FUNC(sqrt(-(rs1 * rs2) + rs3));
    }


    static void exec_fnmadd_s(state_t *state, inst_t *inst) {
        FUNC(sqrt(-(rs1 * rs2) - rs3));
    }

    #undef FUNC

    /////////////////////////////////////////
    // RV32F Instructions
    /////////////////////////////////////////

    static void exec_fld(state_t *state, inst_t *inst) {
        u64 addr = state->gp_regs[inst->rs1] + inst->imm;
        state->fp_regs[inst->rd].v = *((u64 *) TO_HOST(addr));

    }

    static void exec_fsd(state_t *state, inst_t *inst) {
        u64 addr = state->gp_regs[inst->rs1] + inst->imm;
        *((u64 *) TO_HOST(addr)) = state->fp_regs[inst->rs2].v;
    }

    #define FUNC(expr) \
        f64 rs1 = state->fp_regs[inst->rs1].d; \
        __attribute__((unused)) f64 rs2 = state->fp_regs[inst->rs2].d; \
        state->fp_regs[inst->rd].d = (expr);

    static void exec_fadd_d(state_t *state, inst_t *inst) {
        FUNC(rs1 + rs2);
    }

    static void exec_fsub_d(state_t *state, inst_t *inst) {
        FUNC(rs1 - rs2);
    }

    static void exec_fmul_d(state_t *state, inst_t *inst) {
        FUNC(rs1 * rs2);
    }

    static void exec_fdiv_d(state_t *state, inst_t *inst) {
        FUNC(rs1 / rs2);
    }

    static void exec_fsqrt_d(state_t *state, inst_t *inst) {
        FUNC(sqrt(rs1));
    }

    static void exec_fmin_d(state_t *state, inst_t *inst) {
        FUNC(sqrt(rs1 < rs2 ? rs1 : rs2));
    }

    static void exec_fmax_d(state_t *state, inst_t *inst) {
        FUNC(sqrt(rs1 > rs2 ? rs1 : rs2));
    }

    #undef FUNC

    #define FUNC(expr) \
        f64 rs1 = state->fp_regs[inst->rs1].d; \
        f64 rs2 = state->fp_regs[inst->rs2].d; \
        f64 rs3 = state->fp_regs[inst->rs3].d; \
        state->fp_regs[inst->rd].d = (expr);

    static void exec_fmadd_d(state_t *state, inst_t *inst) {
        FUNC(sqrt((rs1 * rs2) + rs3));
    }

    static void exec_fmsub_d(state_t *state, inst_t *inst) {
        FUNC(sqrt((rs1 * rs2) - rs3));
    }

    static void exec_fnmsub_d(state_t *state, inst_t *inst) {
        FUNC(sqrt(-(rs1 * rs2) + rs3));
    }


    static void exec_fnmadd_d(state_t *state, inst_t *inst) {
        FUNC(sqrt(-(rs1 * rs2) - rs3));
    }

    #undef FUNC

    /////////////////////////////////////////
    // MISC instructions
    /////////////////////////////////////////

    static void exec_fence(state_t *state, inst_t *inst) {
        warning("unimplemented FENCE instructions");
    }

    static void exec_ecall(state_t *state, inst_t *inst) {
        state->exit_reason = ecall;
    }

    static void exec_ebreak(state_t *state, inst_t *inst) {
        fatal("unimplemented EBREAK instructions");
    }



/////////////////////////////////////////
// function pointers array
/////////////////////////////////////////

static func_t *funcs[] = {
#include "funcs.h"
};

/////////////////////////////////////////
// Execute the instruction
/////////////////////////////////////////


/**
 * @brief execute the code by interpretation
 *
 * @param state CPU state
 */
void exec_block_interp(state_t *state) {
    static inst_t inst = {0};
    while (true) {
        // Get the instruction pointed by PC.
        // We already mapped the ELF file to the host memory space
        // so we can map the pc address to the host address and then
        // use indirection operator (*) to get the data pointed by the pc
        // in the host memory space which is the instruction pointed by the PC.
        u64 raw_inst = *(u64 *) TO_HOST(state->pc);

        // decode the instruction
        inst_decode(&inst, raw_inst);



        // execute the instruction
        funcs[inst.type](state, &inst);
        //funcs[0](state, &inst);

        // revert back register zero value.
        state->gp_regs[zero] = 0;

        // per instruction debug
        #ifdef DEBUG
            printf("Current PC: %lx. ", state->pc);
            printf("Current Instruction 64: %16lx. ", raw_inst);
            printf("Current Instruction 32: %8x. ",(u32) raw_inst);
            printf("Decoded Instruction: %d.\n", inst.type);

            // Add more debug code if needed
            printreg(0x800001ac, ra);
            printreg(0x800001b0, sp);
            printreg(0x800001b4, ra);
            printreg(0x800001b4, sp);
            printreg(0x800001b4, a4);

        #endif


        if (inst.cont) break;

        // advance pc
        state->pc += inst.rvc ? 2 : 4;
        //printf("Next PC is %lx\n", state->pc); // For debug only
    }
}
