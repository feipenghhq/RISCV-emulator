#include "rvemu.h"


#define QUADRANT(data) (((data) >> 0) & 0x3)

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
    switch(quadrant) {
        case 0x0: fatal("unimplemented");
        case 0x1: fatal("unimplemented");
        case 0x2: fatal("unimplemented");
        case 0x3: fatal("unimplemented");
        default: unreachable();
    }
}