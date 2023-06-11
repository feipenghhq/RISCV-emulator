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
#define INST31_20(data) (((data) >> 20) & 0xFFF)
#define C_FUNCT3(data)  (((data) >> 13) & 0x7)
#define C_FUNCT4(data)  (((data) >> 12) & 0xF)
#define C_RS1(data)     (((data) >> 7) & 0x1F)
#define C_RS2(data)     (((data) >> 2) & 0x1F)
#define C_RS1_(data)    (((data) >> 7) & 0x7)
#define C_RS2_(data)    (((data) >> 2) & 0x7)
#define C_RD_(data)     (((data) >> 2) & 0x7)

/////////////////////////////////////////
// Functions to decode instruction
/////////////////////////////////////////

// Extract different fields from instruction
// Read page 16 of RISC-V Unprivileged ISA V20191213

// Macros to help extract the immediate values from instruction
#define IMM_MASK(imm_h, imm_l) ((1 << ((imm_h) - (imm_l) + 1)) - 1)
// Extract immediate values from instruction and do an unsigned extension
#define EXTRACT_IMM_UNSIGNED(inst, imm_h, imm_l, inst_l) \
    (((u32) (inst >> (inst_l)) & IMM_MASK(imm_h, imm_l)) << (imm_l))
// Extract immediate values from instruction and do a signed extension
#define EXTRACT_IMM_SIGNED(inst, imm_h, imm_l, inst_l) \
    (((i32) (inst >> (inst_l)) & IMM_MASK(imm_h, imm_l)) << (imm_l))

// Macros for invalid instructions
#define INVALID_INST() fatalf("Invalid Instruction: %x", raw_inst)

/**
 * @brief R type instructions
 *
 * @param inst
 * @return inst_t
 */
static inline inst_t inst_r_type(u32 inst) {
    return (inst_t) {
        .rs1 = RS1(inst),
        .rs2 = RS2(inst),
        .rd = RD(inst),
    };
}

/**
 * @brief I type instructions
 *
 * @param inst
 * @return inst_t
 */
static inline inst_t inst_i_type(u32 inst) {
    i32 imm = 0;
    imm = imm | EXTRACT_IMM_SIGNED(inst, 11, 0, 20);   // imm[11:0] -> inst[31:20] - 12 bits
    return (inst_t) {
        .imm = imm,
        .rs1 = RS1(inst),
        .rd = RD(inst),
    };
}

/**
 * @brief S type instructions
 *
 * @param inst
 * @return inst_t
 */
static inline inst_t inst_s_type(u32 inst) {
    i32 imm = 0;
    imm = imm | EXTRACT_IMM_UNSIGNED(inst, 4, 0, 7);    // imm[4:0]  - inst[11:7]  - 5 bits
    imm = imm | EXTRACT_IMM_SIGNED(inst, 11, 5, 25);    // imm[11:5] - inst[31:25] - 7 bits
    return (inst_t) {
        .imm = imm,
        .rs1 = RS1(inst),
        .rs2 = RS2(inst),
    };
}

/**
 * @brief B type instructions
 *
 * @param inst
 * @return inst_t
 */
static inline inst_t inst_b_type(u32 inst) {
    i32 imm = 0;
    imm = imm | EXTRACT_IMM_UNSIGNED(inst, 4, 1, 8);    // imm[4:1]  - inst[11:8]  - 4 bits
    imm = imm | EXTRACT_IMM_UNSIGNED(inst, 10, 5, 25);  // imm[10:5] - inst[30:25] - 6 bits
    imm = imm | EXTRACT_IMM_UNSIGNED(inst, 11, 11, 7);  // imm[11]   - inst[7]     - 1 bits
    imm = imm | EXTRACT_IMM_SIGNED(inst, 12, 12, 31);   // imm[12]   - inst[31]    - 1 bits
    return (inst_t) {
        .imm = imm,
        .rs1 = RS1(inst),
        .rs2 = RS2(inst),
    };
}

/**
 * @brief U type instructions
 *
 * @param inst
 * @return inst_t
 */
static inline inst_t inst_u_type(u32 inst) {
    i32 imm = 0;
    imm = imm | EXTRACT_IMM_SIGNED(inst, 31, 12, 12);    // imm[31:12]  - inst[31:12]  - 20 bits
    return (inst_t) {
        .imm = imm,
        .rd = RD(inst),
    };
}

/**
 * @brief J type instructions
 *
 * @param inst
 * @return inst_t
 */
