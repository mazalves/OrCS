#include <inttypes.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <deque>
#include <sstream>

#include "pin.H"

#include "tracer_log_procedures.hpp"
#include "../../../../defines.hpp"
#include "../../../../utils/enumerations.hpp"
#include "../../../../main_memory/memory_request_client.cpp"
#include "opcodes.hpp"
#include "conversions.cpp"
// ***************************************************************************************************************************************

KNOB<string> KnobConfigFile(KNOB_MODE_WRITEONCE, "pintool", 
                            "c", "config/sandy_bridge/sandy_bridge.cfg", "specify OrCS config file name");

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", 
                            "f", "results.res", "specify OrCS output file");
// ***************************************************************************************************************************************

#define MIN_INSTRUCTION_RESERVE 10
#define MIN_BBL_RESERVE 100
// ***************************************************************************************************************************************

class BlocksLoaded {
    class My_BBL {
        std::vector<opcode_package_t> instructions;
        uint64_t pc;

    public:
        My_BBL() {
            instructions.reserve(MIN_INSTRUCTION_RESERVE);
            pc = 0;
        }

        My_BBL(const My_BBL &other) {
            this->pc = other.pc;
            this->instructions = other.instructions;
        }

        void reset() { pc = 0; }

        opcode_package_t* next_ins() {
            if(pc == instructions.size()) {
                return NULL;
            }
            return &instructions[pc++];
        }

        void insert(opcode_package_t &op) {
            // If more space will be needed, prealloc more than that
            if(instructions.size() == instructions.capacity()) {
                instructions.reserve(instructions.capacity() + MIN_INSTRUCTION_RESERVE);
            }
            // Insert the new instruction
            instructions.push_back(op);
        }
    };
    // #####################################################################################

    std::vector<My_BBL> BBLs;

    public:
        
        BlocksLoaded() {
            BBLs.reserve(MIN_BBL_RESERVE);
        }

        BlocksLoaded(const BlocksLoaded &other) {
            std::cerr << "BlocksLoaded: Copy operator called!" << std::endl;
            exit(1);
        }

        void new_bbl() {
            // If needed reserve with fold
            if (BBLs.size() == BBLs.capacity()) {
                BBLs.reserve(BBLs.capacity() + MIN_BBL_RESERVE);
            }

            // Create the new BBL
            My_BBL temp;
            BBLs.push_back(temp);
        }

        void insert(uint64_t bbl, opcode_package_t &op) {
            BBLs[bbl].insert(op);
        }

        opcode_package_t* next_ins(uint64_t bbl) {
            return BBLs[bbl].next_ins();
        }

        void reset_bbl(uint64_t bbl) {
            BBLs[bbl].reset();
        }

};
// ***************************************************************************************************************************************

class MemoryAccess {
public:
    bool is_read;
    uint64_t addr;
    int32_t size;
    uint32_t bbl;
};
// ***************************************************************************************************************************************

BlocksLoaded program;
std::deque<MemoryAccess> memory_accesses;
uint64_t current_bbl = 0;
FILE *output;
// ***************************************************************************************************************************************

void print(opcode_package_t *op) {
    std::stringstream streamOut;
    
    streamOut   << op->opcode_assembly                          << " "
                << static_cast<int32_t> (op->opcode_operation)  << " "
                << op->opcode_address                           << " "
                << op->opcode_size                              << " ";

    for (int i = 0; i < MAX_REGISTERS; ++i) {
        streamOut << op->read_regs[i] << " " << op->write_regs[i]   << " ";
    }

    streamOut << op->base_reg << " " << op->index_reg               << " ";

    streamOut << ((op->is_read)          ? '1' : '0')           << " ";
    streamOut << op->read_address   << " " << op->read_size         << " ";

    streamOut << ((op->is_read2)         ? '1' : '0')           << " ";
    streamOut << op->read2_address  << " " << op->read2_size        << " ";

    streamOut << ((op->is_write)         ? '1' : '0')           << " ";
    streamOut << op->write_address  << " " << op->write_size        << " ";
                            
    streamOut << static_cast<int64_t> (op->branch_type)         << " "; 
    streamOut << ((op->is_indirect)      ? '1' : '0')           << " ";

    streamOut << ((op->is_predicated)    ? '1' : '0')           << " ";
    streamOut << ((op->is_prefetch)      ? '1' : '0')           << " ";

    streamOut << std::endl;

    std::string strAux = streamOut.str();
    fwrite(strAux.c_str(), strAux.size() , sizeof(char), output);

}
// ***************************************************************************************************************************************

