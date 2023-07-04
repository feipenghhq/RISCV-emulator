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
#include "csr.h"

//////////////////////////////////
// Macros
//////////////////////////////////

// Error logging
#define fatalf(fmt, ...) (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), exit(1))
#define fatal(msg) fatalf("%s", msg)
#define unreachable() (fatal("unreachable"), __builtin_unreachable())
#define warning(msg) printf("%s\n", msg)

// Bit manipulation
#define ROUNDDOWN(x, y)     ((x) & -(y))
#define ROUNDUP(x, y)       (((x) + (y)-1) & -(y))
#define MIN(x, y)           (((x) < (y)) ? (x) : (y))
#define MAX(x, y)           (((x) > (y)) ? (x) : (y))

// Memory mapping between host and guest
#define GUEST_MEMORY_OFFSET 0x088800000000ULL
#define TO_GUEST(addr)  ((addr) - GUEST_MEMORY_OFFSET)
#define TO_HOST(addr)   ((addr) + GUEST_MEMORY_OFFSET)

#define STACK_SIZE          32 * 1024 * 1024
//#define DEBUG

//////////////////////////////////
// Structs
//////////////////////////////////

/**
 * @brief Memory Management Unit
 *
 */
typedef struct {
    u64 entry;      // starting address of the executable section of the guest program (pc entry)
                    // in guest memory space
    u64 host_alloc; // stores the upper boundary of the malloced memory space in host view
    u64 base;       // base is the guest view of host_alloc (host_alloc mapped to guest memory space)
    u64 alloc;      // alloc stores the upper boundary of the malloced memory space
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
    mret,
};

/**
 * @brief CPU state
 *
 */
typedef struct {
    enum exit_reason_t exit_reason;
    u64 gp_regs[num_gp_regs];   // RISCV 32 general purpose registers
    u64 csr[4096];              // 4096 CSR registers

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
#include "inst_type_t.h"
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
u64 mmu_alloc(mmu_t *, i64);
void machine_setup(machine_t *, int, char**);

//////////////////////////////////
// Inline Function
//////////////////////////////////

inline void mmu_write(u64 addr, u8 *data, size_t len) {
    memcpy((void *) TO_HOST(addr), (void *) data, len);
}

inline u64 machine_get_gp_reg(machine_t *m, i32 reg) {
    assert(reg >= 0 && reg <= num_gp_regs);
    return m->state.gp_regs[reg];
}

inline void machine_set_gp_reg(machine_t *m, i32 reg, u64 data) {
    assert(reg >= 0 && reg <= num_gp_regs);
    m->state.gp_regs[reg] = data;
}


#endif