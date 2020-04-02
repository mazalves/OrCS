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
// #include "../../../../sinuca.hpp"
#include "../../../../utils.cpp"
#include "../../../../packages/opcode_package.cpp"
#include "../../../../enumerations.cpp"

#undef ERROR // Required to avoid using backtrace

#include "pin.H"
#include "instlib.H"
// ~ #include "control_manager.H"
#include "xed-interface.h"



// ~ NEW
#include "pinplay.H"
PINPLAY_ENGINE pinplay_engine;


// #include "intrinsics_extension.cpp"
#include "intrinsics_extension.cpp"
// ~ KNOB<BOOL> KnobLogger(KNOB_MODE_WRITEONCE, "pintool", "log", "0", "Activate the pinplay logger");
// ~ KNOB<BOOL> KnobReplayer(KNOB_MODE_WRITEONCE, "pintool", "replay", "0", "Activate the pinplay replayer");

#define KNOB_LOG_NAME  "log"
#define KNOB_REPLAY_NAME "replay"
#define KNOB_FAMILY "pintool:pinplay-driver"

KNOB_COMMENT pinplay_driver_knob_family(KNOB_FAMILY, "PinPlay Driver Knobs");

KNOB<BOOL>KnobReplayer(KNOB_MODE_WRITEONCE, KNOB_FAMILY,
                       KNOB_REPLAY_NAME, "0", "Replay a pinball");
KNOB<BOOL>KnobLogger(KNOB_MODE_WRITEONCE,  KNOB_FAMILY,
                     KNOB_LOG_NAME, "0", "Create a pinball");



// #ifdef TRACE_GENERATOR_DEBUG
//     #define TRACE_GENERATOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
// #else
//     #define TRACE_GENERATOR_DEBUG_PRINTF(...)
// #endif

//==============================================================================
// Global Variables
//==============================================================================

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
// #define PADSIZE 64
//
// class thread_data_t {
//     public:
//         PIN_LOCK dyn_lock;
//         bool is_instrumented_bbl;
//
//         gzFile gzDynamicTraceFile;
//         gzFile gzMemoryTraceFile;
//
//         UINT8 pad[PADSIZE];
// };
//
// thread_data_t* thread_data;     /// Lock for methods shared among multiple threads

LOCALVAR CONTROLLER::CONTROL_MANAGER control;

// ~ CONTROLLER::CONTROL control;       /// Control to PinPoints
// PIN_LOCK lock;                  /// Lock for methods shared among multiple threads
// bool is_instrumented = false;   /// Will be enabled by PinPoints
//
// std::ofstream StaticTraceFile;
// gzFile gzStaticTraceFile;
//
// uint32_t count_trace = 0;       /// Current BBL trace number
uint32_t max_threads = 0;       /// Max number of threads (if intel Add +1)

int32_t count_parallel_start = 0;
int32_t count_parallel_end = 0;

//==============================================================================
// Commandline Switches
//==============================================================================
// ~ KNOB_COMMENT pinplay_driver_knob_family("pintool:sinuca_tracer", "SiNUCA Tracer Driver Knobs");

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "output", "output_trace.out", "specify the trace base file name.");
KNOB<uint32_t> KnobNumberThreads(KNOB_MODE_WRITEONCE, "pintool",
    "threads", "1", "specify the total number of threads.");
KNOB<int32_t> KnobParallelStart(KNOB_MODE_WRITEONCE, "pintool",
    "parallel_start", "-1", "parallel start counter to enable the tracer.");
KNOB<int32_t> KnobParallelEnd(KNOB_MODE_WRITEONCE, "pintool",
    "parallel_end", "-1", "parallel end counter to enable the tracer.");

//==============================================================================
// Sinuca
//==============================================================================
const char* get_label() {
    return "Trace Generator";
};

sinuca_engine_t sinuca_engine;
sinuca_engine_t::sinuca_engine_t() {};
sinuca_engine_t::~sinuca_engine_t() {};
void sinuca_engine_t::global_panic() {};


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
    cerr << "Example of Usage: >$ ../../../pin -t obj-intel64/trace.so -output <output_file> -threads <threads> -parallel_start <parallel_start> -parallel_end <parallel_end>  -- <executable>\n";
    cerr << endl;

    return -1;
};

//==============================================================================
// VOID write_static_char(char *stat_str) {
//     TRACE_GENERATOR_DEBUG_PRINTF("write_static_char()\n");
//
//     PIN_GetLock(&lock, 1);
//         gzwrite(gzStaticTraceFile, stat_str, strlen(stat_str));
//     PIN_ReleaseLock(&lock);
// };

