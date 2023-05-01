#include "rvemu.h"


#define QUADRANT(data) (((data) >> 0) & 0x3)

/////////////////////////////////////////
// Macros to decode instructions
/////////////////////////////////////////

// ignored lower 2 bits for the opcode field
#define OPCODE(data)    (((data) >> 2)  & 0x1F)
#define RD(data)        (((data) >> 7)  & 0x1F)
#define RS1(data)       (((data) >> 15) & 0x1F)
#define RS2(data)       (((data) >> 20) & 0x1F)
#define RS3(data)       (((data) >> 27) & 0x1F)
#define FUNCT2(data)    (((data) >> 25) & 0x3)
#define FUNCT3(data)    (((data) >> 12) & 0x7)
#define FUNCT7(data)    (((data) >> 25) & 0x7F)
#define IMM116(data)    (((data) >> 26) & 0x3F)

/////////////////////////////////////////
// Functions to decode instruction
/////////////////////////////////////////

// RV32I base Instruction Set

/**
 * @brief Read LUI and AUIPC instruction
 */
static inline inst_t inst_lui_auipc_read(u32 data) {
    return (inst_t) {
        .imm = (data >> 20) << 12,
        .rd = RD(data),
    };
}

/**
 * @brief Read JAL instruction
 */
static inline inst_t inst_jal_read(u32 data) {
    i32 imm = 0; // FIXME
    return (inst_t) {
        .imm = imm,
        .rd = RD(data),
    };
}

/**
 * @brief Read JALR instruction
 */
static inline inst_t inst_jalr_read(u32 data) {
    return (inst_t) {
        .imm = (i32) data >> 20,
        .rs1 = RS1(data),
        .rd = RD(data),
    };
}

/**
 * @brief Read Branch instruction
 */
static inline inst_t inst_branch_read(u32 data) {
    i32 imm = 0; // FIXME
    return (inst_t) {
        .imm = imm,
        .rs1 = RS1(data),
        .rs2 = RS2(data),
    };
}

/**
 * @brief Read Load instruction
 */
static inline inst_t inst_load_read(u32 data) {
    return (inst_t) {
        .imm = (i32) data >> 20,
        .rs1 = RS1(data),
        .rd = RD(data),
    };
}

/**
 * @brief Read Store instruction
 */
static inline inst_t inst_store_read(u32 data) {
    i32 imm = 0; // FIXME
    return (inst_t) {
        .imm = imm,
        .rs1 = RS1(data),
        .rs2 = RS2(data),
    };
}

/**
 * @brief Read itype instruction
 */
static inline inst_t inst_itype_read(u32 data) {
    return (inst_t) {
        .imm = (i32) data >> 20,
        .rs1 = RS1(data),
        .rd = RD(data),
    };
}

/**
 * @brief Read arithmetic/logic instruction
 */
static inline inst_t inst_arithmetic_read(u32 data) {
    return (inst_t) {
        .rs1 = RS1(data),
        .rs2 = RS2(data),
        .rd = RD(data),
    };
}

/**
 * @brief decode the instruction
 *
 * @param inst      instruction struct
 * @param raw_inst  raw instruction from ELF file
 */