static inline inst_t inst_j_type(u32 inst) {
    i32 imm = 0;
    imm = imm | EXTRACT_IMM_UNSIGNED(inst, 10, 1, 21);      // imm[10:1]  - inst[30:21] - 10 bits
    imm = imm | EXTRACT_IMM_UNSIGNED(inst, 11, 11, 20);     // imm[11]    - inst[20]    - 1 bits
    imm = imm | EXTRACT_IMM_UNSIGNED(inst, 19, 12, 12);     // imm[19:12] - inst[19:12] - 8 bits
    imm = imm | EXTRACT_IMM_SIGNED(inst, 20, 20, 31);       // imm[20]    - inst[31]    - 1 bits
    return (inst_t) {
        .imm = imm,
        .rd = RD(inst),
    };
}

/**
 * @brief Zicsr type instructions
 *
 * @param inst
 * @return inst_t
 */
static inline inst_t inst_csr_type(u32 inst) {
    u32 imm = EXTRACT_IMM_UNSIGNED(inst, 4, 0, 15);
    return (inst_t) {
        .imm = imm,
        .csr = (inst >> 20) & 0xFFF,
        .rs1 = RS1(inst),
        .rd = RD(inst),
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

    // extract different field from instructions
    u8 opcode = OPCODE(raw_inst);
    u8 funct3 = FUNCT3(raw_inst);
    u8 funct7 = FUNCT7(raw_inst);
    u16 inst_31_20 = INST31_20(raw_inst);
    u8 c_funct3 = C_FUNCT3(raw_inst);
    //u8 c_funct4 = C_FUNCT4(raw_inst);

    switch(quadrant) {
        case 0x0: {
            inst->rvc = true;

            switch(c_funct3) {

                case 0x2: { // RVC - C.LW
                    inst->type = inst_clw;
                    inst->rd = C_RD_(raw_inst);
                    inst->rs1 = C_RS1_(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 2, 2, 6);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 3, 10);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 6, 6, 5);
                    break;
                }

                case 0x3: { // RVC - C.LD
                    inst->type = inst_cld;
                    inst->rd = C_RD_(raw_inst);
                    inst->rs1 = C_RS1_(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 3, 10);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 7, 6, 5);
                    break;
                }


                case 0x6: { // RVC - C.SW
                    inst->type = inst_csw;
                    inst->rs1 = C_RS1_(raw_inst);
                    inst->rs2 = C_RS2_(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 2, 2, 6);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 3, 10);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 6, 6, 5);
                    break;
                }

                case 0x7: { // RVC - C.SD
                    inst->type = inst_csd;
                    inst->rs1 = C_RS1_(raw_inst);
                    inst->rs2 = C_RS2_(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 3, 10);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 7, 6, 5);
                    break;
                }

                default: fatalf("unimplemented. Instruction = %x", raw_inst);

            }

            break;
        }

        case 0x1: {
            inst->rvc = true;

            switch(c_funct3) {

                case 0x5: { // RVC - J
                    inst->type = inst_cj;
                    u16 imm = 0;
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 11, 11, 12);
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 4, 4, 11);
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 9, 8, 9);
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 10, 10, 8);
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 6, 6, 7);
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 7, 7, 6);
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 3, 1, 3);
                    imm = imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 5, 2);
                    inst->imm = (i64) imm;
                    break;
                }

                default: fatalf("unimplemented. Instruction = %x", raw_inst);
            }

            break;
        }

        case 0x2: {
            inst->rvc = true;

            switch(c_funct3) {

                case 0x2: { // RVC - C.LWSP
                    inst->type = inst_clwsp;
                    inst->rd = RD(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 4, 2, 4);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 7, 6, 2);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 5, 12);
                    if (inst->rd == 0) INVALID_INST();
                    break;
                }

                case 0x3: { // RVC - C.LDSP
                    inst->type = inst_cldsp;
                    inst->rd = RD(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 4, 3, 5);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 8, 6, 2);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 5, 12);
                    if (inst->rd == 0) INVALID_INST();
                    break;
                }

                case 0x4: {
                    u8 rs1 = C_RS1(raw_inst);
                    u8 rs2 = C_RS2(raw_inst);
                    u8 inst_12 = (raw_inst >> 12) & 0x1;
                    if (inst_12 == 0 && rs1 != 0 && rs2 == 0) { // RVC - C.JR
                        inst->type = inst_cjr;
                        inst->rs1 = rs1;
                    }

                    if (inst_12 == 1 && rs1 != 0 && rs2 == 0) { // RVC - C.JALR
                        inst->type = inst_cjalr;
                        inst->rs1 = rs1;
                    }

                    break;
                }


                case 0x6: { // RVC - C.SWSP
                    inst->type = inst_cswsp;
                    inst->rs2 = C_RS2(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 2, 9);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 7, 6, 7);
                    break;
                }

                case 0x7: { // RVC - C.SDSP
                    inst->type = inst_csdsp;
                    inst->rs2 = C_RS2(raw_inst);
                    inst->imm = 0;
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 5, 3, 10);
                    inst->imm = inst->imm | EXTRACT_IMM_UNSIGNED(raw_inst, 8, 6, 7);
                    break;
                }

                default: fatalf("unimplemented. Instruction = %x", raw_inst);

            }

            break;
        }

        case 0x3: {

            inst->rvc = false;  // not compressed instructions

            switch(opcode) {

                case 0x0: {
                    *inst = inst_i_type(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_lb; return;     // RV32I - LB
                        case 0x1: inst->type = inst_lh; return;     // RV32I - LH
                        case 0x2: inst->type = inst_lw; return;     // RV32I - LW
                        case 0x3: inst->type = inst_ld; return;     // RV64I - LD
                        case 0x4: inst->type = inst_lbu; return;    // RV32I - LBU
                        case 0x5: inst->type = inst_lhu; return;    // RV32I - LHU
                        case 0x6: inst->type = inst_lwu; return;    // RV64I - LWU
                        default: INVALID_INST();
                    }
                }

                case 0x3: {
                    inst->type = inst_fence; return;        // RV32I - FENCE
                }

                case 0x4: {
                    *inst = inst_i_type(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_addi;  return; // RV32I - ADDI
                        case 0x1: inst->type = inst_slli;  return; // RV64I - SLLI
                        case 0x2: inst->type = inst_slti;  return; // RV32I - SLTI
                        case 0x3: inst->type = inst_sltiu; return; // RV32I - SLTIU
                        case 0x4: inst->type = inst_xori;  return; // RV32I - XORI
                        case 0x6: inst->type = inst_ori;   return; // RV32I - ORI
                        case 0x7: inst->type = inst_andi;  return; // RV32I - ANDI
                        case 0x5: {
                            if      (funct7 == 0x0)  {inst->type = inst_srli; return;} // RV64I - SRLI
                            else if (funct7 == 0x20) {inst->type = inst_srai; return;} // RV64I - SRAI
                            else                     {fatal("Not a valid itype instruction");}
                        }
                        default: INVALID_INST();
                    }
                }

                case 0x5: { // RV32I - AUIPC
                    *inst = inst_u_type(raw_inst);
                    inst->type = inst_auipc;
                    return;
                }

                case 0x6: {
                    *inst = inst_s_type(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_addiw; return;  // RV64I - ADDIW
                        case 0x1: inst->type = inst_slliw; return;  // RV64I - SLLIW
                        case 0x5: {
                            if      (funct7 == 0x0)  {inst->type = inst_srliw; return;} // RV64I - SRLIW
                            else if (funct7 == 0x20) {inst->type = inst_sraiw; return;} // RV64I - SRAIW
                            else                     {fatal("Not a valid itype instruction");}
                        }
                        default: INVALID_INST();
                    }
                }

                case 0x8: {
                    *inst = inst_s_type(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_sb; return;     // RV32I - SB
                        case 0x1: inst->type = inst_sh; return;     // RV32I - SH
                        case 0x2: inst->type = inst_sw; return;     // RV32I - SW
                        case 0x3: inst->type = inst_sd; return;     // RV64I - SD
                        default: INVALID_INST();
                    }
                }

                case 0xC: {
                    *inst = inst_r_type(raw_inst);
                    switch (funct3) {
                        case 0x0: {
                            if      (funct7 == 0x0)  {inst->type = inst_add; return;} // RV32I - ADD
                            else if (funct7 == 0x20) {inst->type = inst_sub; return;} // RV32I - SUB
                            else if (funct7 == 0x1)  { // RV32M
                                switch (funct3) {
                                    case 0x0: {inst->type = inst_mul;    return;}    // RV32M - MUL
                                    case 0x1: {inst->type = inst_mulh;   return;}    // RV32M - MULH
                                    case 0x2: {inst->type = inst_mulhsu; return;}    // RV32M - MULHSU
                                    case 0x3: {inst->type = inst_mulhu;  return;}    // RV32M - MULHU
                                    case 0x4: {inst->type = inst_div;    return;}    // RV32M - DIV
                                    case 0x5: {inst->type = inst_divu;   return;}    // RV32M - DIVU
                                    case 0x6: {inst->type = inst_rem;    return;}    // RV32M - REM
                                    case 0x7: {inst->type = inst_remu;   return;}    // RV32M - REMU
                                    default: INVALID_INST();
                                }
                            }
                            else                     {fatal("Not a valid arithmetic instruction");}
                        }
                        case 0x1: inst->type = inst_sll;  return; // RV32I - SLL
                        case 0x2: inst->type = inst_slt;  return; // RV32I - SLT
                        case 0x3: inst->type = inst_sltu; return; // RV32I - SLTU
                        case 0x4: inst->type = inst_xor;  return; // RV32I - XOR
                        case 0x5: {
                            if      (funct7 == 0x0)  {inst->type = inst_srl; return;} // RV32I - SRL
                            else if (funct7 == 0x20) {inst->type = inst_sra; return;} // RV32I - SRA
                            else                     {fatal("Not a valid itype instruction");}
                        }
                        case 0x6: inst->type = inst_or;  return; // RV32I - OR
                        case 0x7: inst->type = inst_and; return; // RV32I - AND
                        default: INVALID_INST();
                    }
                }

                case 0xD: { // RV32I - LUI
                    *inst = inst_u_type(raw_inst);
                    inst->type = inst_lui;
                    return;
                }

                case 0xE: {
                    switch (funct3) {
                        case 0x0: {
                            if      (funct7 == 0x0)  {inst->type = inst_addw; return;} // RV64I - ADDW
                            else if (funct7 == 0x20) {inst->type = inst_subw; return;} // RV64I - SUBW
                            else if (funct7 == 0x1)  { // RV64M
                                switch (funct3) {
                                    case 0x0: {inst->type = inst_mulw;    return;}    // RV32M - MULW
                                    case 0x4: {inst->type = inst_divw;    return;}    // RV32M - DIVW
                                    case 0x5: {inst->type = inst_divuw;   return;}    // RV32M - DIVUW
                                    case 0x6: {inst->type = inst_remw;    return;}    // RV32M - REMW
                                    case 0x7: {inst->type = inst_remuw;   return;}    // RV32M - REMUW
                                    default: INVALID_INST();
                                }
                            }
                            else                     {INVALID_INST();}
                        }
                        case 0x1: inst->type = inst_sllw;  return; // RV64I - SLLW
                        case 0x5: {
                            if      (funct7 == 0x0)  {inst->type = inst_srlw; return;} // RV64I - SRLW
                            else if (funct7 == 0x20) {inst->type = inst_sraw; return;} // RV64I - SRAW
                            else                     {INVALID_INST();}
                        }

                        default: INVALID_INST();
                    }
                }

                case 0x18: { // RV32I - Branch
                    *inst = inst_b_type(raw_inst);
                    switch (funct3) {
                        case 0x0: inst->type = inst_beq; return;    // RV32I - BEQ
                        case 0x1: inst->type = inst_bne; return;    // RV32I - BNE
                        case 0x4: inst->type = inst_blt; return;    // RV32I - BLE
                        case 0x5: inst->type = inst_bge; return;    // RV32I - BGE
                        case 0x6: inst->type = inst_bltu; return;   // RV32I - BLTU
                        case 0x7: inst->type = inst_bgeu; return;   // RV32I - BGEU
                        default: INVALID_INST();
                    }
                }

                case 0x19: { // RV32I - JALR
                    *inst = inst_i_type(raw_inst);
                    inst->type = inst_jalr;
                    return;
                }

                case 0x1B: { // RV32I - JAL
                    *inst = inst_j_type(raw_inst);
                    inst->type = inst_jal;
                    return;
                }

                case 0x1C: {

                    switch (funct3) {
                        case 0x0: {
                            if      (inst_31_20 == 0x0) {inst->type = inst_ecall; return;}  // RV32I - ECALL
                            else if (inst_31_20 == 0x1) {inst->type = inst_ebreak; return;} // RV32I - EBREAK
                            else                        {fatalf("unimplemented. Instruction = %x", raw_inst);}
                        }
                        case 0x1: {inst->type = inst_csrrw;  *inst = inst_csr_type(raw_inst); return;}  // Zicsr - CSRRW
                        case 0x2: {inst->type = inst_csrrs;  *inst = inst_csr_type(raw_inst); return;}  // Zicsr - CSRRS
                        case 0x3: {inst->type = inst_csrrc;  *inst = inst_csr_type(raw_inst); return;}  // Zicsr - CSRRC
                        case 0x5: {inst->type = inst_csrrwi; *inst = inst_csr_type(raw_inst); return;}  // Zicsr - CSRRWI
                        case 0x6: {inst->type = inst_csrrsi; *inst = inst_csr_type(raw_inst); return;}  // Zicsr - CSRRSI
                        case 0x7: {inst->type = inst_csrrci; *inst = inst_csr_type(raw_inst); return;}  // Zicsr - CSRRCI
                        default: INVALID_INST();
                    }

                }

                default: fatalf("unimplemented. Instruction = %x", raw_inst);
            }

        }
        default: unreachable();
    }
}

#undef IMM_MASK
#undef EXTRACT_IMM_UNSIGNED
#undef EXTRACT_IMM_SIGNED