//==============================================================================
VOID control_instrumented_bbl(THREADID threadid){
    TRACE_GENERATOR_DEBUG_PRINTF("write_dynamic_char()\n");

    /// If the pin-points disabled this region
    if (!is_instrumented) {
        ///=================================================================
        /// If (parallel instrumentation)
        bool is_last = true;
        for (uint32_t i = 0 ; i < max_threads; i++) {
            if (i != threadid && thread_data[i].is_instrumented_bbl == true) {
                is_last = false;
                break;
            }
        }

        /// If this is the (last thread active) and (the trace was active) and (this is a parallel trace)
        if (is_last == true &&
            thread_data[threadid].is_instrumented_bbl == true &&
            KnobParallelStart.Value() >= 0 && KnobParallelEnd.Value() >= 0) {

            /// Can stop the memory trace
            thread_data[threadid].is_instrumented_bbl = false;

            /// EXIT the application after trace the slice wanted **********
            PIN_ExitApplication(EXIT_SUCCESS);
        }
        ///=================================================================

        // Can stop the memory trace
        thread_data[threadid].is_instrumented_bbl = false;
        return;
    }
    else {
        // Can start the memory trace
        thread_data[threadid].is_instrumented_bbl = true;
    }
};

//==============================================================================
// VOID write_dynamic_char(char *dyn_str, THREADID threadid) {
//     TRACE_GENERATOR_DEBUG_PRINTF("write_dynamic_char()\n");
//
//     /// If the pin-points disabled this region
//     if (!is_instrumented) {
//         return;
//     }
//     else {
//
//         // This lock is necessary because when using a parallel program
//         // the thread master may write on multiple threads
//         // ex: omp_parallel_start / omp_parallel_end
//         PIN_GetLock(&thread_data[threadid].dyn_lock, threadid);
//             // ~ gzwrite(thread_data[threadid].gzDynamicTraceFile, dyn_str, strlen(dyn_str));
//             gzwrite(thread_data[threadid].gzDynamicTraceFile, dyn_str, strlen(dyn_str));
//         PIN_ReleaseLock(&thread_data[threadid].dyn_lock);
//     }
// };


// =====================================================================
VOID write_memory(BOOL is_Read, ADDRINT addr, INT32 size, UINT32 bbl, THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("write_memory()\n");

    if (thread_data[threadid].is_instrumented_bbl == false) return;     /// If the pin-points disabled this region

    char mem_str[TRACE_LINE_SIZE];
    char rw;

    if (is_Read) {
        rw = 'R';
    }
    else {
        rw = 'W';
    }

    sprintf(mem_str, "%c %d %" PRIu64 " %d\n", rw, size, (uint64_t)addr, bbl);

    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));
};