void inst_decode(inst_t *inst, u32 raw_inst) {
    // Quadrant 2'b11 mean non-compressed instruction
    // Refer to riscv-spec-20191213.pdf 16.8 RVC Instruction Set Listings
    u32 quadrant = QUADRANT(raw_inst);

    u8 opcode = OPCODE(raw_inst);
    u8 funct3 = FUNCT3(raw_inst);
    u8 funct7 = FUNCT7(raw_inst);

    switch(quadrant) {
        case 0x0: fatal("unimplemented");
        case 0x1: fatalf("unimplemented. func3 = %x", funct3);
        case 0x2: fatal("unimplemented");
        case 0x3: {

            switch(opcode) {

                case 0xD:   // RV32I - LUI
                            // fall through to AUIPC
                case 0x5: { // RV32I - AUIPC
                    *inst = inst_lui_auipc_read(raw_inst);
                    inst->type = inst_lui;
                    return;
                }

                case 0x1B: { // RV32I - JAL
                    *inst = inst_jal_read(raw_inst);
                    inst->type = inst_jal;
                    return;
                }

                case 0x19: { // RV32I - JALR
                    *inst = inst_jalr_read(raw_inst);
                    inst->type = inst_jalr;
                    return;
                }

                case 0x18: { // RV32I - Branch
                    *inst = inst_branch_read(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_beq; return;    // RV32I - BEQ
                        case 0x1: inst->type = inst_bne; return;    // RV32I - BNE
                        case 0x4: inst->type = inst_blt; return;    // RV32I - BLE
                        case 0x5: inst->type = inst_bge; return;    // RV32I - BGE
                        case 0x6: inst->type = inst_bltu; return;   // RV32I - BLTU
                        case 0x7: inst->type = inst_bgeu; return;   // RV32I - BGEU
                        default: fatal("Not a valid branch type instruction");
                    }
                }

                case 0x0: {
                    *inst = inst_load_read(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_lb; return;     // RV32I - LB
                        case 0x1: inst->type = inst_lh; return;     // RV32I - LH
                        case 0x2: inst->type = inst_lw; return;     // RV32I - LW
                        case 0x4: inst->type = inst_lbu; return;    // RV32I - LBU
                        case 0x5: inst->type = inst_lhu; return;    // RV32I - LHU
                        default: fatal("Not a valid load type instruction");
                    }
                }

                case 0x8: {
                    *inst = inst_store_read(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_sb; return;     // RV32I - SB
                        case 0x1: inst->type = inst_sh; return;     // RV32I - SH
                        case 0x2: inst->type = inst_sw; return;     // RV32I - SW
                        default: fatal("Not a valid store type instruction");
                    }
                }

                case 0x4: {
                    *inst = inst_itype_read(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_addi;  return; // RV32I - ADDI
                        case 0x2: inst->type = inst_slti;  return; // RV32I - SLTI
                        case 0x3: inst->type = inst_sltiu; return; // RV32I - SLTIU
                        case 0x4: inst->type = inst_xori;  return; // RV32I - XORI
                        case 0x6: inst->type = inst_ori;   return; // RV32I - ORI
                        case 0x7: inst->type = inst_andi;  return; // RV32I - ANDI
                        case 0x1: inst->type = inst_slli;  return; // RV32I - SLLI
                        case 0x5: {
                            if (funct7 == 0x0)       {inst->type = inst_srli; return;} // RV32I - SRLI
                            else if (funct7 == 0x20) {inst->type = inst_srai; return;} // RV32I - SRAI
                            else                     {fatal("Not a valid itype instruction");}
                        }
                        default: fatal("unimplemented");
                    }
                }

                case 0xC: {
                    *inst = inst_arithmetic_read(raw_inst);
                    switch (funct3) {
                        case 0x0: {
                            if (funct7 == 0x0)       {inst->type = inst_add; return;} // RV32I - ADD
                            else if (funct7 == 0x20) {inst->type = inst_sub; return;} // RV32I - SUB
                            else                     {fatal("Not a valid arithmetic instruction");}
                        }
                        case 0x1: inst->type = inst_sll;  return; // RV32I - SLT
                        case 0x2: inst->type = inst_slt;  return; // RV32I - SLT
                        case 0x3: inst->type = inst_sltu; return; // RV32I - SLTU
                        case 0x4: inst->type = inst_xor;  return; // RV32I - XOR
                        case 0x5: {
                            if (funct7 == 0x0)       {inst->type = inst_srl; return;} // RV32I - SRL
                            else if (funct7 == 0x20) {inst->type = inst_sra; return;} // RV32I - SRA
                            else                     {fatal("Not a valid itype instruction");}
                        }
                        case 0x6: inst->type = inst_or;  return; // RV32I - OR
                        case 0x7: inst->type = inst_and; return; // RV32I - AND
                        default: fatal("unimplemented");
                    }
                }

                case 0x3: {
                    fatal("unimplemented fence instruction");
                }

                case 0x1C: {
                    fatal("unimplemented ecall/ebreak instruction");
                }

                default: fatal("unimplemented");
            }

        }
        default: unreachable();
    }
}