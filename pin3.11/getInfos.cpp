
#include <iostream>
#include "pin.H"
#include "instlib.H"
#include "xed-interface.h"
#include "enumerations.hpp"
#include <stdio.h>
#include <sstream>
// *************************************************************************************************************

KNOB<string> KnobConfigFile(KNOB_MODE_WRITEONCE, "pintool", 
                            "c", "config/sandy_bridge/sandy_bridge.cfg", "specify OrCS config file name");
// *************************************************************************************************************

#define MAX_REGISTERS 6
#define POSITION_FAIL -1
#define TRACE_LINE_SIZE 512
FILE *pipeOut;
// *************************************************************************************************************

class ins_info_t {
    public:
        /// TRACE Variables
        char opcode_assembly[TRACE_LINE_SIZE];
        instruction_operation_t opcode_operation;
        uint64_t opcode_address;
        uint32_t opcode_size;

        int32_t read_regs[MAX_REGISTERS];
        int32_t write_regs[MAX_REGISTERS];

        uint32_t base_reg;
        uint32_t index_reg;

        bool is_read;
        uint64_t read_address;
        uint32_t read_size;

        bool is_read2;
        uint64_t read2_address;
        uint32_t read2_size;

        bool is_write;
        uint64_t write_address;
        uint32_t write_size;

        branch_t branch_type;
        bool is_indirect;

        bool is_predicated;
        bool is_prefetch;

        void show() {
            std::ofstream outputLog("Log.log", std::ios_base::app);
            // Escreve no Log
            outputLog << "opcode_assembly"  << ": " << opcode_assembly << std::endl;
            outputLog << "opcode_operation"  << ": " << opcode_operation << std::endl;

            outputLog << "opcode_address"  << ": " << opcode_address << std::endl;

            outputLog << "opcode_size"  << ": " << opcode_size << std::endl;

            for (int i = 0; i < MAX_REGISTERS; ++i) {
                outputLog << "read_regs"  << ": " << read_regs[i] << std::endl;
                outputLog << "write_regs"  << ": " << write_regs[i] << std::endl;
            }

            outputLog << "base_reg"  << ": " << base_reg << std::endl;
            outputLog << "index_reg"  << ": " << index_reg << std::endl;

            outputLog << "################" << std::endl;
            outputLog << "Read 1:" << std::endl;
            outputLog << "################" << std::endl; 
            outputLog << "is_read"  << ": " << ((is_read)          ? '1' : '0')  << std::endl;
            outputLog << "read_address"  << ": " << read_address << std::endl;
            outputLog << "read_size"  << ": " << read_size << std::endl;
            outputLog << std::endl;

            outputLog << "################" << std::endl;
            outputLog << "Read 2:" << std::endl;
            outputLog << "################" << std::endl; 
            outputLog << "is_read2"  << ": " << ((is_read2)          ? '1' : '0')  << std::endl;
            outputLog << "read2_address"  << ": " << read2_address << std::endl;
            outputLog << "read2_size"  << ": " << read2_size << std::endl;
            outputLog << std::endl;

            outputLog << "################" << std::endl;
            outputLog << "Write:" << std::endl;
            outputLog << "################" << std::endl; 
            outputLog << "is_write"  << ": " << ((is_write)          ? '1' : '0')  << std::endl;
            outputLog << "write_address"  << ": " << write_address << std::endl;
            outputLog << "write_size"  << ": " << write_size << std::endl;
            outputLog << std::endl;


            outputLog << "################" << std::endl;
            outputLog << "Branch:" << std::endl;
            outputLog << "################" << std::endl; 

            outputLog << "branch_type"  << ": " << branch_type << std::endl;
            outputLog << "is_indirect"  << ": " << ((is_indirect)          ? '1' : '0')  << std::endl;
            outputLog << "is_predicated"  << ": " << ((is_predicated)          ? '1' : '0')  << std::endl;
            outputLog << "is_prefetch"  << ": " << ((is_prefetch)          ? '1' : '0')  << std::endl;
            outputLog.close();
        }

