#include "rvemu.h"


/**
 * @brief execute multiple instructions till we hit a ecall
 *
 * @param m pointer to machine
 * @return enum exit_reason_t
 */
enum exit_reason_t machine_step(machine_t *m) {
    while(true) {
        exec_block_interp(&m->state);

        // continue execution if it is indirect branch or direct branch
        if (m->state.exit_reason == indirect_branch || m->state.exit_reason == direct_branch) {
            m->state.exit_reason = none;    // reset the exit_reason
            continue;
        }

        // continue execution if it is mret
        if (m->state.exit_reason == mret) {
            m->state.exit_reason = none;    // reset the exit_reason
            continue;
        }

        // break on ecall.
        assert(m->state.exit_reason == ecall);
        break;
    }

    return ecall;
}

/**
 * @brief Load the program into memory
 * @param m: pointer to a machine
 * @param prog: program to be loaded
 */
void machine_load_program(machine_t *m, char *prog) {
    // open the program and check if it is opened correctly
    int fd = open(prog, O_RDONLY);
    if (fd == -1) {
        fatal(strerror(errno));
    }

    // load ELF information to MMU
    mmu_load_elf(&(m->mmu), fd);
    close(fd);

    // assign the program entry to current PC
    m->state.pc = m->mmu.entry;
}

void machine_setup(machine_t *m, int argc, char *argv[]) {
    size_t stack_size = STACK_SIZE;
    u64 stack = mmu_alloc(&m->mmu, stack_size);
    m->state.gp_regs[sp] = stack + stack_size;

    // stack memory map:
    // [                           | argc argv envp auxv]
    //                             ^ stack bottom (sp)
    m->state.gp_regs[sp] -= 8; // auxv
    m->state.gp_regs[sp] -= 8; // envp
    m->state.gp_regs[sp] -= 8; // argv end

    // processing the arguments from guest program
    u64 args = argc - 1; // first arg is rvemu itself
    for (int i = args; i > 0; i--) {
        // store the argument into following the stack
        // [ program ][ stack ][ argc ]|[ heap ]
        size_t len = strlen(argv[i]);
        // argv[i] is a string, we need to allocate the addition '\0' character
        u64 addr = mmu_alloc(&m->mmu, len+1);
        mmu_write(addr, (u8 *) argv[i], len);
        // store the address of the argument into stack (argc is char** type)
        m->state.gp_regs[sp] -= 8; // argv[i]
        mmu_write(m->state.gp_regs[sp], (u8 *) &addr, sizeof(u64));
    }

    m->state.gp_regs[sp] -= 8; // argc
    mmu_write(m->state.gp_regs[sp], (u8 *) &args, sizeof(u64));

}

