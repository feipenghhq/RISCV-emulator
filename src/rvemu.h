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
    u64 gp_regs[32];      // RISCV 32 general purpose registers
    u64 pc;                         // Program counter
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
 * @brief RISCV instructions
 *
 */
enum inst_type_t {
    inst_addi,
    num_insts,
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