        void print() {
            std::stringstream streamOut;
            
            streamOut   << opcode_assembly                          << " "
                        << static_cast<int32_t> (opcode_operation)  << " "
                        << opcode_address                           << " "
                        << opcode_size                              << " ";

            for (int i = 0; i < MAX_REGISTERS; ++i) {
                streamOut << read_regs[i] << " " << write_regs[i]   << " ";
            }

            streamOut << base_reg << " " << index_reg               << " ";

            streamOut << ((is_read)          ? '1' : '0')           << " ";
            streamOut << read_address   << " " << read_size         << " ";

            streamOut << ((is_read2)         ? '1' : '0')           << " ";
            streamOut << read2_address  << " " << read2_size        << " ";

            streamOut << ((is_write)         ? '1' : '0')           << " ";
            streamOut << write_address  << " " << write_size        << " ";
                                   
            streamOut << static_cast<int64_t> (branch_type)         << " "; 
            streamOut << ((is_indirect)      ? '1' : '0')           << " ";

            streamOut << ((is_predicated)    ? '1' : '0')           << " ";
            streamOut << ((is_prefetch)      ? '1' : '0')           << " ";

            streamOut << std::endl;

            std::string strAux = streamOut.str();
            fwrite(strAux.c_str(), strAux.size() , sizeof(char), pipeOut);
            

        }

};
// *************************************************************************************************************

ins_info_t NewInstruction;
// *************************************************************************************************************

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    std::string strAux ("000\0");
    fwrite(strAux.c_str(), strAux.size() , sizeof(char), pipeOut);
    pclose(pipeOut);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{

    std::ofstream logErro("err.log");
    logErro <<"This Pintool use pipes as input to OrCS\n" << KNOB_BASE::StringKnobSummary() << "\n";
    logErro.close();
    return 1;
}


void x86_to_static(const INS& ins) {
    uint32_t i;
    uint32_t reg;

    strcpy(NewInstruction.opcode_assembly, INS_Mnemonic(ins).c_str());
    NewInstruction.opcode_address = INS_Address(ins);
    NewInstruction.opcode_size = INS_Size(ins);

    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        NewInstruction.read_regs[i] = POSITION_FAIL;
    }

    reg = INS_MaxNumRRegs(ins);
    if(reg > MAX_REGISTERS) {
        std::cout << "O número usado de registradores de leitura ultrapassa o limite previamente definido!" << std::endl;
        exit(1);
    }
    for (i = 0; i < reg; i++) {
        NewInstruction.read_regs[i] = INS_RegR(ins, i);
    }


    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        NewInstruction.write_regs[i] = POSITION_FAIL;
    }

    reg = INS_MaxNumWRegs(ins);
    if(reg > MAX_REGISTERS) {
        std::cout << "O número usado de registradores de escrita ultrapassa o limite previamente definido!" << std::endl;
        exit(1);
    }
    for (i = 0; i < reg; i++) {
        //std::cout << INS_MaxNumWRegs(ins) << std::endl;
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
    }

    if (NewInstruction.is_read2) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_LOAD;
    
    }

    if (NewInstruction.is_write) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_MEM_STORE;
    
    }

     //  INS_IsBranchOrCall -> INS_IsControlFlow
    if (INS_IsControlFlow(ins) || INS_IsSyscall(ins)) {
        NewInstruction.opcode_operation = INSTRUCTION_OPERATION_BRANCH;
    

        // Call
        if (INS_IsCall(ins)) {
        
            NewInstruction.branch_type = BRANCH_CALL;
        }
        // Return
        else if (INS_IsRet(ins)) {
        
            NewInstruction.branch_type = BRANCH_RETURN;
        }
        // Syscall
        else if (INS_IsSyscall(ins)) {
        
            NewInstruction.branch_type = BRANCH_SYSCALL;
        }
        // Branch/Call
        else {
            // Conditional?
            if (INS_HasFallThrough(ins)){
            
                NewInstruction.branch_type = BRANCH_COND;
            }
            else {
            
                NewInstruction.branch_type = BRANCH_UNCOND;
            }
        }

        // INS_IsIndirectBranchOrCall -> INS_IsIndirectControlFlow
        NewInstruction.is_indirect = INS_IsIndirectControlFlow(ins);
        std::ofstream tempSaida ("s.temp");
        tempSaida << NewInstruction.is_indirect << std::endl;
        tempSaida.close();
        if (INS_IsIndirectControlFlow(ins)) {
        
        }
        else if (INS_IsIndirectControlFlow(ins)) {
        
        }
        else if (INS_IsSyscall(ins)){
        
        }
        else {
        
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
                                
                                }
                                else {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_ALU;
                                
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
                                
                                }
                                else {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_ALU;
                                
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

        case XED_ICLASS_PUNPCKHBW:     // Unpack High Data
        case XED_ICLASS_PUNPCKHQDQ:     // Unpack High Data
        case XED_ICLASS_PUNPCKHDQ:     // Unpack High Data
        case XED_ICLASS_PUNPCKHWD:     // Unpack High Data

        case XED_ICLASS_PUNPCKLBW:      // Unpack Low Data
        case XED_ICLASS_PUNPCKLQDQ:     // Unpack Low Data
        case XED_ICLASS_PUNPCKLDQ:      // Unpack Low Data
        case XED_ICLASS_PUNPCKLWD:      // Unpack Low Data

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
                            
                                break;

        //==================================================================
        // INT MUL
        case XED_ICLASS_MUL:            // Unsigned multiply
        case XED_ICLASS_IMUL:           // Signed multiply
                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_MUL;
                            
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
                            
                                break;

        //==================================================================
        // INT DIV
        case XED_ICLASS_DIV:            // Divide
        case XED_ICLASS_IDIV:           // Integer Divide
                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_INT_DIV;
                            
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
        // XED_ICLASS_PFSQRT -> XED_ICLASS_PFRSQRT
        case XED_ICLASS_PFRSQRT:         // Square root
        // FP SQRT SSE
        case XED_ICLASS_RSQRTPS:        // Compute Recipr. of Square Roots of Packed Single-FP Values
        case XED_ICLASS_SQRTPD:         // Compute Square Roots of Packed Double-FP Values
        case XED_ICLASS_SQRTPS:         // Compute Square Roots of Packed Single-FP Values
        case XED_ICLASS_SQRTSD:         // Compute Square Root of Scalar Double-FP Value
        case XED_ICLASS_SQRTSS:         // Compute Square Root of Scalar Single-FP Value
                                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_FP_DIV;
                            
                                break;

        //==================================================================
        default:
                                if (NewInstruction.opcode_operation == INSTRUCTION_OPERATION_NOP) {
                                    NewInstruction.opcode_operation = INSTRUCTION_OPERATION_OTHER;
                                
                                }
                                break;
    }
    

    
}
// **********************************************************************************************************************

