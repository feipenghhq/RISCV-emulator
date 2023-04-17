#include "rvemu.h"

int main (int argc, char **argv) {
    // check if arguments are valid.
    if (argc < 2) {
        fatal("No input files");
    }

    machine_t machine = {0};
    machine_load_program(&machine, argv[1]);

    printf("entry: %llx\n", TO_HOST(machine.mmu.entry));
    printf("host_alloc: %lx\n", machine.mmu.host_alloc);
    return 0;
}