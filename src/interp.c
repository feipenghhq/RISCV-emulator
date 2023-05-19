#include "rvemu.h"

typedef void (func_t)(state_t *, inst_t *);

/////////////////////////////////////////
// Functions to execute instruction
/////////////////////////////////////////

// ADDI/SLTI(U)/ANDI/ORI/XORI - Page 18

    // Macros for i-type instruction
    #define EXEC_MACRO_UNSIGNED(op) \
        state->gp_regs[inst->rd] = (u32) state->gp_regs[inst->rs1] op (u32) inst->imm

    #define EXEC_MACRO_SIGNED(op) \
        state->gp_regs[inst->rd] = (i32) state->gp_regs[inst->rs1] op (i32) inst->imm

    // addi instruction
    static void exec_addi(state_t *state, inst_t *inst) {
        EXEC_MACRO_SIGNED(+);
    }

    // slti instruction
    static void exec_slti(state_t *state, inst_t *inst) {
        EXEC_MACRO_SIGNED(<);
    }

    // sltiu instruction
    static void exec_sltiu(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(<);
    }

    // andi instruction
    static void exec_andi(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(&);
    }

    // ori instruction
    static void exec_ori(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(|);
    }

    // xori instruction
    static void exec_xori(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(^);
    }

    #undef EXEC_MACRO_UNSIGNED
    #undef EXEC_MACRO_SIGNED

// SLLI/SRLI/SRAI - Page 18

    // slli instruction
    static void exec_slli(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rs1] << (inst->imm & 0x1F);
    }

    // srli instruction
    static void exec_srli(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (u32) state->gp_regs[inst->rs1] >> (inst->imm & 0x1F);
    }

    // srai instruction
    static void exec_srai(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i32) state->gp_regs[inst->rs1] >> (inst->imm & 0x1F);
    }

// LUI/AUIPC - page 19

    // lui instruction
    static void exec_lui(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = inst->imm;
    }

    // auipc instruction
    static void exec_auipc(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = inst->imm + state->pc;
    }

// ADD/SLT/SLTU/AND/OR/XOR/SLL/SRL/SUB/SRA - page 9

    // Macros for r-type instruction
    #define EXEC_MACRO_UNSIGNED(op) \
        state->gp_regs[inst->rd] = (u32) state->gp_regs[inst->rs1] op (u32) state->gp_regs[inst->rs2]

    #define EXEC_MACRO_SIGNED(op) \
        state->gp_regs[inst->rd] = (i32) state->gp_regs[inst->rs1] op (i32) state->gp_regs[inst->rs2]

    // add instruction
    static void exec_add(state_t *state, inst_t *inst) {
        EXEC_MACRO_SIGNED(+);
    }

    // slt instruction
    static void exec_slt(state_t *state, inst_t *inst) {
        EXEC_MACRO_SIGNED(<);
    }

    // sltu instruction
    static void exec_sltu(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(<);
    }

    // and instruction
    static void exec_and(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(&);
    }

    // or instruction
    static void exec_or(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(|);
    }

    // xor instruction
    static void exec_xor(state_t *state, inst_t *inst) {
        EXEC_MACRO_UNSIGNED(^);
    }

    // sll instruction
    static void exec_sll(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = state->gp_regs[inst->rs1] << (state->gp_regs[inst->rs2] & 0x1F);
    }

    // srl instruction
    static void exec_srl(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (u32) state->gp_regs[inst->rs1] >> (state->gp_regs[inst->rs2] & 0x1F);
    }

    // sub instruction
    static void exec_sub(state_t *state, inst_t *inst) {
        EXEC_MACRO_SIGNED(-);
    }

    // sra instruction
    static void exec_sra(state_t *state, inst_t *inst) {
        state->gp_regs[inst->rd] = (i32) state->gp_regs[inst->rs1] >> (state->gp_regs[inst->rs2] & 0x1F);
    }

    #undef EXEC_MACRO_UNSIGNED
    #undef EXEC_MACRO_SIGNED

/////////////////////////////////////////
// function pointers array
/////////////////////////////////////////
// FIXME
static func_t *funcs[] = {
    exec_addi,
    exec_slti,
    exec_sltiu,
    exec_andi,
    exec_ori,
    exec_xori,
    exec_slli,
    exec_srli,
    exec_srai,
    exec_lui,
    exec_auipc,
    exec_add,
    exec_slt,
    exec_sltu,
    exec_and,
    exec_or,
    exec_xor,
    exec_sll,
    exec_srl,
    exec_sub,
    exec_sra,
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
        u32 raw_inst = *(u32 *) TO_HOST(state->pc);

        // decode the instruction
        inst_decode(&inst, raw_inst);

        // FIXME
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