void bbl_executed(uint32_t bbl) {
    //--------------------------------------------------------------------------
    // Print all the executed block instructions
    //--------------------------------------------------------------------------
    opcode_package_t *op = program.next_ins(bbl);
    while(op) {

        //--------------------------------------------------------------------------
        // Fill its memory access
        //--------------------------------------------------------------------------
        if(op->is_read) {
            MemoryAccess mem = memory_accesses.front();
            memory_accesses.pop_front();

            op->read_address = mem.addr;
            op->read_size = mem.size;

            //--------------------------------------------------------------------------
            // Validations
            //--------------------------------------------------------------------------
            if(mem.is_read == false) {
                std::cerr << "bbl_executed: WRITE found when READ expected!" << std::endl;
                exit(1);
            }
            if(mem.bbl != bbl) {
                std::cerr << "bbl_executed: Memory access from bbl " << mem.bbl << 
                            " found when bbl " << bbl << " expected!" << std::endl;
                exit(1);
            }
        }
        if(op->is_read2) {
            MemoryAccess mem = memory_accesses.front();
            memory_accesses.pop_front();

            op->read2_address = mem.addr;
            op->read2_size = mem.size;

            //--------------------------------------------------------------------------
            // Validations
            //--------------------------------------------------------------------------
            if(mem.is_read == false) {
                std::cerr << "bbl_executed: WRITE found when READ expected!" << std::endl;
                exit(1);
            }
            if(mem.bbl != bbl) {
                std::cerr << "bbl_executed: Memory access from bbl " << mem.bbl << 
                            " found when bbl " << bbl << " expected!" << std::endl;
                exit(1);
            }
        }

        if(op->is_write) {
            MemoryAccess mem = memory_accesses.front();
            memory_accesses.pop_front();

            op->write_address = mem.addr;
            op->write_size = mem.size;

            //--------------------------------------------------------------------------
            // Validations
            //--------------------------------------------------------------------------
            if(mem.is_read == true) {
                std::cerr << "bbl_executed: READ found when WRITE expected!" << std::endl;
                exit(1);
            }
            if(mem.bbl != bbl) {
                std::cerr << "bbl_executed: Memory access from bbl " << mem.bbl << 
                            " found when bbl " << bbl << " expected!" << std::endl;
                exit(1);
            }
        }

        //--------------------------------------------------------------------------
        // Send instruction to OrCS
        //--------------------------------------------------------------------------
        print(op);


        //--------------------------------------------------------------------------
        // Go to the next BBL instruction
        //--------------------------------------------------------------------------
        op = program.next_ins(bbl);
    }

    //--------------------------------------------------------------------------
    // Reset the BBL pc counter
    //--------------------------------------------------------------------------
    program.reset_bbl(bbl);

}
// ***************************************************************************************************************************************

