//==============================================================================
//
// Copyright (C) 2010, 2011, 2012
// Marco Antonio Zanata Alves
//
// GPPD - Parallel and Distributed Processing Group
// Universidade Federal do Rio Grande do Sul
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//==============================================================================

#ifndef __CONVERSIONS_CPP__
#define __CONVERSIONS_CPP__



#include <vector>
#include <algorithm>
#include <string>

#include "./tracer_log_procedures.hpp"
#include "../../../../defines.hpp"
#include "../../../../utils/enumerations.hpp"
#include "../../../../main_memory/memory_request_client.hpp"
#include "./opcodes.hpp"

#include "pin.H"
#include "instlib.H"
#include "./xed-interface.h"

#ifdef TRACE_GENERATOR_DEBUG
    #define TRACE_GENERATOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define TRACE_GENERATOR_DEBUG_PRINTF(...)
#endif

unsigned int hex_to_dec(char x) {
    if (x >= '0' && x <= '9')
        return x - '0';
    return (toupper(x) - 'A') + 10U;
}

unsigned int ascii_to_hex(const char *src,
                          xed_uint32_t *dst,
                          unsigned int max_bytes) {
    const unsigned int len = strlen(src);
    memset(dst, 0, max_bytes);

    for (unsigned int p = 0, i = 0; i < len / 2; ++i, p += 2)
        dst[i] = (xed_uint32_t) (hex_to_dec(src[p]) * 16 + hex_to_dec(src[p+1]));
    return len/2;
}

std::string to_string(xed_uint_t x) {
    std::string res;
    while (x) {
        res.push_back((x % 10) + '0');
        x /= 10;
    }
    std::reverse(res.begin(), res.end());
    return res;
}

std::string gen_icode(xed_decoded_inst_t *xedd) {
    const xed_inst_t* xi = xed_decoded_inst_inst(xedd);
    unsigned int noperands = xed_inst_noperands(xi);

    std::string result = "";
    for (unsigned int i = 0; i < noperands; ++i) {
        const xed_operand_t* op = xed_inst_operand(xi, i);
        xed_operand_enum_t op_name = xed_operand_name(op);

        std::string ops = "";

        switch (op_name) {
            case XED_OPERAND_AGEN:
                break;

            // Memory
            case XED_OPERAND_MEM0:
            case XED_OPERAND_MEM1: {
                ops += "M";
                break;
            }

            // Pointer and rel
            case XED_OPERAND_PTR:
            case XED_OPERAND_RELBR: {
                ops += "Rel";
                break;
            }

            // Immediates
            case XED_OPERAND_IMM0: {
                ops += "I";
                break;
            }

            // Registers
            case XED_OPERAND_REG0:
            case XED_OPERAND_REG1:
            case XED_OPERAND_REG2:
            case XED_OPERAND_REG3:
            case XED_OPERAND_REG4:
            case XED_OPERAND_REG5:
            case XED_OPERAND_REG6:
            case XED_OPERAND_REG7:
            case XED_OPERAND_REG8:
            case XED_OPERAND_BASE0:
            case XED_OPERAND_BASE1: {
                ops += "R";
                break;
            }
            default:
                assert(0);
        }

        auto vis = xed_operand_operand_visibility(op);
        if (vis == XED_OPVIS_EXPLICIT && ops.size() > 0) {
            result += "+";
            xed_uint_t bits = xed_decoded_inst_operand_length_bits(xedd, i);
            ops += to_string(bits);
            result += ops;
        }
    }

    return std::string(xed_iform_enum_t2str(xed_decoded_inst_get_iform_enum(xedd))) + result;
}