//==============================================================================
// Translate the x86 Instructions to Simulator Instruction
void x86_to_static(const INS& ins) {
    TRACE_GENERATOR_DEBUG_PRINTF("x86_to_static()\n");
    ERROR_ASSERT_PRINTF( INS_MaxNumRRegs(ins) <= MAX_REGISTERS && INS_MaxNumWRegs(ins) <= MAX_REGISTERS, "More registers than MAX_REGISTERS\n");
    uint32_t i;
    opcode_package_t NewInstruction;

    strcpy(NewInstruction.opcode_assembly, INS_Mnemonic(ins).c_str());
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

    NewInstruction.is_read = INS_IsMemoryRead(ins);
    NewInstruction.is_read2 = INS_HasMemoryRead2(ins);
    NewInstruction.is_write = INS_IsMemoryWrite(ins);

    NewInstruction.is_predicated = INS_IsPredicated(ins);
    NewInstruction.is_prefetch = INS_IsPrefetch(ins);


    if (NewInstruction.is_read) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_LOAD;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM LOAD:%s\n", INS_Disassemble(ins).c_str());
    }

    if (NewInstruction.is_read2) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_LOAD;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM LOAD:%s\n", INS_Disassemble(ins).c_str());
    }

    if (NewInstruction.is_write) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_STORE;
        TRACE_GENERATOR_DEBUG_PRINTF("\t MEM STORE:%s\n", INS_Disassemble(ins).c_str());
    }

    if (INS_IsBranchOrCall(ins) || INS_IsSyscall(ins)) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_BRANCH;
        TRACE_GENERATOR_DEBUG_PRINTF("\t BRANCH:%s\n", INS_Disassemble(ins).c_str());

        // Call
        if (INS_IsCall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Call    ");
            NewInstruction.branch_type = BRANCH_CALL;
        }
        // Return
        else if (INS_IsRet(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Return  ");
            NewInstruction.branch_type = BRANCH_RETURN;
        }
        // Syscall
        else if (INS_IsSyscall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Syscall ");
            NewInstruction.branch_type = BRANCH_SYSCALL;
        }
        // Branch/Call
        else {
            // Conditional?
            if (INS_HasFallThrough(ins)){
                TRACE_GENERATOR_DEBUG_PRINTF("Branch  ");
                NewInstruction.branch_type = BRANCH_COND;
            }
            else {
                TRACE_GENERATOR_DEBUG_PRINTF("Jump    ");
                NewInstruction.branch_type = BRANCH_UNCOND;
            }
        }


        NewInstruction.is_indirect = INS_IsIndirectBranchOrCall(ins);

        if (INS_IsDirectBranchOrCall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Direct \n");
        }
        else if (INS_IsIndirectBranchOrCall(ins)) {
            TRACE_GENERATOR_DEBUG_PRINTF("Indirect \n");
        }
        else if (INS_IsSyscall(ins)){
            TRACE_GENERATOR_DEBUG_PRINTF("Syscall \n");
        }
        else {
            TRACE_GENERATOR_DEBUG_PRINTF("**ERROR**\n");
        }
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
        //==================================================================
        // BRANCH

        // ~ case XED_ICLASS_CALL_FAR:       // Call Procedure
        // ~ case XED_ICLASS_CALL_NEAR:      // Call Procedure
        // ~ case XED_ICLASS_RET_FAR:        // Return from procedure
        // ~ case XED_ICLASS_RET_NEAR:       // Return from procedure



        case XED_ICLASS_PAUSE:          // PAUSE (for Cacheability and spin lock)
        case XED_ICLASS_NOP:
        case XED_ICLASS_FNOP:
                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_NOP;
                                TRACE_GENERATOR_DEBUG_PRINTF("\t NOP:%s\n", INS_Disassemble(ins).c_str());
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

                                if (NewInstruction.is_read || NewInstruction.is_read2 || NewInstruction.is_write) {
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

        case XED_ICLASS_XGETBV:         // Get Value of Extended Control Register
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
        case XED_ICLASS_SAR:            // Shift Arithmetically right (signed shift right)
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
                                TRACE_GENERATOR_DEBUG_PRINTF("\t INT ALU:%s\n", INS_Disassemble(ins).c_str());
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
                                if (NewInstruction.is_read || NewInstruction.is_read2 || NewInstruction.is_write) {
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

        case XED_ICLASS_FRNDINT:        // Floating-Point Round to Integer
        case XED_ICLASS_FABS:           // Floating-Point Absolute Value
        case XED_ICLASS_FWAIT:          // Check pending unmasked floating-point exceptions
        case XED_ICLASS_MWAIT:          // Monitor Wait
        case XED_ICLASS_FSTP:           // Store Floating Point Value and Pop
        case XED_ICLASS_FNSTSW:         // Store Floating-Point Status Word
        case XED_ICLASS_FXAM:           // Examine Class of Value
        case XED_ICLASS_FXCH:           // Exchange Register Contents
        case XED_ICLASS_FCHS:           // Floating-Point Change Sign

        case XED_ICLASS_FUCOM:          // Floating-Point Unordered Compare
        case XED_ICLASS_FUCOMI:         // Floating-Point Unordered Compare
        case XED_ICLASS_FUCOMIP:        // Floating-Point Unordered Compare
        case XED_ICLASS_FUCOMP:         // Floating-Point Unordered Compare
        case XED_ICLASS_FUCOMPP:        // Floating-Point Unordered Compare

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

        case XED_ICLASS_UNPCKLPD:       // Unpack and Interleave Low Packed Double-FP Values
        case XED_ICLASS_UNPCKLPS:       // Unpack and Interleave Low Packed Double-FP Values

        case XED_ICLASS_UNPCKHPD:       // Unpack and Interleave High Packed Double-FP Values
        case XED_ICLASS_UNPCKHPS:       // Unpack and Interleave High Packed Double-FP Values

        case XED_ICLASS_PMOVMSKB:       // Move Integer SSE
        case XED_ICLASS_CVTTSD2SI:      // Convert from Double
        case XED_ICLASS_CVTSI2SD:       // Convert to Double
        case XED_ICLASS_CVTSD2SI:       // Convert to Double
        case XED_ICLASS_CVTSI2SS:       // Convert to Double
        case XED_ICLASS_CVTSS2SD:       // Convert to Double
        case XED_ICLASS_CVTDQ2PD:       // Convert to Double
        case XED_ICLASS_CVTTPD2DQ:      // Convert to Double
        case XED_ICLASS_COMISD:         // Compare Scalar Ordered Double-FP Values and Set EFLAGS
        case XED_ICLASS_UCOMISD:        // Unordered Compare Scalar Single-FP Values and Set EFLAGS
        case XED_ICLASS_CMPSD_XMM:      // Compare Scalar Double-FP Values

        case XED_ICLASS_PCMPEQD:        // Compare Packed Data for Equal
        case XED_ICLASS_PCMPISTRI:      // Packed Compare Implicit Length Strings, Return Index

        case XED_ICLASS_PCMPGTB:        // Compare Packed Data for Greater Than
        case XED_ICLASS_PCMPGTD:        // Compare Packed Data for Greater Than
        case XED_ICLASS_PCMPGTW:        // Compare Packed Data for Greater Than

        case XED_ICLASS_COMISS:         // Compare Scalar Single-FP Values and Set EFLAGS
        case XED_ICLASS_UCOMISS:        // Unordered Compare Scalar Single-FP Values and Set EFLAGS

        case XED_ICLASS_CMPSS:          // Compare Scalar Single-FP Values
        case XED_ICLASS_CMPSD:          // Compare Scalar Double-FP Values
        case XED_ICLASS_CMPPS:          // Compare Packed Single-FP Values
        case XED_ICLASS_CMPPD:          // Compare Packed Double-FP Values

        case XED_ICLASS_ANDPS:          // Bitwise Logical AND of Packed Single-FP Values
        case XED_ICLASS_ANDPD:          // Bitwise Logical AND of Packed Double-FP Values

        case XED_ICLASS_ANDNPS:         // Bitwise Logical NAND of Packed Single-FP Values
        case XED_ICLASS_ANDNPD:         // Bitwise Logical NAND of Packed Double-FP Values

        case XED_ICLASS_XORPS:          // Bitwise Logical XOR for Packed Single-FP Values
        case XED_ICLASS_XORPD:          // Bitwise Logical XOR for Packed Double-FP Values

        case XED_ICLASS_ORPS:           // Bitwise Logical OR for Packed Single-FP Values
        case XED_ICLASS_ORPD:           // Bitwise Logical OR for Packed Double-FP Values

        case XED_ICLASS_PMAXUB:         // Return Maximum Scalar Single-FP Value
        case XED_ICLASS_MAXSS:          // Return Maximum Scalar Single-FP Value
        case XED_ICLASS_MAXSD:          // Return Maximum Scalar Double-FP Value
        case XED_ICLASS_MAXPS:          // Return Maximum Packed Single-FP Values
        case XED_ICLASS_MAXPD:          // Return Maximum Packed Double-FP Values

        case XED_ICLASS_PMINUB:         // Return Minimum Packed Single-FP Values
        case XED_ICLASS_MINPS:          // Return Minimum Packed Single-FP Values
        case XED_ICLASS_MINPD:          // Return Minimum Packed Double-FP Values
        case XED_ICLASS_MINSS:          // Return Minimum Scalar Single-FP Value
        case XED_ICLASS_MINSD:          // Return Minimum Scalar Double-FP Value

        case XED_ICLASS_ADDPD:          // Add Packed Double-Precision Floating-Point Values
        case XED_ICLASS_ADDPS:          // Add Packed Single-Precision Floating-Point Values
        case XED_ICLASS_ADDSS:          // Add Scalar Single-Precision Floating-Point
        case XED_ICLASS_ADDSD:          // Add Low Double-Precision Floating-Point Value

        case XED_ICLASS_ROUNDPD:        // Round Packed Double-Precision Floating-Point Values
        case XED_ICLASS_ROUNDPS:        // Round Packed Single-Precision Floating-Point Values
        case XED_ICLASS_ROUNDSS:        // Round Scalar Single-Precision Floating-Point
        case XED_ICLASS_ROUNDSD:        // Round Low Double-Precision Floating-Point Value

        case XED_ICLASS_SUBPD:          // Sub Packed Double-Precision Floating-Point Values
        case XED_ICLASS_SUBPS:          // Sub Packed Single-Precision Floating-Point Values
        case XED_ICLASS_SUBSS:          // Sub Scalar Single-Precision Floating-Point
        case XED_ICLASS_SUBSD:          // Sub Low Double-Precision Floating-Point Value

                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_FP_ALU;
                                TRACE_GENERATOR_DEBUG_PRINTF("\t FP ALU:%s\n", INS_Disassemble(ins).c_str());
                                break;

        //==================================================================
        // INT MUL
        case XED_ICLASS_MUL:            // Unsigned multiply
        case XED_ICLASS_IMUL:           // Signed multiply
                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_MUL;
                                TRACE_GENERATOR_DEBUG_PRINTF("\t INT MUL:%s\n", INS_Disassemble(ins).c_str());
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

                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_FP_MUL;
                                TRACE_GENERATOR_DEBUG_PRINTF("\t FP MUL:%s\n", INS_Disassemble(ins).c_str());
                                break;

        //==================================================================
        // INT DIV
        case XED_ICLASS_DIV:            // Divide
        case XED_ICLASS_IDIV:           // Integer Divide
                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_DIV;
                                TRACE_GENERATOR_DEBUG_PRINTF("\t INT DIV:%s\n", INS_Disassemble(ins).c_str());
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
        case XED_ICLASS_FPREM:          // Floating-Point Partial Remainder
        case XED_ICLASS_FPREM1:         // Floating-Point Partial Remainder

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
                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_FP_DIV;
                                TRACE_GENERATOR_DEBUG_PRINTF("\t FP DIV:%s\n", INS_Disassemble(ins).c_str());
                                break;

        //==================================================================
        default:
                                if (NewInstruction.opcode_operation == INSTRUCTION_OPERATION_NOP) {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_OTHER;
                                    TRACE_GENERATOR_DEBUG_PRINTF("\t OTHER:%s\n", INS_Disassemble(ins).c_str());
                                }
                                break;
    }

    char opcode_str[TRACE_LINE_SIZE];
    NewInstruction.opcode_to_trace_string(opcode_str);
    write_static_char(opcode_str);
};


//==============================================================================
VOID trace_instruction(TRACE trace, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("trace_instruction()\n");
    char bbl_count_str[TRACE_LINE_SIZE];

    std::string rtn_name;
    RTN rtn = TRACE_Rtn(trace);
    if(RTN_Valid(rtn)) {
        RTN_Open(rtn);
        rtn_name = RTN_Name(rtn);
        sprintf(bbl_count_str, "#%s\n", rtn_name.c_str());
        write_static_char(bbl_count_str);    // Write the static trace
        RTN_Close(rtn);
    }
    if (strcmp(rtn_name.c_str(), "omp_get_num_procs") == 0 ||           // <== SpinLock Static Linked (its binary has a PAUSE instruction)
        strcmp(rtn_name.c_str(), "gomp_barrier_wait_end") == 0 ||       // <== SpinLock Static Linked (its binary has a PAUSE instruction)
        strcmp(rtn_name.c_str(), "gomp_team_barrier_wait_end") == 0){   // <== SpinLock Static Linked (its binary has a PAUSE instruction)
        return;
    }

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {

        // HMC Traces
        if (((KnobTrace.Value().compare(0, 3, "x86")) != 0) && icheck_conditions(rtn_name))
            continue;

        //----------------------------------------------------------------------
        // Write the static trace (assembly instructions)
        //----------------------------------------------------------------------
        count_trace++;              // Identify basic blocks with a counter id
        sprintf(bbl_count_str, "@%u\n", count_trace);
        write_static_char(bbl_count_str);    // Write the static trace

        //----------------------------------------------------------------------
        // Write the dynamic trace (basic block numbers)
        //----------------------------------------------------------------------
        char *bbl_str = new char[32];
        sprintf(bbl_str, "%u\n", count_trace);

        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(control_instrumented_bbl),
                        IARG_THREAD_ID, IARG_END);

        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {
            x86_to_static(ins);       // pin::ins => static trace

            //--------------------------------------------------------------------------
            // Write the Memory
            // is_instrumenteds loads using a predicated call, i.e.
            // the call happens if the load will be actually executed
            //--------------------------------------------------------------------------
            if (INS_IsMemoryRead(ins)) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_memory, IARG_BOOL, true, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
            }
            if (INS_HasMemoryRead2(ins)) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_memory, IARG_BOOL, true, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE, IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
            }
            if (INS_IsMemoryWrite(ins)) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_memory, IARG_BOOL, false, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
            }
        }

        // HMC Traces
        if (((KnobTrace.Value().compare(0, 3, "x86")) != 0) && icheck_conditions(rtn_name))
            continue;

        INS_InsertCall(BBL_InsTail(bbl), IPOINT_BEFORE, AFUNPTR(write_dynamic_char), IARG_PTR, bbl_str, IARG_THREAD_ID, IARG_END);

    }
};