VOID touch_memory(bool is_read, uint64_t addr, int32_t size, uint64_t bbl) {
    //--------------------------------------------------------------------------
    // Create a new access with the current data
    //--------------------------------------------------------------------------
    MemoryAccess memAcc;
    memAcc.is_read = is_read;
    memAcc.addr = addr;
    memAcc.size = size;
    memAcc.bbl = bbl;

    //--------------------------------------------------------------------------
    // Apend the new access into the access list
    //--------------------------------------------------------------------------
    memory_accesses.push_back(memAcc);

};
// ***************************************************************************************************************************************
/*
ADDRINT 	memoryAddress
PIN_MEMOP_ENUM 	memopType
UINT32 	bytesAccessed
BOOL 	maskOn

VOID touch_memory(bool is_read, uint64_t addr, int32_t size, uint64_t bbl) {
*/
VOID unknown_memory_size_f(PIN_MULTI_MEM_ACCESS_INFO* multi_size, uint64_t bbl) {
    uint32_t i;
    uint32_t max = multi_size->numberOfMemops;
    // Salva loads e stores
    for(i = 0; i < max; ++i) {
        PIN_MEM_ACCESS_INFO *info = &multi_size->memop[i];
        touch_memory(   (info->memopType == PIN_MEMOP_LOAD) ? true : false, // is_read
                        info->memoryAddress,                                // addr
                        static_cast<int32_t> (info->bytesAccessed),         // size
                        bbl);                                               // BBL
    }

}
// ***************************************************************************************************************************************

VOID tracer(TRACE trace, VOID *v) {

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl), ++current_bbl) {

        //--------------------------------------------------------------------------
        // New BBL
        //--------------------------------------------------------------------------
        program.new_bbl();

        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

            //--------------------------------------------------------------------------
            // New BBL instruction
            //--------------------------------------------------------------------------
            if(INS_hasKnownMemorySize(ins)) {
                
                opcode_package_t op = x86_to_static(ins);
                program.insert(current_bbl, op);

                //--------------------------------------------------------------------------
                // Memory accesses executed
                //--------------------------------------------------------------------------
                if (INS_IsMemoryRead(ins)) {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)touch_memory, IARG_BOOL, true, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_UINT64, current_bbl, IARG_END);
                }
                if (INS_HasMemoryRead2(ins)) {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)touch_memory, IARG_BOOL, true, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE, IARG_UINT64, current_bbl, IARG_END);
                }
                if (INS_IsMemoryWrite(ins)) {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)touch_memory, IARG_BOOL, false, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_UINT64, current_bbl, IARG_END);
                }
            } else {

                std::vector<opcode_package_t> *op = vgather_vscatter_to_static(ins);
                std::vector<opcode_package_t>::iterator it;

                it = op->begin();
                while(it != op->end()) {
                    program.insert(current_bbl, *it);
                    ++it;
                }
                delete(op);
                

                //--------------------------------------------------------------------------
                // Memory accesses list
                //--------------------------------------------------------------------------
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)unknown_memory_size_f, IARG_MULTI_MEMORYACCESS_EA, IARG_UINT64, current_bbl, IARG_END);
            }
            
        }

        //--------------------------------------------------------------------------
        // BBL was executed
        //--------------------------------------------------------------------------
        INS_InsertCall(BBL_InsTail(bbl), IPOINT_BEFORE, AFUNPTR(bbl_executed), IARG_UINT64, current_bbl, IARG_END);

    }

}
// ***************************************************************************************************************************************

void Fini(int rValue, void *other) {
    std::string strAux ("000\0");
    fwrite(strAux.c_str(), strAux.size() , sizeof(char), output);
    pclose(output);

    if(memory_accesses.size() > 0) {
        std::cerr << "There are remaining memory accesses in 'memory_accesses'" << std::endl;
        exit(1);
    }

    //fclose(output);
}
// ***************************************************************************************************************************************

int main(int argc, char *argv[]) {

    if (PIN_Init(argc, argv)) {
        return 1;
    }

    // Initialize symbols table
    PIN_InitSymbols();

    // Open PIPE
    std::stringstream pipedCommand;
    pipedCommand << "./orcs -c " << KnobConfigFile.Value() << " -f " << KnobOutputFile.Value() << " -p";
    std::string tempStr = pipedCommand.str();
    
    output = popen(tempStr.c_str(), "w");
    //output = fopen("output.test", "w");


    // Trace code
    TRACE_AddInstrumentFunction(tracer, 0);
        
    // Close the Files
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
