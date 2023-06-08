#include "rvemu.h"

typedef void (func_t)(state_t *, inst_t *);

/////////////////////////////////////////
// Functions to execute instruction
/////////////////////////////////////////

// ADDI/SLTI(U)/ANDI/ORI/XORI/ADDIW

    // Macros for i-type instruction
    #define EXEC_MACRO(type, op) state->gp_regs[inst->rd] = (type) state->gp_regs[inst->rs1] op (type) inst->imm \


    // addi instruction
    static void exec_addi(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, +);
    }

    // slti instruction
    static void exec_slti(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, <);
    }

    // sltiu instruction
    static void exec_sltiu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, <);
    }

    // andi instruction
    static void exec_andi(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, &);
    }

    // ori instruction
    static void exec_ori(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, |);
    }

    // xori instruction
    static void exec_xori(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, ^);
    }

    // addi instruction
    static void exec_addiw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, +);
    }

    #undef EXEC_MACRO

// SLLI/SRLI/SRAI/SLLIW/SRLIW/SRAIW

    // Macros for shift instruction
    #define EXEC_MACRO(type, op, mask) state->gp_regs[inst->rd] = (type) state->gp_regs[inst->rs1] op (inst->imm & (mask)) \


    // slli instruction
    static void exec_slli(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, <<, 0x3F);
    }

    // srli instruction
    static void exec_srli(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, >>, 0x3F);
    }

    // srai instruction
    static void exec_srai(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, >>, 0x3F);
    }

    // slliw instruction
    static void exec_slliw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, <<, 0x1F);
    }

    // srliw instruction
    static void exec_srliw(state_t *state, inst_t *inst) {
        EXEC_MACRO(u32, >>, 0x1F);
    }

    // sraiw instruction
    static void exec_sraiw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, >>, 0x1F);
    }

    #undef EXEC_MACRO

// LUI/AUIPC - page 19

    // lui instruction
    static void exec_lui(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) inst->imm;
    }

    // auipc instruction
    static void exec_auipc(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i64) inst->imm + state->pc;
    }

// ADD/SLT/SLTU/AND/OR/XOR/SUB/ADDW/SUBW

    // Macros for r-type instruction
    #define EXEC_MACRO(type, op) \
        state->gp_regs[inst->rd] =(i64) (type) state->gp_regs[inst->rs1] op (type) state->gp_regs[inst->rs2] \


    // add instruction
    static void exec_add(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, +);
    }

    // slt instruction
    static void exec_slt(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, <);
    }

    // sltu instruction
    static void exec_sltu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, <);
    }

    // and instruction
    static void exec_and(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, &);
    }

    // or instruction
    static void exec_or(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, |);
    }

    // xor instruction
    static void exec_xor(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, ^);
    }

    // sub instruction
    static void exec_sub(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, -);
    }

    // addw instruction
    static void exec_addw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, +);
    }

    // subw instruction
    static void exec_subw(state_t *state, inst_t *inst) {
        EXEC_MACRO(u32, -);
    }

    #undef EXEC_MACRO

// SLL/SRL/SRA/SLLW/SRLW/SRAW

    // Macros for shift instruction
    #define EXEC_MACRO(type, op, mask) \
        state->gp_regs[inst->rd] = (type) state->gp_regs[inst->rs1] op (state->gp_regs[inst->rs2] & mask) \

    // sll instruction
    static void exec_sll(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, <<, 0x3F);
    }

    // srl instruction
    static void exec_srl(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, >>, 0x3F);
    }

    // sra instruction
    static void exec_sra(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, >>, 0x3F);
    }

    // sllw instruction
    static void exec_sllw(state_t *state, inst_t *inst) {
        EXEC_MACRO(u32, <<, 0x1F);
    }

    // srlw instruction
    static void exec_srlw(state_t *state, inst_t *inst) {
        EXEC_MACRO(u32, >>, 0x1F);
    }

    // sraw instruction
    static void exec_sraw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, >>, 0x1F);
    }

    #undef EXEC_MACRO