//==============================================================================
// Translate the x86 Instructions to Simulator Instruction
opcode_package_t x86_to_static(const INS& ins) {
    TRACE_GENERATOR_DEBUG_PRINTF("x86_to_static()\n");
    ERROR_ASSERT_PRINTF(INS_MaxNumRRegs(ins) <= MAX_REGISTERS
                     && INS_MaxNumWRegs(ins) <= MAX_REGISTERS,
                        "More registers than MAX_REGISTERS\n");
    uint32_t i;
    opcode_package_t NewInstruction;

    // strcpy(NewInstruction.opcode_assembly, INS_Mnemonic(ins).c_str());
    NewInstruction.opcode_address = INS_Address(ins);
    NewInstruction.opcode_size = INS_Size(ins);

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        NewInstruction.read_regs[i] = POSITION_FAIL;
    }
    for (i = 0; i < INS_MaxNumRRegs(ins); i++) {
        NewInstruction.read_regs[i] = INS_RegR(ins, i);
    }


    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        NewInstruction.write_regs[i] = POSITION_FAIL;
    }
    for (i = 0; i < INS_MaxNumWRegs(ins); i++) {
        NewInstruction.write_regs[i] = INS_RegW(ins, i);
    }

    NewInstruction.base_reg = INS_MemoryBaseReg(ins);
    NewInstruction.index_reg = INS_MemoryIndexReg(ins);

    NewInstruction.num_reads = 0;
    NewInstruction.num_writes = 0;

    if (INS_hasKnownMemorySize(ins)) {
        if (INS_IsMemoryRead(ins)) NewInstruction.num_reads++;
        if (INS_HasMemoryRead2(ins)) NewInstruction.num_reads++;
        if (INS_IsMemoryWrite(ins)) NewInstruction.num_writes++;
    } else {
        int32_t numAccesses, accessSize, indexSize;
        

        if(INS_IsVgather(ins)) {
            GetNumberAndSizeOfMemAccesses(ins, &numAccesses, &accessSize, &indexSize);
            NewInstruction.num_reads = numAccesses;
            if (numAccesses > 1) {
                std::cerr << "Encontrei com " << numAccesses << "!!" << std::endl;
            } else {
                std::cerr << "Just one!!" << std::endl;
            }

        } else if (INS_IsVscatter(ins)) {
            GetNumberAndSizeOfMemAccesses(ins, &numAccesses, &accessSize, &indexSize);
            NewInstruction.num_writes = numAccesses;

        } else {
            std::cerr << "vgather_vscatter_to_static: Unknown instruction type!" << std::endl;
            exit(1);
        }
    }

    //NewInstruction.is_read = INS_IsMemoryRead(ins);
    //NewInstruction.is_read2 = INS_HasMemoryRead2(ins);
    //NewInstruction.is_write = INS_IsMemoryWrite(ins);

    NewInstruction.is_predicated = INS_IsPredicated(ins);
    NewInstruction.is_prefetch = INS_IsPrefetch(ins);

    if (NewInstruction.num_reads > 0) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_LOAD;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM LOAD (%u reads):%s\n", NewInstruction.num_reads, INS_Disassemble(ins).c_str());
    }
    if (NewInstruction.num_writes > 0) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_STORE;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM STORE (%u writes):%s\n", NewInstruction.num_writes, INS_Disassemble(ins).c_str());
    }
    /*if (NewInstruction.is_read) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_LOAD;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM LOAD:%s\n",
                                    INS_Disassemble(ins).c_str());
    }

    if (NewInstruction.is_read2) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_LOAD;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM LOAD:%s\n",
                                    INS_Disassemble(ins).c_str());
    }
    if (NewInstruction.is_write) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_STORE;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM STORE:%s\n",
                                     INS_Disassemble(ins).c_str());
    }
    */



    if (INS_IsBranchOrCall(ins) || INS_IsSyscall(ins)) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_BRANCH;
        TRACE_GENERATOR_DEBUG_PRINTF("\t BRANCH:%s\n",
                                    INS_Disassemble(ins).c_str());

        // Call
        if (INS_IsCall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Call    ");
            NewInstruction.branch_type = BRANCH_CALL;
        } else if (INS_IsRet(ins)) {  // Return
            TRACE_GENERATOR_DEBUG_PRINTF("Return  ");
            NewInstruction.branch_type = BRANCH_RETURN;
        } else if (INS_IsSyscall(ins)) {  // Syscall
            TRACE_GENERATOR_DEBUG_PRINTF("Syscall ");
            NewInstruction.branch_type = BRANCH_SYSCALL;
        } else {  // Branch/Call
            // Conditional?
            if (INS_HasFallThrough(ins)) {
                TRACE_GENERATOR_DEBUG_PRINTF("Branch  ");
                NewInstruction.branch_type = BRANCH_COND;
            } else {
                TRACE_GENERATOR_DEBUG_PRINTF("Jump    ");
                NewInstruction.branch_type = BRANCH_UNCOND;
            }
        }


        NewInstruction.is_indirect = INS_IsIndirectBranchOrCall(ins);

        if (INS_IsDirectBranchOrCall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Direct \n");
        } else if (INS_IsIndirectBranchOrCall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Indirect \n");
        } else if (INS_IsSyscall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Syscall \n");
        } else {
            TRACE_GENERATOR_DEBUG_PRINTF("**ERROR**\n");
        }
    }

    xed_decoded_inst_t* xedd = INS_XedDec(ins);
    const xed_inst_t* xi = xed_decoded_inst_inst(xedd);
    xed_iclass_enum_t iclass = xed_inst_iclass(xi);

    std::string icode = gen_icode(xedd);
    strncpy(NewInstruction.opcode_assembly,
        icode.c_str(),
        MAX_ASSEMBLY_SIZE);

    switch (iclass) {
        //==================================================================
        // Source:
        //          http://ref.x86asm.net/geek.html#x63
        //          http://en.wikipedia.org/wiki/X86_instruction_listings
        //          home/pin/extras/xed2-ia32/misc/idata.txt
        //==================================================================
        //==================================================================
        // BRANCH

        // ~ case XED_ICLASS_CALL_FAR:       // Call Procedure
        // ~ case XED_ICLASS_CALL_NEAR:      // Call Procedure
        // ~ case XED_ICLASS_RET_FAR:        // Return from procedure
        // ~ case XED_ICLASS_RET_NEAR:       // Return from procedure
        case XED_ICLASS_PAUSE:          // PAUSE (Cacheability and spin lock)
        case XED_ICLASS_NOP:
        case XED_ICLASS_FNOP:
            NewInstruction.opcode_operation = INSTRUCTION_OPERATION_NOP;
            TRACE_GENERATOR_DEBUG_PRINTF("\t NOP:%s\n",
                                        INS_Disassemble(ins).c_str());
            break;

        //==================================================================
        // INT ALU or MEMORY
        case XED_ICLASS_POP:            // Pop from stack
        case XED_ICLASS_POPF:           // Pop Stack into rFLAGS Register
        case XED_ICLASS_POPFD:          // Pop Stack into eFLAGS Register
        case XED_ICLASS_POPFQ:          // Pop Stack into rFLAGS Register

        case XED_ICLASS_POPCNT:         // Bit Population Count

        case XED_ICLASS_PUSH:           // Push to stack
        case XED_ICLASS_PUSHF:          // Push rFLAGS Register onto the Stack
        case XED_ICLASS_PUSHFD:         // Push eFLAGS Register onto the Stack
        case XED_ICLASS_PUSHFQ:         // Push rFLAGS Register onto the Stack

        case XED_ICLASS_MOV:            // Move
        case XED_ICLASS_MOVSX:          // Move with sign-extend
        case XED_ICLASS_MOVZX:          // Move with zero-extend
        case XED_ICLASS_MOVSXD:         // Move with Sign-Extension
        case XED_ICLASS_MOVSD:          // Move Data from String to String
        case XED_ICLASS_MOVSQ:          // Move Data from String to String
                                /*
                                if (NewInstruction.is_read || NewInstruction.is_read2 || NewInstruction.is_write) {
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t MEM:%s\n", INS_Disassemble(ins).c_str());
                                }
                                else {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_ALU;
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t INT ALU:%s\n", INS_Disassemble(ins).c_str());
                                }*/
                                if (NewInstruction.num_reads > 0 || NewInstruction.num_writes > 0) {
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t MEM:%s\n", INS_Disassemble(ins).c_str());
                                }
                                else {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_ALU;
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t INT ALU:%s\n", INS_Disassemble(ins).c_str());
                                }  
                                break;

        //==================================================================
        // INT ALU
        case XED_ICLASS_CPUID:          // CPU IDentification

        case XED_ICLASS_XGETBV:         // Get Value Extended Control Register
        case XED_ICLASS_XSETBV:         // Set Extended Control Register

        case XED_ICLASS_CMOVS:          // Conditional Move
        case XED_ICLASS_CMOVNS:         // Conditional Move
        case XED_ICLASS_CMOVNP:         // Conditional Move
        case XED_ICLASS_CMOVB:          // Conditional Move
        case XED_ICLASS_CMOVBE:         // Conditional Move
        case XED_ICLASS_CMOVNB:         // Conditional Move
        case XED_ICLASS_CMOVNBE:        // Conditional Move
        case XED_ICLASS_CMOVL:          // Conditional Move
        case XED_ICLASS_CMOVLE:         // Conditional Move
        case XED_ICLASS_CMOVNL:         // Conditional Move
        case XED_ICLASS_CMOVNLE:        // Conditional Move
        case XED_ICLASS_CMOVNZ:         // Conditional Move
        case XED_ICLASS_CMOVZ:          // Conditional Move
        case XED_ICLASS_LEA:            // Load Effective Address
        case XED_ICLASS_SHL:            // Shift left (unsigned shift left)
        case XED_ICLASS_SHR:            // Shift right (unsigned shift right)
        case XED_ICLASS_SAR:            // Shift Arithmetically right
                                        // (signed shift right)
        case XED_ICLASS_SAHF:           // Store AH to Flags
        case XED_ICLASS_LAHF:           // Load AH from Flags
        case XED_ICLASS_SHLD:           // Double Precision Shift Left
        case XED_ICLASS_SHRD:           // Double Precision Shift Right
        case XED_ICLASS_ROL:            // Rotate left
        case XED_ICLASS_ROR:            // Rotate right
        case XED_ICLASS_BSF:            // Bit scan forward
        case XED_ICLASS_BSR:            // Bit scan reverse
        case XED_ICLASS_SCASB:          // Scan string
        case XED_ICLASS_SCASD:          // Scan string
        case XED_ICLASS_SCASQ:          // Scan string
        case XED_ICLASS_SCASW:          // Scan string
        case XED_ICLASS_BSWAP:          // Byte Swap
        case XED_ICLASS_XOR:            // Exclusive OR
        case XED_ICLASS_OR:             // Logical OR
        case XED_ICLASS_AND:            // Logical AND
        case XED_ICLASS_NOT:            // Negate the operand, logical NOT
        case XED_ICLASS_NEG:            // Two's complement negation

        case XED_ICLASS_SETB:           // Set flag to one on condition
        case XED_ICLASS_SETZ:           // Set flag to one on condition
        case XED_ICLASS_SETNZ:          // Set flag to one on condition
        case XED_ICLASS_SETP:           // Set flag to one on condition
        case XED_ICLASS_SETS:           // Set flag to one on condition
        case XED_ICLASS_SETL:           // Set flag to one on condition

        case XED_ICLASS_SETBE:          // Set byte on condition
        case XED_ICLASS_SETNB:          // Set byte on condition
        case XED_ICLASS_SETNBE:         // Set byte on condition
        case XED_ICLASS_SETNL:          // Set byte to one on condition
        case XED_ICLASS_SETNP:          // Set byte to one on condition
        case XED_ICLASS_SETNS:          // Set byte to one on condition
        case XED_ICLASS_SETNLE:         // Set byte to one on condition
        case XED_ICLASS_SETLE:          // Set byte to one on condition

        case XED_ICLASS_TEST:           // Logical compare (AND)
        case XED_ICLASS_BT:             // Bit test
        case XED_ICLASS_SUB:            // Subtract
        case XED_ICLASS_SBB:            // Subtraction with borrow
        case XED_ICLASS_ADD:            // Add
        case XED_ICLASS_XADD:           // Exchange and Add
        case XED_ICLASS_ADC:            // Add with carry
        case XED_ICLASS_INC:            // Increment by 1
        case XED_ICLASS_DEC:            // Decrement by 1
        case XED_ICLASS_CMP:            // Compare operands
        case XED_ICLASS_CMPSB:          // Compare String Operands
        case XED_ICLASS_CMPXCHG:        // Compare and Exchange

        case XED_ICLASS_CDQE:           // Convert / extend the sign
        case XED_ICLASS_CDQ:            // Convert / extend the sign
        case XED_ICLASS_CQO:            // Convert / extend the sign
        case XED_ICLASS_CBW:            // Convert / extend the sign
        case XED_ICLASS_CWD:            // Convert / extend the sign
        case XED_ICLASS_CWDE:           // Convert / extend the sign

        case XED_ICLASS_STC:            // Set the carry flags
        case XED_ICLASS_STD:            // Set the direction flags
        case XED_ICLASS_STI:            // Set the interrupt flags

        case XED_ICLASS_CLC:            // Clear the carry flags
        case XED_ICLASS_CLD:            // Clear the direction flags
        case XED_ICLASS_CLI:            // Clear the interrupt flags

        case XED_ICLASS_XCHG:           // Exchange data
        case XED_ICLASS_RDTSC:          // Read Time-Stamp Counter
            NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_ALU;
            TRACE_GENERATOR_DEBUG_PRINTF("\t INT ALU:%s\n",
                                        INS_Disassemble(ins).c_str());
            break;

        //==================================================================
        // FP ALU or MEMORY
        case XED_ICLASS_MOVD:           // Move Doubleword
        case XED_ICLASS_MOVQ:           // Move Doubleword/Quadword
        case XED_ICLASS_MOVSS:          // Move Scalar Single-FP Values
        case XED_ICLASS_MOVAPS:         // Move Aligned Packed Single-FP Values
        case XED_ICLASS_MOVAPD:         // Move Aligned Packed Double-FP Values
        case XED_ICLASS_MOVDQA:         // Move Aligned Double Quadword
        case XED_ICLASS_MOVDQU:         // Move Aligned Double Quadword
        case XED_ICLASS_MOVSD_XMM:      // Move Data from String to String

        case XED_ICLASS_MOVHPS:         // Move High Packed Single-FP Values
        case XED_ICLASS_MOVHPD:         // Move High Packed Double-FP Value
        case XED_ICLASS_MOVHLPS:        // Move Packed Single-FP Values
                                        // High to Low
        case XED_ICLASS_MOVLPS:         // Move Low Packed Single-FP Values
        case XED_ICLASS_MOVLPD:         // Move Low Packed Double-FP Value
        case XED_ICLASS_MOVLHPS:        // Move Packed Single-FP Values
                                        // Low to High
        case XED_ICLASS_MOVDDUP:        // Move One Double-FP and Duplicate
        case XED_ICLASS_MOVSLDUP:       // Move Packed Single-FP Low
                                        // and Duplicate
        case XED_ICLASS_MOVSHDUP:       // Move Packed Single-FP High
                                        // and Duplicate
        case XED_ICLASS_MOVMSKPD:       // Extract Packed Double-FP Sign Mask

        case XED_ICLASS_LDDQU:          // Load Unaligned Integer 128 Bits
        case XED_ICLASS_LDMXCSR:        // Load MXCSR Register

        case XED_ICLASS_FST:            // Store Floating Point Value
        case XED_ICLASS_FLD:            // Load Floating Point Value
        case XED_ICLASS_FLDZ:           // Load Floating Point Constants
        case XED_ICLASS_FLDCW:          // Load x87 FPU Control Word
        case XED_ICLASS_FNSTCW:         // Store x87 FPU Control Word

        case XED_ICLASS_STMXCSR:        // Store MXCSR Register State
        case XED_ICLASS_STOSB:          // Store String
        case XED_ICLASS_STOSD:          // Store String
        case XED_ICLASS_STOSQ:          // Store String
        case XED_ICLASS_STOSW:          // Store String

                                /*if (NewInstruction.is_read || NewInstruction.is_read2 || NewInstruction.is_write) {
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t MEM:%s\n", INS_Disassemble(ins).c_str());
                                }
                                else {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_ALU;
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t FP ALU:%s\n", INS_Disassemble(ins).c_str());
                                }*/
                                if (NewInstruction.num_reads > 0 || NewInstruction.num_writes > 0) {
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t MEM:%s\n", INS_Disassemble(ins).c_str());
                                }
                                else {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_ALU;
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t FP ALU:%s\n", INS_Disassemble(ins).c_str());
                                }
                                break;

        //==================================================================
        // FP ALU x87
        case XED_ICLASS_FNCLEX:         // Clear Exceptions
        case XED_ICLASS_FADD:           // x87 Add
        case XED_ICLASS_FADDP:          // x87 Add and pop

        case XED_ICLASS_FSUB:           // x87 Subtract
        case XED_ICLASS_FSUBR:          // x87 Subtract
        case XED_ICLASS_FSUBRP:         // x87 Subtract
        case XED_ICLASS_FSUBP:          // x87 Subtract and pop

        case XED_ICLASS_FRNDINT:        // FP Round to Integer
        case XED_ICLASS_FABS:           // FP Absolute Value
        case XED_ICLASS_FWAIT:          // Check pending unmasked FP exceptions
        case XED_ICLASS_MWAIT:          // Monitor Wait
        case XED_ICLASS_FSTP:           // Store Floating Point Value and Pop
        case XED_ICLASS_FNSTSW:         // Store FP Status Word
        case XED_ICLASS_FXAM:           // Examine Class of Value
        case XED_ICLASS_FXCH:           // Exchange Register Contents
        case XED_ICLASS_FCHS:           // FP Change Sign

        case XED_ICLASS_FUCOM:          // FP Unordered Compare
        case XED_ICLASS_FUCOMI:         // FP Unordered Compare
        case XED_ICLASS_FUCOMIP:        // FP Unordered Compare
        case XED_ICLASS_FUCOMP:         // FP Unordered Compare
        case XED_ICLASS_FUCOMPP:        // FP Unordered Compare

        // FP ALU MMX
        case XED_ICLASS_PXOR:           // Logical Exclusive OR
        case XED_ICLASS_POR:            // Logical OR
        case XED_ICLASS_PAND:           // Logical AND
        case XED_ICLASS_PANDN:          // Logical Not AND

        case XED_ICLASS_PSUBD:          // Subtract Packed Integers
        case XED_ICLASS_PSUBB:          // Subtract Packed Integers
        case XED_ICLASS_PSUBQ:          // Subtract Packed Integers
        case XED_ICLASS_PSUBUSB:          // Subtract Packed Integers

        case XED_ICLASS_PADDD:          // Add Packed Integers
        case XED_ICLASS_PADDB:          // Add Packed Integers
        case XED_ICLASS_PADDQ:          // Add Packed Integers


        case XED_ICLASS_PCMPEQB:        // Compare Packed Data for Equal
        // FP ALU SSE
        case XED_ICLASS_PTEST:          // Logical Compare
        case XED_ICLASS_SFENCE:         // Store Fence
        case XED_ICLASS_LFENCE:         // Load Fence
        case XED_ICLASS_MFENCE:         // Complete Memory Fence
        case XED_ICLASS_SHUFPD:         // Shuffle Packed Double-FP values
        case XED_ICLASS_SHUFPS:         // Shuffle Packed Single-FP Values


        case XED_ICLASS_PSHUFB:         // Shuffle Packed Bytes
        case XED_ICLASS_PSHUFD:         // Shuffle Packed Doublewords
        case XED_ICLASS_PSHUFLW:        // Shuffle Packed Low Words
        case XED_ICLASS_PSHUFHW:        // Shuffle Packed High Words

        case XED_ICLASS_PSLLD:          // Shift Double Quadword Left Logical
        case XED_ICLASS_PSLLQ:          // Shift Double Quadword Left Logical
        case XED_ICLASS_PSLLDQ:         // Shift Double Quadword Left Logical

        case XED_ICLASS_PSRAD:         // Shift Double Quadword Right Logical
        case XED_ICLASS_PSRLD:         // Shift Double Quadword Right Logical
        case XED_ICLASS_PSRLQ:         // Shift Double Quadword Right Logical
        case XED_ICLASS_PSRLDQ:         // Shift Double Quadword Right Logical

        case XED_ICLASS_PALIGNR:        // Packed Align Right

        case XED_ICLASS_PUNPCKHBW:     // pack High Data
        case XED_ICLASS_PUNPCKHQDQ:    // pack High Data
        case XED_ICLASS_PUNPCKHDQ:     // pack High Data
        case XED_ICLASS_PUNPCKHWD:     // pack High Data

        case XED_ICLASS_PUNPCKLBW:      // pack Low Data
        case XED_ICLASS_PUNPCKLQDQ:     // pack Low Data
        case XED_ICLASS_PUNPCKLDQ:      // pack Low Data
        case XED_ICLASS_PUNPCKLWD:      // pack Low Data

        case XED_ICLASS_UNPCKLPD:       // Unpack and Interleave
                                        // Low Packed Double-FP
        case XED_ICLASS_UNPCKLPS:       // Unpack and Interleave
                                        // Low Packed Double-FP

        case XED_ICLASS_UNPCKHPD:       // Unpack and Interleave
                                        // High Packed Double-FP
        case XED_ICLASS_UNPCKHPS:       // Unpack and Interleave
                                        // High Packed Double-FP

        case XED_ICLASS_PMOVMSKB:       // Move Integer SSE
        case XED_ICLASS_CVTTSD2SI:      // Convert from Double
        case XED_ICLASS_CVTSI2SD:       // Convert to Double
        case XED_ICLASS_CVTSD2SI:       // Convert to Double
        case XED_ICLASS_CVTSI2SS:       // Convert to Double
        case XED_ICLASS_CVTSS2SD:       // Convert to Double
        case XED_ICLASS_CVTDQ2PD:       // Convert to Double
        case XED_ICLASS_CVTTPD2DQ:      // Convert to Double
        case XED_ICLASS_COMISD:         // Compare Scalar Ordered Double-FP
                                        // and Set EFLAGS
        case XED_ICLASS_UCOMISD:        // Unordered Compare Scalar Single-FP
                                        // and Set EFLAGS
        case XED_ICLASS_CMPSD_XMM:      // Compare Scalar Double-FP Values

        case XED_ICLASS_PCMPEQD:        // Compare Packed Data for Equal
        case XED_ICLASS_PCMPISTRI:      // Packed Compare
                                        // Implicit Length Strings, Return Index

        case XED_ICLASS_PCMPGTB:        // Compare Packed Data for Greater Than
        case XED_ICLASS_PCMPGTD:        // Compare Packed Data for Greater Than
        case XED_ICLASS_PCMPGTW:        // Compare Packed Data for Greater Than

        case XED_ICLASS_COMISS:         // Compare Scalar Single-FP
                                        // and Set EFLAGS
        case XED_ICLASS_UCOMISS:        // Unordered Compare Scalar Single-FP
                                        // and Set EFLAGS

        case XED_ICLASS_CMPSS:          // Compare Scalar Single-FP
        case XED_ICLASS_CMPSD:          // Compare Scalar Double-FP
        case XED_ICLASS_CMPPS:          // Compare Packed Single-FP
        case XED_ICLASS_CMPPD:          // Compare Packed Double-FP

        case XED_ICLASS_ANDPS:          // Bitwise Logical AND Packed Single-FP
        case XED_ICLASS_ANDPD:          // Bitwise Logical AND Packed Double-FP

        case XED_ICLASS_ANDNPS:         // Bitwise Logical NAND Packed Single-FP
        case XED_ICLASS_ANDNPD:         // Bitwise Logical NAND Packed Double-FP

        case XED_ICLASS_XORPS:          // Bitwise Logical XOR Packed Single-FP
        case XED_ICLASS_XORPD:          // Bitwise Logical XOR Packed Double-FP

        case XED_ICLASS_ORPS:           // Bitwise Logical OR Packed Single-FP
        case XED_ICLASS_ORPD:           // Bitwise Logical OR Packed Double-FP

        case XED_ICLASS_PMAXUB:         // Return Maximum Scalar Single-FP
        case XED_ICLASS_MAXSS:          // Return Maximum Scalar Single-FP
        case XED_ICLASS_MAXSD:          // Return Maximum Scalar Double-FP
        case XED_ICLASS_MAXPS:          // Return Maximum Packed Single-FP
        case XED_ICLASS_MAXPD:          // Return Maximum Packed Double-FP

        case XED_ICLASS_PMINUB:         // Return Minimum Packed Single-FP
        case XED_ICLASS_MINPS:          // Return Minimum Packed Single-FP
        case XED_ICLASS_MINPD:          // Return Minimum Packed Double-FP
        case XED_ICLASS_MINSS:          // Return Minimum Scalar Single-FP
        case XED_ICLASS_MINSD:          // Return Minimum Scalar Double-FP

        case XED_ICLASS_ADDPD:          // Add Packed Double-Precision FP
        case XED_ICLASS_ADDPS:          // Add Packed Single-Precision FP
        case XED_ICLASS_ADDSS:          // Add Scalar Single-Precision FP
        case XED_ICLASS_ADDSD:          // Add Low Double-Precision FP

        case XED_ICLASS_ROUNDPD:        // Round Packed Double-Precision FP
        case XED_ICLASS_ROUNDPS:        // Round Packed Single-Precision FP
        case XED_ICLASS_ROUNDSS:        // Round Scalar Single-Precision FP
        case XED_ICLASS_ROUNDSD:        // Round Low Double-Precision FP

        case XED_ICLASS_SUBPD:          // Sub Packed Double-Precision FP
        case XED_ICLASS_SUBPS:          // Sub Packed Single-Precision FP
        case XED_ICLASS_SUBSS:          // Sub Scalar Single-Precision FP
        case XED_ICLASS_SUBSD:          // Sub Low Double-Precision FP
        case XED_ICLASS_VPAND:
        case XED_ICLASS_VPANDD:
        case XED_ICLASS_VPANDN:
        case XED_ICLASS_VPANDND:
        case XED_ICLASS_VPANDNQ:
        case XED_ICLASS_VPANDQ:
        case XED_ICLASS_VPADDB:
        case XED_ICLASS_VPADDD:
        case XED_ICLASS_VPADDQ:
        case XED_ICLASS_VPADDSB:
        case XED_ICLASS_VPADDSW:
        case XED_ICLASS_VPADDUSB:
        case XED_ICLASS_VPADDUSW:
        case XED_ICLASS_VPADDW:
        case XED_ICLASS_VPSLLD:
        case XED_ICLASS_VPSLLDQ:
        case XED_ICLASS_VPSLLQ:
        case XED_ICLASS_VPSLLVD:
        case XED_ICLASS_VPSLLVQ:
        case XED_ICLASS_VPSLLVW:
        case XED_ICLASS_VPSLLW:
        case XED_ICLASS_VPTEST:
        case XED_ICLASS_VPTESTMB:
        case XED_ICLASS_VPTESTMD:
        case XED_ICLASS_VPTESTMQ:
        case XED_ICLASS_VPTESTMW:
        case XED_ICLASS_VPTESTNMB:
        case XED_ICLASS_VPTESTNMD:
        case XED_ICLASS_VPTESTNMQ:
        case XED_ICLASS_VPTESTNMW:
        case XED_ICLASS_KORW:
            NewInstruction.opcode_operation = INSTRUCTION_OPERATION_FP_ALU;
            TRACE_GENERATOR_DEBUG_PRINTF("\t FP ALU:%s\n",
                                        INS_Disassemble(ins).c_str());
            break;

        //==================================================================
        // INT MUL
        case XED_ICLASS_MUL:            // Unsigned multiply
        case XED_ICLASS_IMUL:           // Signed multiply
            NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_MUL;
            TRACE_GENERATOR_DEBUG_PRINTF("\t INT MUL:%s\n",
                                        INS_Disassemble(ins).c_str());
            break;

        //==================================================================
        // FP MUL x87
        case XED_ICLASS_FMUL:           // Multiply
        case XED_ICLASS_FIMUL:          // Integer multiply
        case XED_ICLASS_FMULP:          // Multiply and pop
        // FP MUL SSE
        case XED_ICLASS_MULSS:          // Multiply Scalar Single-FP Values
        case XED_ICLASS_MULSD:          // Multiply Scalar Double-FP Values
        case XED_ICLASS_MULPD:          // Multiply Packed Double-FP Values
        case XED_ICLASS_MULPS:          // Multiply Packed Single-FP Values

        case XED_ICLASS_PMULUDQ:        // Multiply Packed Unsigned DW Integers

        case XED_ICLASS_VPMULLD:
        case XED_ICLASS_VPMULLQ:
        case XED_ICLASS_VPMULLW:
            NewInstruction.opcode_operation = INSTRUCTION_OPERATION_FP_MUL;
            TRACE_GENERATOR_DEBUG_PRINTF("\t FP MUL:%s\n",
                                        INS_Disassemble(ins).c_str());
            break;

        //==================================================================
        // INT DIV
        case XED_ICLASS_DIV:            // Divide
        case XED_ICLASS_IDIV:           // Integer Divide
            NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_DIV;
            TRACE_GENERATOR_DEBUG_PRINTF("\t INT DIV:%s\n",
                                        INS_Disassemble(ins).c_str());
            break;

        //==================================================================
        // LONG TIME EXECUTION INSTRUCTIONS
        case XED_ICLASS_INT:            // Software interrupt
        case XED_ICLASS_SYSENTER:       // System Call 32 bits
        case XED_ICLASS_SYSCALL:        // System Call 64 bits

        // FP DIV x87
        case XED_ICLASS_FDIV:           // Divide
        case XED_ICLASS_FDIVP:          // Divide and pop
        case XED_ICLASS_FDIVR:          // Divide reversed
        case XED_ICLASS_FDIVRP:         // Divide reversed and pop
        case XED_ICLASS_FIDIV:          // Integer divide
        case XED_ICLASS_FIDIVR:         // Integer divide reversed
        // FP DIV SSE
        case XED_ICLASS_DIVSS:          // Divide Scalar Single-FP Values
        case XED_ICLASS_DIVSD:          // Divide Scalar Double-FP Values
        case XED_ICLASS_DIVPD:          // Divide Packed Double-FP Values
        case XED_ICLASS_DIVPS:          // Divide Packed Single-FP Values
        // FP TRIGONOMETRY
        case XED_ICLASS_FSIN:           // Compute the sine
        case XED_ICLASS_FCOS:           // Compute the sine
        case XED_ICLASS_FSINCOS:        // Compute the cosine
        case XED_ICLASS_FPTAN:          // Compute the tangent
        case XED_ICLASS_FPATAN:         // Compute the arc-tangent
        // FP SCALE POWER
        case XED_ICLASS_FSCALE:         // Scale FP to power of two
        // FP Partial Remainder
        case XED_ICLASS_FPREM:          // FP Partial Remainder
        case XED_ICLASS_FPREM1:         // FP Partial Remainder

        // FP SQRT x87
        case XED_ICLASS_FSQRT:          // Square root
        // FP SQRT 3D NOW
        case XED_ICLASS_PFRSQRT:         // Square root
        // FP SQRT SSE
        case XED_ICLASS_RSQRTPS:        // Recipr. of Square Roots of
                                        // Packed Single-FP
        case XED_ICLASS_SQRTPD:         // Square Roots of Packed Double-FP
        case XED_ICLASS_SQRTPS:         // Square Roots of Packed Single-FP
        case XED_ICLASS_SQRTSD:         // Square Root of Scalar Double-FP
        case XED_ICLASS_SQRTSS:         // Square Root of Scalar Single-FP
            NewInstruction.opcode_operation = INSTRUCTION_OPERATION_FP_DIV;
            TRACE_GENERATOR_DEBUG_PRINTF("\t FP DIV:%s\n",
                                        INS_Disassemble(ins).c_str());
            break;

        //==================================================================
        default:
            if (NewInstruction.opcode_operation == INSTRUCTION_OPERATION_NOP) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_OTHER;
                TRACE_GENERATOR_DEBUG_PRINTF("\t OTHER:%s\n",
                                            INS_Disassemble(ins).c_str());
            }
            break;
    }

    return NewInstruction;
}

#endif
