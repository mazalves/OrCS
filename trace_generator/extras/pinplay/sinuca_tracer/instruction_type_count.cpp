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
/// Define macros to use uint_64 for printf (PRIu64) and scanf (SCNu64)
#define __STDC_FORMAT_MACROS

#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <fstream>

#include "pin.H"
#include "instlib.H"
#include "xed-interface.h"



// ~ #define DEBUG
#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf(__VA_ARGS__);
#else
    #define DEBUG_PRINTF(...)
#endif

//==============================================================================
// Global Variables
//==============================================================================

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
#define PADSIZE 56  // 64 byte line size: 64-8

enum instruction_operation_t {
    /// NOP
    INSTRUCTION_OPERATION_ANY,
    /// NOP
    INSTRUCTION_OPERATION_NOP,
    /// INTEGERS
    INSTRUCTION_OPERATION_INT_ALU,
    INSTRUCTION_OPERATION_INT_MUL,
    INSTRUCTION_OPERATION_INT_DIV,
    /// FLOAT POINT
    INSTRUCTION_OPERATION_FP_ALU,
    INSTRUCTION_OPERATION_FP_MUL,
    INSTRUCTION_OPERATION_FP_DIV,
    /// BRANCHES
    INSTRUCTION_OPERATION_BRANCH,
    /// MEMORY OPERATIONS
    INSTRUCTION_OPERATION_MEM_LOAD,
    INSTRUCTION_OPERATION_MEM_STORE,
    /// NOT INDENTIFIED
    INSTRUCTION_OPERATION_OTHER,
    /// SYNCRONIZATION
    INSTRUCTION_OPERATION_NUMBER
};


// a running count of the instructions
class thread_data_t {
  public:
    UINT64 counter[INSTRUCTION_OPERATION_NUMBER];
    UINT8 pad[PADSIZE];

    thread_data_t(){
        for (uint32_t i = 0; i < INSTRUCTION_OPERATION_NUMBER; i++) {
            counter[i] = 0;
        }
    };
};
thread_data_t* thread_data;

PIN_LOCK lock;                  /// Lock for methods shared among multiple threads
uint32_t max_threads = 0;       /// Max number of threads (if Intel Compiler Add +1)
std::ofstream DynamicTraceFile;

//==============================================================================
// Commandline Switches
//==============================================================================
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "output_trace.out", "specify the trace base file name.");
KNOB<uint32_t> KnobNumberThreads(KNOB_MODE_WRITEONCE, "pintool",
    "t", "1", "specify the total number of threads.");

//==============================================================================
// Print Help Message
//==============================================================================
uint32_t Usage() {
    cerr <<
        "This tool produces a compressed (dynamic) instruction trace.\n"
        "The trace is still in textual form but repeated sequences\n"
        "of the same code are abbreviated with a number which dramatically\n"
        "reduces the output size and the overhead of the tool.\n"
        "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << "Example of Usage: >$ ../../../pin -t obj-intel64/trace.so -o ~/trace_mg/mg_S -c (compress) 1 -t (threads) 9 -i (intel) 1 -- ~/NPB3.3-OMP/bin/mg.S\n";
    cerr << endl;

    return -1;
};

VOID docount(uint32_t inst_type,  THREADID threadid)
{
    thread_data[threadid].counter[inst_type] += 1;
}