//==============================================================================
VOID Fini(INT32 code, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("Fini()\n");
    char trace_end[TRACE_LINE_SIZE];
    sprintf(trace_end, "# eof\n");

    gzwrite(gzStaticTraceFile, trace_end, strlen(trace_end));
    gzclose(gzStaticTraceFile);
    for (UINT32 i = 0; i < max_threads; i++){
        gzwrite(thread_data[i].gzDynamicTraceFile, trace_end, strlen(trace_end));
        gzclose(thread_data[i].gzDynamicTraceFile);

        gzwrite(thread_data[i].gzMemoryTraceFile, trace_end, strlen(trace_end));
        gzclose(thread_data[i].gzMemoryTraceFile);
    }
};

//==============================================================================
// Control the Number of Parallel Regions to trace
//==============================================================================
VOID handleParallelControlEvent(bool is_start, THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("handleParallelControlEvent()\n");

    /// If no parallel instrumentation was done => return
    if (KnobParallelStart.Value() < 0 && KnobParallelEnd.Value() < 0) return;

    PIN_GetLock(&lock, threadid);
        if (is_start) {
            /// New PARALLEL START
            count_parallel_start++;
        }
        else {
            /// New PARALLEL END
            count_parallel_end++;
        }

        bool old_status = is_instrumented;

        /// Reach the start parallel region
        if (count_parallel_start >= KnobParallelStart.Value() && KnobParallelStart.Value() >= 0) {
            TRACE_GENERATOR_DEBUG_PRINTF("\t Parallel START tracing\n");
            is_instrumented = true;
        }

        /// Reach the end parallel region
        if (count_parallel_end >= KnobParallelEnd.Value() && KnobParallelEnd.Value() >= 0) {
            TRACE_GENERATOR_DEBUG_PRINTF("\t Parallel STOP tracing\n");
            is_instrumented = false;

            // ~ ///=================================================================
            // ~ /// EXIT the application after trace the slice wanted
            // ~ printf("\t Parallel %d[%s] => %d[%s]\n", count_parallel_start, old_status ? "TRUE":"FALSE", count_parallel_end, is_instrumented ? "TRUE":"FALSE");
            // ~ PIN_ExitApplication(EXIT_SUCCESS);
            // ~ ///=================================================================
        }

    if (old_status != is_instrumented) {
        printf("\t Parallel Start(%d) End(%d) - Trace Status [%s] => [%s]\n", count_parallel_start, count_parallel_end, old_status ? "ON":"OFF", is_instrumented ? "ON":"OFF");
    }
    PIN_ReleaseLock(&lock);
};

