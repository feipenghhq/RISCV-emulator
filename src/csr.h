////////////////////////////////////
// CSR register
////////////////////////////////////

// Machine Trap Setup
 #define mstatus_id     0x300
 #define misa_id        0x301
 #define medeleg_id     0x302
 #define mideleg_id     0x303
 #define mie_id         0x304
 #define mtvec_id       0x305
 #define mcounteren_id  0x306
 #define mstatush_id    0x310

// Machine Trap Handling
#define mscratch_id     0x340
#define mepc_id         0x341
#define mcause_id       0x342
#define mtval_id        0x343
#define mip_id          0x344
#define mtinst_id       0x34A
#define mtval2_id       0x34B

////////////////////////////////////
// CSR register field access
////////////////////////////////////

// Helper macro
#define csr_gen_mask64(data, pos)       ((u64) (data) << (pos))
#define csr_set_field(reg, data, mask)  ((reg) & ~(mask) | ((data) & (mask)))
#define csr_get_field(reg, pos, mask)   (((reg) & (mask)) >> (pos))
#define csr_mask_name(csr, field)       (csr ## _ ## field ## _mask)
#define csr_pos_name(csr, field)        (csr ## _ ## field ## _pos)

// set a field for a csr register
#define csr_set(csr, field, reg, data)  csr_set_field(reg, data, csr_mask_name(csr, field))

// get a field for a csr register
#define csr_get(csr, field, reg)        csr_get_field(reg, csr_pos_name(csr, field), csr_mask_name(csr, field))

//////////////////////////////////////
// CSR Field definations
////////////////////////////////////

// - start of mstatus - //
#define mstatus_mpv_pos             39
#define mstatus_mpv_mask            csr_gen_mask64(0x1, mstatus_mpv_pos)

#define mstatus_mpp_pos             11
#define mstatus_mpp_mask            csr_gen_mask64(0x3, mstatus_mpp_pos)

#define mstatus_mpie_pos             7
#define mstatus_mpie_mask           csr_gen_mask64(0x1, mstatus_mpie_pos)

#define mstatus_mie_pos             3
#define mstatus_mie_mask            csr_gen_mask64(0x1, mstatus_mie_pos)

// - end of mstatus - //