VOID has_read_f(ADDRINT addr, INT32 size) {
    NewInstruction.read_address = static_cast<uint64_t> (addr);
    NewInstruction.read_size = static_cast<uint32_t> (size);

}
// **********************************************************************************************************************

VOID has_read2_f(ADDRINT addr, INT32 size) {
    NewInstruction.read2_address = static_cast<uint64_t> (addr);
    NewInstruction.read2_size = static_cast<uint32_t> (size);

}
// **********************************************************************************************************************

VOID has_write_f(ADDRINT addr, INT32 size) {
    NewInstruction.write_address = static_cast<uint64_t> (addr);
    NewInstruction.write_size = static_cast<uint32_t> (size);

}
// **********************************************************************************************************************

void instrument(TRACE trace,void * v){

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

            // ***************************************************
            // On every instruction, make a call to newInstruction
            // ***************************************************
            x86_to_static(ins);
            
            //--------------------------------------------------------------------------
            // Write the Memory
            // is_instrumenteds loads using a predicated call, i.e.
            // the call happens if the load will be actually executed
            //--------------------------------------------------------------------------
            if (INS_IsMemoryRead(ins)) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)has_read_f, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_END);
            }
            if (INS_HasMemoryRead2(ins)) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)has_read2_f, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE, IARG_END);
            }
            if (INS_IsMemoryWrite(ins)) {
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)has_write_f,  IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_END);
            }
            NewInstruction.print();
        }
    }
}


int main(int argc, char * argv[])
{

    std::stringstream pipedCommand;

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Open pipe with OrCS
    pipedCommand << "./orcs -c " << KnobConfigFile.Value() << " -p";
    pipeOut = popen(pipedCommand.str().c_str(), "w");
   

    // Register Instruction to be called to instrument instructions
    TRACE_AddInstrumentFunction(instrument, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