//==============================================================================
// Pin Point Events
//==============================================================================
// ~ VOID handleControlEvent(CONTROLLER::EVENT_TYPE ev, VOID *, CONTEXT * ctxt, VOID *, THREADID threadid) {
VOID handleControlEvent(CONTROLLER::EVENT_TYPE ev, VOID *val, CONTEXT * ctxt, VOID *ip, THREADID threadid, bool bcast) {
    TRACE_GENERATOR_DEBUG_PRINTF("handleControlEvent()\n");

    /// If (parallel instrumentation) => return
    if (KnobParallelStart.Value() >= 0 || KnobParallelEnd.Value() >= 0) return;

    PIN_GetLock(&lock, threadid);
        bool old_status = is_instrumented;

        switch(ev) {
            case CONTROLLER::EVENT_START:
                TRACE_GENERATOR_DEBUG_PRINTF("\t PinPoint START tracing\n");
                is_instrumented = true;
            break;

            case CONTROLLER::EVENT_STOP:
                TRACE_GENERATOR_DEBUG_PRINTF("\t PinPoint STOP tracing\n");
                is_instrumented = false;
            break;

            default:
                ASSERTX(false);
            break;
        }

        if (old_status != is_instrumented) {
            TRACE_GENERATOR_DEBUG_PRINTF("\t PinPoint %s => %s\n", old_status ? "TRUE":"FALSE", is_instrumented ? "TRUE":"FALSE");
        }

    PIN_ReleaseLock(&lock);
};

