#ifndef __RVEMU_H__
#define __RVEMU_H__

//////////////////////////////////
// Includes
//////////////////////////////////

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdbool.h>

#include "types.h"
#include "elfdef.h"
#include "reg.h"

//////////////////////////////////
// Macros
//////////////////////////////////

// Error logging
#define fatalf(fmt, ...) (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), exit(1))
#define fatal(msg) fatalf("%s", msg)
#define unreachable() (fatal("unreachable"), __builtin_unreachable())

// Bit manipulation
#define ROUNDDOWN(x, y)     ((x) & -(y))
#define ROUNDUP(x, y)       (((x) + (y)-1) & -(y))
#define MIN(x, y)           (((x) < (y)) ? (x) : (y))
#define MAX(x, y)           (((x) > (y)) ? (x) : (y))

// Memory mapping between host and guest
#define GUEST_MEMORY_OFFSET 0x088800000000ULL
#define TO_GUEST(addr)  ((addr) - GUEST_MEMORY_OFFSET)
#define TO_HOST(addr)   ((addr) + GUEST_MEMORY_OFFSET)

//////////////////////////////////
// Structs
//////////////////////////////////

/**
 * @brief Memory Management Unit
 *
 */
typedef struct {
    u64 entry;  // starting address of the executable section of the program
    u64 host_alloc;
    u64 alloc;
    u64 base;
} mmu_t;

/**
 * @brief exit reason
 *
 */
enum exit_reason_t {
    none,
    direct_branch,
    indirect_branch,
    ecall,
};

/**
 * @brief CPU state
 *
 */
typedef struct {
    enum exit_reason_t exit_reason;
    u64 gp_regs[num_gp_regs];   // RISCV 32 general purpose registers
    u32 csr[4096];              // 4096 CSR registers

    u64 pc;                     // Program counter
    u64 reenter_pc;             // Re-enter Program counter

    bool raise_exception;       // exception happens
    u32  exception_code;        // exception types
} state_t;

/**
 * @brief store machine status
 *
 */
typedef struct {
    state_t state;
    mmu_t mmu;
} machine_t;

/**
 * @brief RISC-V instructions
 *
 */
enum inst_type_t {
    // RV32I/RV64I Base Instruction Set
    inst_lui, inst_auipc,
    inst_jal, inst_jalr, inst_beq, inst_bne, inst_blt, inst_bge, inst_bltu, inst_bgeu,
    inst_lb, inst_lh, inst_lw, inst_lbu, inst_lhu,
    inst_sb, inst_sh, inst_sw,
    inst_addi, inst_slti, inst_sltiu, inst_xori, inst_ori, inst_andi,
    inst_slli, inst_srli, inst_srai,
    inst_add, inst_sub, inst_sll, inst_slt, inst_sltu, inst_xor, inst_srl, inst_sra, inst_or, inst_and,
    inst_fence, inst_ecall, inst_ebreak,
    inst_lwu, inst_ld, inst_sd,
    inst_addiw, inst_slliw, inst_srliw, inst_sraiw,
    inst_addw, inst_subw, inst_sllw, inst_srlw, inst_sraw,
    // RV32M/RV64M
    inst_mul, inst_mulh, inst_mulhsu, inst_mulhu,
    inst_div, inst_divu, inst_rem, inst_remu,
    inst_mulw, inst_divw, inst_divuw, inst_remw, inst_remuw,
    // "Zicsr"
    inst_csrrw, inst_csrrs, inst_csrrc, inst_csrrwi, inst_csrrsi, inst_csrrci,
    // RVC instructions
    inst_clwsp, inst_cldsp, inst_cswsp, inst_csdsp,
    inst_clw, inst_cld, inst_csw, inst_csd,
    inst_cj, inst_cjr, inst_cjalr,
    inst_cbeqz, inst_cbnez,
    inst_cli, inst_clui,
    inst_caddi, inst_caddiw, inst_caddi16sp, inst_caddi4spn, inst_cslli, inst_csrli, inst_csrai, inst_candi,
    inst_cmv, inst_cadd, inst_cand, inst_cor, inst_cxor, inst_csub, inst_caddw, inst_csubw,
    inst_cnop,
    // Numbered instructions
    num_insts,
};

/**
 * @brief RISC-V Exceptions
 *
 */
enum exception_type_t {
    instruction_address_misaligned = 0, // FIXME: assign to the actual exception number
};

/**
 * @brief RISCV Instruction format
 *
 */
typedef struct {
    i8 rd;
    i8 rs1;
    i8 rs2;
    i32 imm;
    i16 csr;
    enum inst_type_t type;
    bool rvc;
    bool cont;
} inst_t;


//////////////////////////////////
// Function prototype
//////////////////////////////////

void mmu_load_elf(mmu_t *, int);
void machine_load_program(machine_t *, char *);
void inst_decode(inst_t *inst, u32 data);
void exec_block_interp(state_t *state);
enum exit_reason_t machine_step(machine_t *m);

#endif