#ifndef __RVEMU_H__
#define __RVEMU_H__

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "types.h"

#define fatalf(fmt, ...) (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), exit(1))
#define fatal(msg) fatalf("%s", msg)

/**
 * @brief Memory Management Unit
 *
 */
typedef struct {
    u64 entry;  // starting address of the executable section of the program
} mmu_t;

/**
 * @brief CPU state
 *
 */
typedef struct {
    u64 gp_regs[32];    // RISCV 32 general purpose registers
    u64 pc;             // Program counter
} state_t;

/**
 * @brief store machine status
 *
 */
typedef struct {
    state_t state;
    mmu_t mmu;
} machine_t;


void machine_load_program(machine_t *, char *);


#endif