//==============================================================================
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("ThreadStart()\n");
    PIN_GetLock(&lock, threadid);
        ASSERTX(threadid < max_threads);
    PIN_ReleaseLock(&lock);
};

//==============================================================================
VOID DynamicOMP_char(char *sync_str, THREADID threadid, bool is_spawn) {
    TRACE_GENERATOR_DEBUG_PRINTF("==>(%d) %s", threadid, sync_str);
    write_dynamic_char(sync_str, threadid);

    if (is_spawn) {
        for (uint32_t j = 1; j < max_threads; j++) {
            TRACE_GENERATOR_DEBUG_PRINTF("==>(%d)>(%d) %s", threadid, j, sync_str);
            write_dynamic_char(sync_str, j);
        }
    }
};



//==============================================================================
// Instrumentation Routines
//==============================================================================
// This routine is executed for each image.
VOID ImageLoad(IMG img, VOID *) {
    printf("Loading %s, Image id = %d\n", IMG_Name(img).c_str(), IMG_Id(img));

    // HMC data initialization - HMC Traces
    data_instr hmc_x86_data[20], vim_x86_data[112], mps_x86_data[28];
    initialize_intrinsics(hmc_x86_data, vim_x86_data, mps_x86_data);

    TRACE_GENERATOR_DEBUG_PRINTF("ImageLoad()\n");
    /// Only the thread master runs these calls
    std::vector<const char*> OMP_barrier_master_start;
        OMP_barrier_master_start.push_back("GOMP_parallel_start");
        OMP_barrier_master_start.push_back("GOMP_parallel_loop_dynamic_start");
        OMP_barrier_master_start.push_back("GOMP_parallel_loop_static_start");

    std::vector<const char*> OMP_barrier_master_end;
        OMP_barrier_master_end.push_back("GOMP_parallel_end");

    /// All the threads runs these calls
    std::vector<const char*> OMP_barrier_simple;
        OMP_barrier_simple.push_back(("GOMP_barrier"));
        OMP_barrier_simple.push_back(("GOMP_loop_dynamic_start"));
        OMP_barrier_simple.push_back(("GOMP_loop_ordered_static_start"));
        OMP_barrier_simple.push_back(("GOMP_loop_guided_start"));
        OMP_barrier_simple.push_back(("GOMP_loop_end"));
        OMP_barrier_simple.push_back(("GOMP_parallel_sections_start"));
        OMP_barrier_simple.push_back(("GOMP_sections_end"));

    /// Enters in a critical section
    std::vector<const char*> OMP_critical_start;
        OMP_critical_start.push_back(("GOMP_atomic_start"));
        OMP_critical_start.push_back(("GOMP_critical_start"));
        OMP_critical_start.push_back(("GOMP_critical_name_start"));

    /// Exits in a critical section
    std::vector<const char*> OMP_critical_end;
        OMP_critical_end.push_back(("GOMP_atomic_end"));
        OMP_critical_end.push_back(("GOMP_critical_end"));
        OMP_critical_end.push_back(("GOMP_critical_name_end"));


    /// No Sync or very fast sync that will be ignored
    std::vector<const char*> OMP_ignore;
        /// No Wait
        OMP_ignore.push_back(("GOMP_loop_end_nowait"));
        OMP_ignore.push_back(("GOMP_sections_end_nowait"));
        /// Check if it is running alone
        OMP_ignore.push_back(("GOMP_single_start"));
        /// Reduction
        OMP_ignore.push_back(("GOMP_ordered_start"));
        OMP_ignore.push_back(("GOMP_ordered_end"));
        /// Lock to get the next chunck of iterations
        OMP_ignore.push_back(("GOMP_loop_dynamic_next"));
        OMP_ignore.push_back(("GOMP_loop_ordered_static_next"));
        OMP_ignore.push_back(("GOMP_loop_guided_next"));
        OMP_ignore.push_back(("GOMP_sections_next"));
        /// "loop_next" should be implemented with instrument BEFORE and AFTER
        /// Pin’s implementation of RTN_InsertCall() with IPOINT_AFTER is not perfect.
        /// Pin statically scans the instruction in the function looking for RET.
        /// However, weird code due to compiler optimizations or hand-coded assembly can confuse Pin, and prevent it from finding all the RET’s.
        /// Note that RTN_InsertCall() with IPOINT_BEFORE does not have this problem.

    bool found_GOMP;
    std::string rtn_name;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // RTN_InsertCall() and INS_InsertCall() are executed in order of
        // appearance.  The IPOINT_AFTER may be executed before the IPOINT_BEFORE.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            found_GOMP = false;
            rtn_name = RTN_Name(rtn);

            // Syntetic Traces (HMC, x86, VIMA, MIPS)
            if ((KnobTrace.Value().compare(0, 3, "x86")) != 0) {
                synthetic_trace_generation(rtn_name, hmc_x86_data, vim_x86_data, mps_x86_data, rtn);
            }

            // ~ if (
            // ~ //==========================================================
            // ~ // Dynamic Linked Programs...
            // ~ // No Pause but inlined during compilation
            // ~ // strcmp(rtn_name.c_str(), "GOMP_barrier") == 0 ||    // <== Dynamic Linked, works only for spinlock end, because the RTN in inline
// ~
            // ~ //==========================================================
            // ~ // libgomp.so.1
            // ~ // strcmp(rtn_name.c_str(), "omp_set_nest_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "omp_test_nest_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "omp_get_num_procs") >= 0 ||
// ~
            // ~ //==========================================================
            // ~ // libgomp.a - Requires Static Linkage
            // ~ // strcmp(rtn_name.c_str(), "gomp_mutex_lock_slow") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "gomp_sem_wait_slow") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "gomp_ptrlock_get_slow") >= 0 ||
            // ~ strcmp(rtn_name.c_str(), "gomp_barrier_wait_end") == 0 ||      // <== Static Linked
            // ~ strcmp(rtn_name.c_str(), "gomp_team_barrier_wait_end") == 0    // <== Static Linked
// ~
            // ~ //==========================================================
            // ~ // libpthread.so.0
            // ~ // libpthread.a
            // ~ // strcmp(rtn_name.c_str(), "pthread_mutex_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "pthread_mutex_cond_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "pthread_spinlock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "pthread_mutex_timedlock") >= 0 ||
            // ~ ) {
                // ~ RTN_Open(rtn);
                // ~ // SPIN LOCK START
                // ~ char *sync_str = new char[200];
                // ~ sprintf(sync_str, "#BEGIN:\"%s\"\n", rtn_name.c_str());
                // ~ RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);
// ~
                // ~ // Examine each instruction in the routine.
                // ~ for( INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins) ) {
                    // ~ // instrument each return instruction.
                    // ~ if( INS_IsRet(ins) ) {
                        // ~ // SPIN LOCK END
                        // ~ char *sync_str = new char[200];
                        // ~ sprintf(sync_str, "#END:\"%s\"\n", rtn_name.c_str());
                        // ~ // IPOINT_TAKEN_BRANCH always occurs last.
                        // ~ // INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);
                    // ~ }
                // ~ }
// ~
                // ~ RTN_Close(rtn);
            // ~ }

            if (rtn_name.compare(0, 4, "GOMP") != 0) {
                continue;
            }
            // ~ printf("%s\n", rtn_name.c_str());

            /// Barrier only on Master, insert on all the traces (PARALLEL_END)
            for (uint32_t i = 0; i < OMP_barrier_master_end.size() && !found_GOMP; i++){
                /// FOUND a Parallel END
                if (strcmp(rtn_name.c_str(), OMP_barrier_master_end[i]) == 0) {
                    RTN_Open(rtn);

                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_BARRIER);
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, true, IARG_END);
                    /// Parallel End
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(handleParallelControlEvent), IARG_BOOL, false, IARG_THREAD_ID, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }
            /// Barrier only on Master, insert on all the traces (PARALLEL_START)
            for (uint32_t i = 0; i < OMP_barrier_master_start.size() && !found_GOMP; i++){
                /// FOUND a Parallel START
                if (strcmp(rtn_name.c_str(), OMP_barrier_master_start[i]) == 0) {
                    RTN_Open(rtn);

                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_BARRIER);
                    /// Parallel Start
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(handleParallelControlEvent), IARG_BOOL, true, IARG_THREAD_ID, IARG_END);

                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, true, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Barrier on all the threads
            for (uint32_t i = 0; i < OMP_barrier_simple.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_barrier_simple[i]) == 0) {
                    RTN_Open(rtn);

                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_BARRIER);
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Enter in a critical section
            for (uint32_t i = 0; i < OMP_critical_start.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_critical_start[i]) == 0) {
                    RTN_Open(rtn);

                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_CRITICAL_START);
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Exit the critical section
            for (uint32_t i = 0; i < OMP_critical_end.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_critical_end[i]) == 0) {
                    RTN_Open(rtn);

                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_CRITICAL_END);
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Ignore the primitive OpenMP
            for (uint32_t i = 0; i < OMP_ignore.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_ignore[i]) == 0) {
                    #ifdef TRACE_GENERATOR_DEBUG
                        RTN_Open(rtn);

                        char *sync_str = new char[TRACE_LINE_SIZE];
                        sprintf(sync_str, "#Ignoring GOMP call:\"%s\"\n", rtn_name.c_str());
                        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                        RTN_Close(rtn);
                    #endif
                    found_GOMP = true;
                    break;
                }
            }

            if (!found_GOMP) {
                /// If a different OpenMP call is found on the binary
                RTN_Open(rtn);

                char *sync_str = new char[TRACE_LINE_SIZE];
                sprintf(sync_str, "#Found different GOMP call:\"%s\"\n", rtn_name.c_str());
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                RTN_Close(rtn);
            }
        }
    }
};

