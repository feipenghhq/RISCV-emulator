#include "rvemu.h"

typedef void (func_t)(state_t *, inst_t *);

/////////////////////////////////////////
// Functions to execute instruction
/////////////////////////////////////////

    /////////////////////////////////////////
    // Reg / Imm type instructions
    /////////////////////////////////////////

    #define FUNC(expr) \
        i64 rs1 = state->gp_regs[inst->rs1]; \
        i64 imm = (i64) inst->imm; \
        state->gp_regs[inst->rd] = (i64) (expr); \

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

    // addi instruction
    static void exec_addiw(state_t *state, inst_t *inst) {
        FUNC(rs1 + imm);
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
        FUNC((i32) rs1 << (imm & 0x1F));
    }

    // srliw instruction
    static void exec_srliw(state_t *state, inst_t *inst) {
        FUNC((u32) rs1 >> (imm & 0x1F));
    }

    // sraiw instruction
    static void exec_sraiw(state_t *state, inst_t *inst) {
        FUNC((i32) rs1 >> (imm & 0x1F));
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
        FUNC((i32) rs1 + (i32) rs2);
    }

    // subw instruction
    static void exec_subw(state_t *state, inst_t *inst) {
        FUNC((i32) rs1 - (i32) rs2);
    }

    // sll instruction
    static void exec_sll(state_t *state, inst_t *inst) {
        FUNC(rs1 << (rs2 & 0x3F));
    }

    // srl instruction
    static void exec_srl(state_t *state, inst_t *inst) {
        FUNC((u64) rs1 << (rs2 & 0x3F));
    }

    // sra instruction
    static void exec_sra(state_t *state, inst_t *inst) {
        FUNC(rs1 << (rs2 & 0x3F));
    }

    // sllw instruction
    static void exec_sllw(state_t *state, inst_t *inst) {
        FUNC((u32) rs1 << (rs2 & 0x1F));
    }

    // srlw instruction
    static void exec_srlw(state_t *state, inst_t *inst) {
        FUNC((u32) rs1 << (rs2 & 0x1F));
    }

    // sraw instruction
    static void exec_sraw(state_t *state, inst_t *inst) {
        FUNC((i32) rs1 << (rs2 & 0x1F));
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
        FUNC((i32) rs1 * (i32) rs2);
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

    #define FUNC(expr) \
        state->gp_regs[inst->rd] = state->pc + 4; \
        state->exit_reason = direct_branch; \
        state->reenter_pc = (expr); \
        if ((state->reenter_pc & 0x3) != 0) { \
            state->raise_exception = true; \
            state->exception_code = instruction_address_misaligned; \
        } \


    static void exec_jal(state_t *state, inst_t *inst) {
        FUNC(state->pc + (i64) inst->imm);
    }

    static void exec_jalr(state_t *state, inst_t *inst) {
        FUNC((state->gp_regs[inst->rs1] + (i64) inst->imm) & 0xFFFFFFFFFFFFFFFE);
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
            if ((state->reenter_pc & 0x3) != 0) { \
                state->raise_exception = true; \
                state->exception_code = instruction_address_misaligned; \
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

    #define FUNC(cond, expr) \
        i64 rs1 = state->gp_regs[inst->rs1]; \
        u64 csr = (u64) state->csr[inst->csr]; \
        if (cond != 0) { \
            state->gp_regs[inst->rd] = csr; \
            state->csr[inst->csr] = (expr); \
        } \

    static void exec_csrrw(state_t *state, inst_t *inst) {
        FUNC(inst->rd, rs1);
    }

    static void exec_csrrs(state_t *state, inst_t *inst) {
        FUNC(inst->rs1, csr | rs1);
    }

    static void exec_csrrc(state_t *state, inst_t *inst) {
        FUNC(inst->rs1, csr & ~rs1);
    }

    #undef FUNC

    #define FUNC(cond, expr) \
        u64 csr = (u64) state->csr[inst->csr]; \
        u32 imm = inst->imm; \
        if (cond != 0) { \
            state->gp_regs[inst->rd] = csr; \
            state->csr[inst->csr] = (expr); \
        } \

    static void exec_csrrwi(state_t *state, inst_t *inst) {
        FUNC(inst->rd, imm);
    }

    static void exec_csrrsi(state_t *state, inst_t *inst) {
        FUNC(imm, csr | imm);
    }

    static void exec_csrrci(state_t *state, inst_t *inst) {
        FUNC(imm, csr & ~imm);
    }

    #undef FUNC

// MISC instructions

    static void exec_fence(state_t *state, inst_t *inst) {
        fatal("unimplemented FENCE instructions");
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
    exec_lui,
    exec_auipc,
    exec_jal,
    exec_jalr,
    exec_beq,
    exec_bne,
    exec_blt,
    exec_bge,
    exec_bltu,
    exec_bgeu,
    exec_lb,
    exec_lh,
    exec_lw,
    exec_lbu,
    exec_lhu,
    exec_sb,
    exec_sh,
    exec_sw,
    exec_add,
    exec_slt,
    exec_sltu,
    exec_addi,
    exec_slti,
    exec_sltiu,
    exec_xori,
    exec_ori,
    exec_andi,
    exec_slli,
    exec_srli,
    exec_srai,
    exec_add,
    exec_sub,
    exec_sll,
    exec_slt,
    exec_sltu,
    exec_xor,
    exec_srl,
    exec_sra,
    exec_or,
    exec_and,
    exec_fence,
    exec_ecall,
    exec_ebreak,
    exec_lwu,
    exec_ld,
    exec_sd,
    exec_addiw,
    exec_slliw,
    exec_srliw,
    exec_sraiw,
    exec_addw,
    exec_subw,
    exec_sllw,
    exec_srlw,
    exec_sraw,
    exec_mul,
    exec_mulh,
    exec_mulhsu,
    exec_mulhu,
    exec_div,
    exec_divu,
    exec_rem,
    exec_remu,
    exec_mulw,
    exec_divw,
    exec_divuw,
    exec_remw,
    exec_remuw,
    exec_csrrw,
    exec_csrrs,
    exec_csrrc,
    exec_csrrwi,
    exec_csrrsi,
    exec_csrrci,
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
        //funcs[inst.type](state, &inst);
        funcs[0](state, &inst);

        // revert back register zero value.
        state->gp_regs[zero] = 0;

        if (inst.cont) break;

        // advance pc
        state-> pc += inst.rvc ? 2 : 4;
    }
}
