// See LICENSE for license details.

#ifndef ROCC_SOFTWARE_SRC_XCUSTOM_H_
#define ROCC_SOFTWARE_SRC_XCUSTOM_H_

#define STR1(x) #x
#ifndef STR
#define STR(x) STR1(x)
#endif
#define EXTRACT(a, size, offset) (((~(~0 << size) << offset) & a) >> offset)

// rd = rs2[offset + size - 1 : offset]
// rs1 is clobbered
// rs2 is left intact
#define EXTRACT_RAW(rd, rs1, rs2, size, offset) \
  not x ## rs1, x0;                             \
  slli x ## rs1, x ## rs1, size;                \
  not x ## rs1, x ## rs1;                       \
  slli x ## rs1, x ## rs1, offset;              \
  and x ## rd, x ## rs1, x ## rs2;              \
  srai x ## rd, x ## rd, offset;


// Custom opcode values:
// These values are defined at the following file:
// src/main/scala/tile/LazyRoCC.scal
//
// You can have your own opcodes base on ISA user manual as well
#define XCUSTOM_OPCODE(x) XCUSTOM_OPCODE_ ## x
#define XCUSTOM_OPCODE_0 0b0001011
#define XCUSTOM_OPCODE_1 0b0101011
#define XCUSTOM_OPCODE_2 0b1011011
#define XCUSTOM_OPCODE_3 0b1111011

//RISCV custom instruciton format
//   ------------------------------------------------------------------------------
//  |   funct(7)  |  rs2(5) | rs1(5) | xd(1) | xs1(1) | xs2(1) | rd(5) | OpCode(7) |
//   ------------------------------------------------------------------------------
//
//   1) Funct  : For different kinds of accelerator instructions; Value is designer's choice
//   2) rs2    : Source register ID
//   3) rs1    : Source register ID
//   4) xd     : Set if destination register exists
//   5) xs1    : Set if source register exists
//   6) xs2    : Set if source register exists
//   7) rd     : Destination register ID
//   8) OpCode : Custom instruction opcode may be used in case of multiple accelerator
//
//
//  In this example rs1 and rs2 are always exist -> 0x3
//  If rd is 0 then xd is set to zero otherwise it's set to 1
//
#define XCUSTOM(x, rd, rs1, rs2, funct)         \
  XCUSTOM_OPCODE(x)                   |         \
  (rd                   << (7))       |         \
  (0x3                  << (7+5))     |         \
  ((rd != 0) & 1        << (7+5+2))   |         \
  (rs1                  << (7+5+3))   |         \
  (rs2                  << (7+5+3+5)) |         \
  (EXTRACT(funct, 7, 0) << (7+5+3+5+5))

#define ROCC_INSTRUCTION_RAW_R_R_R(x, rd, rs1, rs2, funct)      \
  .word XCUSTOM(x, ## rd, ## rs1, ## rs2, funct)

// Standard macro that passes rd, rs1, and rs2 via registers
#define ROCC_INSTRUCTION(x, rd, rs1, rs2, funct)                \
  ROCC_INSTRUCTION_R_R_R(x, rd, rs1, rs2, funct, 10, 11, 12)

// rd, rs1, and rs2 are data
// rd_n, rs_1, and rs2_n are the register numbers to use
//
//
// Please note that "register" keyword here help us to tell compiler
// impose move commands if it's necessary.
#define ROCC_INSTRUCTION_R_R_R(x, rd, rs1, rs2, funct, rd_n, rs1_n, rs2_n) \
  {                                                                     \
    register uint64_t rd_  asm ("x" # rd_n);                            \
    register uint64_t rs1_ asm ("x" # rs1_n) = (uint64_t) rs1;          \
    register uint64_t rs2_ asm ("x" # rs2_n) = (uint64_t) rs2;          \
    asm volatile (                                                      \
        ".word " STR(XCUSTOM(x, rd_n, rs1_n, rs2_n, funct)) "\n\t"      \
        : "=r" (rd_)                                                    \
        : [_rs1] "r" (rs1_), [_rs2] "r" (rs2_));                        \
    rd = rd_;                                                           \
  }

#define ROCC_INSTRUCTION_0_R_R(x, rs1, rs2, funct, rs1_n, rs2_n)  \
  {                                                               \
    register uint64_t rs1_ asm ("x" # rs1_n) = (uint64_t) rs1;    \
    register uint64_t rs2_ asm ("x" # rs2_n) = (uint64_t) rs2;    \
    asm volatile (                                                \
        ".word " STR(XCUSTOM(x, 0, rs1_n, rs2_n, funct)) "\n\t"   \
        :: [_rs1] "r" (rs1_), [_rs2] "r" (rs2_));                 \
  }

// [TODO] fix these to align with the above approach
// Macro to pass rs2_ as an immediate
/*
#define ROCC_INSTRUCTION_R_R_I(XCUSTOM_, rd_, rs1_, rs2_, funct_) \
  asm volatile (XCUSTOM_" %[rd], %[rs1], %[rs2], %[funct]"        \
                : [rd] "=r" (rd_)                                 \
                : [rs1] "r" (rs1_), [rs2] "i" (rs2_), [funct] "i" (funct_))

// Macro to pass rs1_ and rs2_ as immediates
#define ROCC_INSTRUCTION_R_I_I(XCUSTOM_, rd_, rs1_, rs2_, funct_) \
  asm volatile (XCUSTOM_" %[rd], %[rs1], %[rs2], %[funct]"        \
                : [rd] "=r" (rd_)                                 \
                : [rs1] "i" (rs1_), [rs2] "i" (rs2_), [funct] "i" (funct_))
*/

#endif  // ROCC_SOFTWARE_SRC_XCUSTOM_H_