//==============================================================================
// Main
//==============================================================================
int main(int argc, char *argv[]) {
    char trace_header[TRACE_LINE_SIZE];
    sprintf(trace_header, "#\n");
    sprintf(trace_header, "%s# Compressed Trace Generated By Pin to SiNUCA\n", trace_header);
    sprintf(trace_header, "%s#\n", trace_header);

    if (PIN_Init(argc, argv)) {
        return Usage();
    }
    PIN_InitSymbols();

    max_threads = KnobNumberThreads.Value();
    SINUCA_PRINTF("GCC Threads = %d\n", max_threads);

    /// Initialize the pin lock
    PIN_InitLock(&lock);
    thread_data = new thread_data_t[max_threads];
    for (uint32_t i = 0; i < max_threads; i++) {
        PIN_InitLock(&thread_data[i].dyn_lock);
        thread_data[i].is_instrumented_bbl = false;
    }

    pinplay_engine.Activate(argc, argv, KnobLogger, KnobReplayer);

    // Activate alarm, must be done before PIN_StartProgram
    control.RegisterHandler(handleControlEvent, 0, FALSE);
    control.Activate();

    //==========================================================================
    // Static Trace File
    //==========================================================================
    static char stat_file_name[500];
    stat_file_name[0] = '\0';

    SINUCA_PRINTF("Inserted Output File Name = %s\n", KnobOutputFile.Value().c_str());

    sprintf(stat_file_name , "%s.tid0.stat.out.gz", KnobOutputFile.Value().c_str());
    gzStaticTraceFile = gzopen(stat_file_name, "wb");   /// Open the .gz file
    ASSERTX(gzStaticTraceFile != NULL);                 /// Check the .gz file
    gzwrite(gzStaticTraceFile, trace_header, strlen(trace_header));

    SINUCA_PRINTF("Real Static File = %s => READY !\n",stat_file_name);

    //==========================================================================
    // Dynamic Trace Files
    //==========================================================================
    static char dyn_file_name[500];

    for (UINT32 i = 0; i < max_threads; i++){
        dyn_file_name[0] = '\0';
        sprintf(dyn_file_name, "%s.tid%d.dyn.out.gz", KnobOutputFile.Value().c_str(), i);
        thread_data[i].gzDynamicTraceFile = gzopen(dyn_file_name, "wb");    /// Open the .gz file
        ASSERTX(thread_data[i].gzDynamicTraceFile != NULL);                 /// Check the .gz file
        gzwrite(thread_data[i].gzDynamicTraceFile, trace_header, strlen(trace_header));
        SINUCA_PRINTF("Real Dynamic File = %s => READY !\n", dyn_file_name);
    }

    //==========================================================================
    // Memory Trace Files
    //==========================================================================
    static char mem_file_name[500];

    for (UINT32 i = 0; i < max_threads; i++){
        mem_file_name[0] = '\0';
        sprintf(mem_file_name, "%s.tid%d.mem.out.gz", KnobOutputFile.Value().c_str(), i);
        thread_data[i].gzMemoryTraceFile = gzopen(mem_file_name, "wb");     /// Open the .gz file
        ASSERTX(thread_data[i].gzMemoryTraceFile != NULL);                  /// Check the .gz file
        gzwrite(thread_data[i].gzMemoryTraceFile, trace_header, strlen(trace_header));
        SINUCA_PRINTF("Real Memory File = %s => READY !\n", mem_file_name);
    }

    //=======================================================================
    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Static, Dynamic and Memory Trace
    TRACE_AddInstrumentFunction(trace_instruction, 0);

    // Close the Files
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return EXIT_SUCCESS;
};

//==============================================================================
// eof
//==============================================================================