//==============================================================================
// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID trace_instruction(TRACE trace, VOID *v) {
    DEBUG_PRINTF("trace_instruction()\n");

    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_ANY ,IARG_THREAD_ID, IARG_END);

            if (INS_IsMemoryRead(ins)) {
                DEBUG_PRINTF("\t MEM LOAD:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(&ins).c_str());
                BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_MEM_LOAD ,IARG_THREAD_ID, IARG_END);
            }

            if (INS_HasMemoryRead2(ins)) {
                DEBUG_PRINTF("\t MEM LOAD:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_MEM_LOAD ,IARG_THREAD_ID, IARG_END);
            }

            if (INS_IsMemoryWrite(ins)) {
                DEBUG_PRINTF("\t MEM STORE:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_MEM_STORE ,IARG_THREAD_ID, IARG_END);
            }

            if (INS_IsBranch(ins)) {
                DEBUG_PRINTF("\t BRANCH:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_BRANCH ,IARG_THREAD_ID, IARG_END);
            }

            xed_decoded_inst_t* xedd = INS_XedDec(ins);
            const xed_inst_t* xi = xed_decoded_inst_inst(xedd);
            xed_iclass_enum_t iclass = xed_inst_iclass(xi);

            switch (iclass) {
                //==================================================================
                // Source:
                //          http://ref.x86asm.net/geek.html#x63
                //          http://en.wikipedia.org/wiki/X86_instruction_listings
                //          home/pin/extras/xed2-ia32/misc/idata.txt
                //==================================================================
                case XED_ICLASS_NOP:
                case XED_ICLASS_FNOP:
                                        DEBUG_PRINTF("\t NOP:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                        BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_NOP ,IARG_THREAD_ID, IARG_END);
                                        break;

                //==================================================================
                // INT ALU or MEMORY
                case XED_ICLASS_CALL_FAR:       // Call Procedure
                case XED_ICLASS_CALL_NEAR:      // Call Procedure
                case XED_ICLASS_RET_FAR:        // Return from procedure
                case XED_ICLASS_RET_NEAR:       // Return from procedure

                case XED_ICLASS_POP:            // Pop from stack
                case XED_ICLASS_POPF:           // Pop Stack into rFLAGS Register
                case XED_ICLASS_POPFD:          // Pop Stack into eFLAGS Register
                case XED_ICLASS_POPFQ:          // Pop Stack into rFLAGS Register

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
                                        if (!INS_IsMemoryRead(ins) && !INS_HasMemoryRead2(ins) && !INS_IsMemoryWrite(ins)) {
                                            DEBUG_PRINTF("\t INT ALU:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_INT_ALU ,IARG_THREAD_ID, IARG_END);
                                        }
                                        break;

                //==================================================================
                // INT ALU
                case XED_ICLASS_SYSENTER:       // System Call 32 bits
                case XED_ICLASS_SYSCALL:        // System Call 64 bits
                case XED_ICLASS_CPUID:          // CPU IDentification

                case XED_ICLASS_CMOVS:          // Conditional Move
                case XED_ICLASS_CMOVNS:         // Conditional Move
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
                case XED_ICLASS_SAR:            // Shift Arithmetically right (signed shift right)
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
                case XED_ICLASS_XOR:            // Exclusive OR
                case XED_ICLASS_OR:             // Logical OR
                case XED_ICLASS_AND:            // Logical AND
                case XED_ICLASS_NOT:            // Negate the operand, logical NOT
                case XED_ICLASS_NEG:            // Two's complement negation
                case XED_ICLASS_SETB:           // Set byte on condition
                case XED_ICLASS_SETNB:          // Set byte on condition
                case XED_ICLASS_SETNBE:         // Set byte on condition
                case XED_ICLASS_SETZ:           // Set byte to one on condition
                case XED_ICLASS_SETNZ:          // Set byte to one on condition
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
                case XED_ICLASS_CDQE:           // Convert
                case XED_ICLASS_CDQ:            // Convert
                case XED_ICLASS_CQO:            // Convert
                case XED_ICLASS_XCHG:           // Exchange data
                case XED_ICLASS_RDTSC:          // Read Time-Stamp Counter
                                            DEBUG_PRINTF("\t INT ALU:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_INT_ALU ,IARG_THREAD_ID, IARG_END);
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
                case XED_ICLASS_MOVHLPS:        // Move Packed Single-FP Values High to Low
                case XED_ICLASS_MOVLPS:         // Move Low Packed Single-FP Values
                case XED_ICLASS_MOVLPD:         // Move Low Packed Double-FP Value
                case XED_ICLASS_MOVLHPS:        // Move Packed Single-FP Values Low to High
                case XED_ICLASS_MOVDDUP:        // Move One Double-FP and Duplicate
                case XED_ICLASS_MOVSLDUP:       // Move Packed Single-FP Low and Duplicate
                case XED_ICLASS_MOVSHDUP:       // Move Packed Single-FP High and Duplicate

                case XED_ICLASS_LDDQU:          // Load Unaligned Integer 128 Bits
                case XED_ICLASS_LDMXCSR:        // Load MXCSR Register

                case XED_ICLASS_FST:            // Store Floating Point Value
                case XED_ICLASS_FLD:            // Load Floating Point Value
                case XED_ICLASS_FLDCW:          // Load x87 FPU Control Word
                case XED_ICLASS_FNSTCW:         // Store x87 FPU Control Word

                case XED_ICLASS_STMXCSR:        // Store MXCSR Register State
                case XED_ICLASS_STOSB:          // Store String
                case XED_ICLASS_STOSD:          // Store String
                case XED_ICLASS_STOSQ:          // Store String
                case XED_ICLASS_STOSW:          // Store String
                                        if (!INS_IsMemoryRead(ins) && !INS_HasMemoryRead2(ins) && !INS_IsMemoryWrite(ins)) {
                                            DEBUG_PRINTF("\t FP ALU:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_FP_ALU ,IARG_THREAD_ID, IARG_END);
                                        }
                                        break;

                //==================================================================
                // FP ALU x87
                case XED_ICLASS_FNCLEX:         // Clear Exceptions
                case XED_ICLASS_FADD:           // x87 Add
                case XED_ICLASS_FADDP:          // x87 Add and pop
                case XED_ICLASS_FSUB:           // x87 Subtract
                case XED_ICLASS_FWAIT:          // Check pending unmasked floating-point exceptions
                case XED_ICLASS_MWAIT:          // Monitor Wait
                case XED_ICLASS_FSTP:           // Store Floating Point Value and Pop
                case XED_ICLASS_FXCH:           // Exchange Register Contents
                // FP ALU MMX
                case XED_ICLASS_PXOR:           // Logical Exclusive OR
                case XED_ICLASS_POR:            // Logical OR
                case XED_ICLASS_PAND:           // Logical AND
                case XED_ICLASS_PANDN:          // Logical Not AND
                case XED_ICLASS_PSUBD:          // Subtract Packed Integers
                case XED_ICLASS_PSUBB:          // Subtract Packed Integers
                case XED_ICLASS_PADDD:          // Add Packed Integers
                case XED_ICLASS_PUNPCKLBW:      // Unpack Low Data
                case XED_ICLASS_PCMPEQB:        // Compare Packed Data for Equal
                // FP ALU SSE
                case XED_ICLASS_PAUSE:          // PAUSE (for Cacheability)
                case XED_ICLASS_LFENCE:         // Load Fence
                case XED_ICLASS_PSHUFD:         // Shuffle Packed Doublewords
                case XED_ICLASS_SHUFPS:         // Shuffle Packed Single-FP Values
                case XED_ICLASS_PSHUFLW:        // Shuffle Packed Low Words
                case XED_ICLASS_PSHUFHW:        // Shuffle Packed High Words
                case XED_ICLASS_PSLLDQ:         // Shift Double Quadword Left Logical
                case XED_ICLASS_PSRLDQ:         // Shift Double Quadword Right Logical
                case XED_ICLASS_PALIGNR:        // Packed Align Right
                case XED_ICLASS_PUNPCKLQDQ:     // Unpack Low Data
                case XED_ICLASS_PUNPCKHQDQ:     // Unpack High Data
                case XED_ICLASS_PMOVMSKB:       // Move Integer SSE
                case XED_ICLASS_UNPCKLPD:       // Unpack and Interleave Low Packed Double-FP Values
                case XED_ICLASS_UNPCKHPD:       // Unpack and Interleave High Packed Double-FP Values
                case XED_ICLASS_CVTTSD2SI:      // Convert from Double
                case XED_ICLASS_CVTSI2SD:       // Convert to Double
                case XED_ICLASS_CVTSD2SI:       // Convert to Double
                case XED_ICLASS_CVTSI2SS:       // Convert to Double
                case XED_ICLASS_CVTSS2SD:       // Convert to Double
                case XED_ICLASS_CVTDQ2PD:       // Convert to Double
                case XED_ICLASS_CVTTPD2DQ:      // Convert to Double
                case XED_ICLASS_COMISD:         // Compare Scalar Ordered Double-FP Values and Set EFLAGS
                case XED_ICLASS_UCOMISD:        // Unordered Compare Scalar Single-FP Values and Set EFLAGS
                case XED_ICLASS_CMPPS:          // Compare Packed Single-FP Values
                case XED_ICLASS_CMPSS:          // Compare Scalar Single-FP Values
                case XED_ICLASS_CMPPD:          // Compare Packed Double-FP Values
                case XED_ICLASS_CMPSD:          // Compare Scalar Double-FP Values
                case XED_ICLASS_CMPSD_XMM:      // Compare Scalar Double-FP Values
                case XED_ICLASS_PCMPEQD:        // Compare Packed Data for Equal
                case XED_ICLASS_PCMPISTRI:      // Packed Compare Implicit Length Strings, Return Index
                case XED_ICLASS_ANDPS:          // Bitwise Logical AND of Packed Single-FP Values
                case XED_ICLASS_ANDPD:          // Bitwise Logical AND of Packed Double-FP Values
                case XED_ICLASS_MAXPD:          // Return Maximum Packed Double-FP Values
                case XED_ICLASS_MAXSD:          // Return Maximum Scalar Double-FP Value
                case XED_ICLASS_ADDPD:          // Add Packed Double-Precision Floating-Point Values
                case XED_ICLASS_ADDSD:          // Add Low Double-Precision Floating-Point Value
                case XED_ICLASS_SUBPD:          // Sub Packed Double-Precision Floating-Point Values
                case XED_ICLASS_SUBSD:          // Sub Low Double-Precision Floating-Point Value
                case XED_ICLASS_XORPD:          // Bitwise Logical XOR for Double-FP Values
                                            DEBUG_PRINTF("\t FP ALU:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_FP_ALU ,IARG_THREAD_ID, IARG_END);
                                        break;

                //======================================================================
                // INT MUL
                case XED_ICLASS_MUL:            // Unsigned multiply
                case XED_ICLASS_IMUL:           // Signed multiply
                                            DEBUG_PRINTF("\t INT MUL:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_INT_MUL ,IARG_THREAD_ID, IARG_END);
                                        break;

                //======================================================================
                // FP MUL x87
                case XED_ICLASS_FMUL:           // Multiply
                case XED_ICLASS_FIMUL:          // Integer multiply
                case XED_ICLASS_FMULP:          // Multiply and pop
                // FP MUL SSE
                case XED_ICLASS_MULSS:          // Multiply Scalar Double-FP Values
                case XED_ICLASS_MULSD:          // Multiply Scalar Double-FP Values
                case XED_ICLASS_MULPD:          // Multiply Packed Double-FP Values

                case XED_ICLASS_PMULUDQ:        // Multiply Packed Unsigned DW Integers
                                            DEBUG_PRINTF("\t FP MUL:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_FP_ALU ,IARG_THREAD_ID, IARG_END);
                                        break;

                //======================================================================
                // INT DIV
                case XED_ICLASS_DIV:            // Divide
                case XED_ICLASS_IDIV:           // Integer Divide
                                            DEBUG_PRINTF("\t INT DIV:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_INT_DIV ,IARG_THREAD_ID, IARG_END);
                                        break;

                //======================================================================
                // FP DIV x87
                case XED_ICLASS_FDIV:           // Divide
                case XED_ICLASS_FDIVP:          // Divide and pop
                case XED_ICLASS_FDIVR:          // Divide reversed
                case XED_ICLASS_FDIVRP:         // Divide reversed and pop
                case XED_ICLASS_FIDIV:          // Integer divide
                case XED_ICLASS_FIDIVR:         // Integer divide reversed
                // FP DIV SSE
                case XED_ICLASS_DIVSS:          // Divide Scalar Double-FP Values
                case XED_ICLASS_DIVSD:          // Divide Scalar Double-FP Values
                case XED_ICLASS_DIVPD:          // Divide Packed Double-FP Values
                // FP SQRT x87
                case XED_ICLASS_FSQRT:          // Square root
                // FP SQRT 3D NOW
                case XED_ICLASS_PFRSQRT:         // Square root
                // FP SQRT SSE
                case XED_ICLASS_RSQRTPS:        // Compute Recipr. of Square Roots of Packed Single-FP Values
                case XED_ICLASS_SQRTPD:         // Compute Square Roots of Packed Double-FP Values
                case XED_ICLASS_SQRTPS:         // Compute Square Roots of Packed Single-FP Values
                case XED_ICLASS_SQRTSD:         // Compute Square Root of Scalar Double-FP Value
                case XED_ICLASS_SQRTSS:         // Compute Square Root of Scalar Single-FP Value
                                            DEBUG_PRINTF("\t FP DIV:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_FP_DIV ,IARG_THREAD_ID, IARG_END);
                                        break;

                //======================================================================
                default:
                                            DEBUG_PRINTF("\t OTHER:%s - %s\n", INS_Disassemble(ins).c_str(), INS_Mnemonic(ins).c_str());
                                            BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(docount),IARG_UINT32, (uint32_t)INSTRUCTION_OPERATION_OTHER ,IARG_THREAD_ID, IARG_END);
                                        break;
            }
        }
    }
}

//==============================================================================
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v) {
    DEBUG_PRINTF("ThreadStart()\n");
    PIN_GetLock(&lock, threadid);
        ASSERTX(threadid < max_threads);
    PIN_ReleaseLock(&lock);
};

//==============================================================================
VOID Fini(INT32 code, VOID *v) {
    DEBUG_PRINTF("Fini()\n");

    std::stringstream trace_header;
    //char trace_header[5000];
    for (uint32_t i = 0; i < max_threads; i++) {
        trace_header.str(std::string());
        trace_header    << "============================================\n"
                        << " Thread:" << i <<"\n"
                        << " INSTRUCTION_OPERATION_ANY:" << thread_data[i].counter[INSTRUCTION_OPERATION_ANY] <<"\n"
                        
                        << "INSTRUCTION_OPERATION_INT_ALU" << thread_data[i].counter[INSTRUCTION_OPERATION_INT_ALU] << "\n"       
                        << "INSTRUCTION_OPERATION_INT_MUL" << thread_data[i].counter[INSTRUCTION_OPERATION_INT_MUL] << "\n"
                        << "INSTRUCTION_OPERATION_INT_DIV" << thread_data[i].counter[INSTRUCTION_OPERATION_INT_DIV] << "\n"
                        
                        << "INSTRUCTION_OPERATION_FP_ALU" << thread_data[i].counter[INSTRUCTION_OPERATION_FP_ALU] << "\n"
                        << "INSTRUCTION_OPERATION_FP_MUL" << thread_data[i].counter[INSTRUCTION_OPERATION_FP_MUL] << "\n"
                        << "INSTRUCTION_OPERATION_FP_DIV" << thread_data[i].counter[INSTRUCTION_OPERATION_FP_DIV] << "\n"
                        
                        << "INSTRUCTION_OPERATION_BRANCH" << thread_data[i].counter[INSTRUCTION_OPERATION_BRANCH] << "\n"
                        << "INSTRUCTION_OPERATION_NOP" << thread_data[i].counter[INSTRUCTION_OPERATION_NOP] << "\n"
                        << "INSTRUCTION_OPERATION_OTHER" << thread_data[i].counter[INSTRUCTION_OPERATION_OTHER] << "\n";

        /*
        sprintf(trace_header, "============================================\n");
        sprintf(trace_header, "%s Thread:%d\n", trace_header, i);
        sprintf(trace_header, "%s INSTRUCTION_OPERATION_ANY:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_ANY]);

        sprintf(trace_header, "%s INSTRUCTION_OPERATION_INT_ALU:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_ANY]);
        sprintf(trace_header, "%s INSTRUCTION_OPERATION_INT_MUL:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_INT_MUL]);
        sprintf(trace_header, "%s INSTRUCTION_OPERATION_INT_DIV:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_INT_DIV]);

        sprintf(trace_header, "%s INSTRUCTION_OPERATION_FP_ALU:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_ANY]);
        sprintf(trace_header, "%s INSTRUCTION_OPERATION_FP_MUL:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_FP_MUL]);
        sprintf(trace_header, "%s INSTRUCTION_OPERATION_FP_DIV:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_FP_DIV]);

        sprintf(trace_header, "%s INSTRUCTION_OPERATION_BRANCH:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_BRANCH]);
        sprintf(trace_header, "%s INSTRUCTION_OPERATION_NOP:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_NOP]);
        sprintf(trace_header, "%s INSTRUCTION_OPERATION_OTHER:%" PRIu64 "\n",trace_header, thread_data[i].counter[INSTRUCTION_OPERATION_OTHER]);
        */
        std::string tempStr = trace_header.str();
       
        DynamicTraceFile.write(tempStr.c_str(), strlen(tempStr.c_str()));
    }
    DynamicTraceFile.close();
};

//==============================================================================
// Main
//==============================================================================
int main(int argc, char *argv[]) {
    std::stringstream trace_header;
    trace_header    << "#\n"
                    << "# Compressed Profile Generated By Pin to SiNUCA\n"
                    << "#\n";
    std::string tempStr = trace_header.str();

    /*
    char trace_header[300];
    sprintf(trace_header, "#\n");
    sprintf(trace_header, "%s# Compressed Profile Generated By Pin to SiNUCA\n", trace_header);
    sprintf(trace_header, "%s#\n", trace_header);
    */

    if (PIN_Init(argc, argv)) {
        return Usage();
    }
    PIN_InitSymbols();

    max_threads = KnobNumberThreads.Value();
    DEBUG_PRINTF("Threads = %d\n", max_threads);

    /// Initialize the pin lock
    PIN_InitLock(&lock);
    thread_data = new thread_data_t[max_threads];

    //==========================================================================
    // Dynamic Trace Files
    //==========================================================================
    static char dyn_file_name[500];
    dyn_file_name[0] = '\0';
    sprintf(dyn_file_name, "%s.out", KnobOutputFile.Value().c_str());
    DynamicTraceFile.open(dyn_file_name);
    DynamicTraceFile.write(tempStr.c_str(), strlen(tempStr.c_str()));
    DEBUG_PRINTF("Real Dynamic File = %s => READY !\n", dyn_file_name);


    //=======================================================================
    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Static and Dynamic Trace
    TRACE_AddInstrumentFunction(trace_instruction, 0);

    // Close the Files
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
};

//==============================================================================
// eof
//==============================================================================
