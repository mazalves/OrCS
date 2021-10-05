#ifndef __ORCS_OPCODES_hpp__
#define __ORCS_OPCODES_hpp__
#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "../../../../package/opcode_package.cpp"
namespace opcodes {
    void opcode_to_trace_string(opcode_package_t &op, char *trace_string) {

        //char register_string[TRACE_LINE_SIZE];
        uint32_t reg_count;
        std::stringstream str;
        std::stringstream reg_str;

        str << op.opcode_assembly               \
            << " " << op.opcode_operation       \
            << " " << op.opcode_address         \
            << " " << op.opcode_size;
        
        /*trace_string[0] = '\0';
        sprintf(trace_string, "%s", op.opcode_assembly);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.opcode_operation);
        sprintf(trace_string, "%s %" PRId64 "", trace_string, op.opcode_address);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.opcode_size);
        */

        reg_str.str(std::string());
        //register_string[0] = '\0';
        reg_count = 0;
        for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
            if (op.read_regs[i] > 0 && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_ALU
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_DIV
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_MLA
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_MUL
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_ALU
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_DIV
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_MLA
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_MUL
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_GATHER
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_SCATTER) {
                reg_count++;
                reg_str << " " << op.read_regs[i];
                //sprintf(register_string, "%s %" PRId32 "", register_string, op.read_regs[i]);
            }
        }

        str << " " << reg_count        \
            << reg_str.str();
        /*
        sprintf(trace_string, "%s %" PRId32 "", trace_string, reg_count);
        sprintf(trace_string, "%s%s", trace_string, register_string);
        */

        reg_str.str(std::string());
        //register_string[0] = '\0';
        reg_count = 0;
        for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
            if (op.write_regs[i] > 0 && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_ALU
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_DIV
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_MLA
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_FP_MUL
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_ALU
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_DIV
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_MLA
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_INT_MUL
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_GATHER
                                    && op.opcode_operation != INSTRUCTION_OPERATION_VIMA_SCATTER) {
                reg_count++;
                reg_str << " " << op.write_regs[i];
                //sprintf(register_string, "%s %" PRId32 "", register_string, op.write_regs[i]);
            }
        }

        str << " " << reg_count         \
            << reg_str.str();

        /*
        sprintf(trace_string, "%s %" PRId32 "", trace_string, reg_count);
        sprintf(trace_string, "%s%s", trace_string, register_string);
        */

        str << " " << op.base_reg               \
            << " " << op.index_reg;
        /*
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.base_reg);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.index_reg);
        */

        reg_str.str(std::string());

        //register_string[0] = '\0';
        reg_str << " " << op.num_reads;
        reg_str << " " << op.num_writes;

        /*
        if (op.is_read == true){
            reg_str << " 1";
        }else {
            reg_str << " 0";
        }
        if (op.is_read2 == true) 
            reg_str << " 1";
        else
            reg_str << " 0";

        if (op.is_write == true)
            reg_str << " 1";
        else
            reg_str << " 0";
        */
        reg_str << " " << op.branch_type;
        //sprintf(register_string, "%s %" PRId32 "", register_string, op.branch_type);
        if (op.is_indirect == true)
            reg_str << " 1";
        else
            reg_str << " 0";


        if (op.is_predicated == true)
            reg_str << " 1";
        else
            reg_str << " 0";

        // if (op.is_prefetch == true)
        //     strcat(register_string, " 1");
        // else
        //     strcat(register_string, " 0");

        if (op.is_hive == true)
            reg_str << " 1";
        else
            reg_str << " 0";

        str << reg_str.str();
        //sprintf(trace_string, "%s%s", trace_string, register_string);


        str << " " << op.hive_read1             \
            << " " << op.hive_read2             \
            << " " << op.hive_write;             
          //  << "\n";
        /*
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.hive_read1);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.hive_read2);
        sprintf(trace_string, "%s %" PRId32 "\n", trace_string, op.hive_write);
        */
	
		if (op.is_vima == true){	
			str << " " << "1" << "\n";
		}
		else{
			str << " " << "0" << "\n";
		}
       // Prepare return
       std::string str_temp = str.str();
       strcpy(trace_string, str_temp.c_str());

    }

}
#endif
