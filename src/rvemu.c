#include "rvemu.h"

int main (int argc, char **argv) {
    // check if arguments are valid.
    if (argc < 2) {
        fatal("No input files");
    }

    machine_t machine = {0};
    machine_load_program(&machine, argv[1]);

    while(true) {
        enum exit_reason_t reason = machine_step(&machine);
        assert(reason == ecall);
        machine.state.exit_reason = none; // reset the exit_reason
    }

    printf("ELF info (in host memory space)\n");
    printf("entry: %llx\n", TO_HOST(machine.mmu.entry));
    printf("host_alloc: %lx\n", machine.mmu.host_alloc);
    return 0;
}