// JAL/JALR

    #define EXEC_MACRO(expr) \
        state->gp_regs[inst->rd] = state->pc + 4; \
        state->exit_reason = direct_branch; \
        state->reenter_pc = (expr); \
        if ((state->reenter_pc & 0x3) != 0) { \
            state->raise_exception = true; \
            state->exception_code = instruction_address_misaligned; \
        } \


    static void exec_jal(state_t *state, inst_t *inst) {
        EXEC_MACRO(state->pc + (i64) inst->imm);
    }

    static void exec_jalr(state_t *state, inst_t *inst) {
        EXEC_MACRO((state->gp_regs[inst->rs1] + (i64) inst->imm) & 0xFFFFFFFFFFFFFFFE);
    }

    #undef EXEC_MACRO

// BEQ/BNE/BLT(U)/BGE(U)

    #define EXEC_MACRO(type, op) \
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
        EXEC_MACRO(i64, ==);
    }

    static void exec_bne(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, !=);
    }

    static void exec_blt(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, <);
    }

    static void exec_bltu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, <);
    }

    static void exec_bge(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, >=);
    }

    static void exec_bgeu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, >=);
    }

    #undef EXEC_MACRO

// LB/LH/LW/LD/LBU/LHU/LWU

    #define EXEC_MACRO(type) \
        u64 address = state->gp_regs[inst->rs1] + (i64) inst->imm; \
        state->gp_regs[inst->rd] =  *((type *) TO_HOST(address)); \


    static void exec_lb(state_t *state, inst_t *inst) {
        EXEC_MACRO(i8);
    }

    static void exec_lh(state_t *state, inst_t *inst) {
        EXEC_MACRO(i16);
    }

    static void exec_lw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32);
    }

    static void exec_ld(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64);
    }

    static void exec_lbu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u8);
    }

    static void exec_lhu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u16);
    }

    static void exec_lwu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u32);
    }

    #undef EXEC_MACRO

// SB/SH/SW/SD

    #define EXEC_MACRO(type) \
        u64 address = state->gp_regs[inst->rs1] + (i64) inst->imm; \
        *((type *) TO_HOST(address)) = state->gp_regs[inst->rs2]; \


    static void exec_sb(state_t *state, inst_t *inst) {
        EXEC_MACRO(i8);
    }

    static void exec_sh(state_t *state, inst_t *inst) {
        EXEC_MACRO(i16);
    }

    static void exec_sw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32);
    }

    static void exec_sd(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64);
    }

    #undef EXEC_MACRO

// MUL/MULH/MULHSU/MULHU/DIV/DIVU/REM/REMU/MULW/DIVW/DIVUW/REMW/REMUW

    #define EXEC_MACRO(type, op) \
        state->gp_regs[inst->rd] =(i64) (type) state->gp_regs[inst->rs1] op (type) state->gp_regs[inst->rs2] \

    static void exec_mul(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, *);
    }

    static void exec_mulh(state_t *state, inst_t *inst) {
        fatal("unimplemented MULH instructions");
    }

    static void exec_mulhsu(state_t *state, inst_t *inst) {
        fatal("unimplemented MULHSU instructions");
    }

    static void exec_mulhu(state_t *state, inst_t *inst) {
        fatal("unimplemented MULHU instructions");
    }

    static void exec_div(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, /);
    }

    static void exec_divu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, /);
    }

    static void exec_rem(state_t *state, inst_t *inst) {
        EXEC_MACRO(i64, %);
    }

    static void exec_remu(state_t *state, inst_t *inst) {
        EXEC_MACRO(u64, %);
    }

    static void exec_mulw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, *);
    }

    static void exec_divw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, /);
    }

    static void exec_divuw(state_t *state, inst_t *inst) {
        EXEC_MACRO(u32, /);
    }

    static void exec_remw(state_t *state, inst_t *inst) {
        EXEC_MACRO(i32, %);
    }

    static void exec_remuw(state_t *state, inst_t *inst) {
        EXEC_MACRO(u32, %);
    }

    #undef EXEC_MACRO

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
