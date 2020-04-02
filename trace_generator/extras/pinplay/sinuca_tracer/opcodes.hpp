#ifndef __ORCS_OPCODES_hpp__
#define __ORCS_OPCODES_hpp__
#include <stdint.h>
#include <stdio.h>
#include "opcode_package.hpp"
namespace opcodes {
    void opcode_to_trace_string(opcode_package_t &op, char *trace_string) {
        char register_string[TRACE_LINE_SIZE];
        uint32_t reg_count;

        trace_string[0] = '\0';
        sprintf(trace_string, "%s", op.opcode_assembly);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.opcode_operation);
        sprintf(trace_string, "%s %" PRId64 "", trace_string, op.opcode_address);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.opcode_size);

        register_string[0] = '\0';
        reg_count = 0;
        for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
            if (op.read_regs[i] >= 0) {
                reg_count++;
                sprintf(register_string, "%s %" PRId32 "", register_string, op.read_regs[i]);
            }
        }
        sprintf(trace_string, "%s %" PRId32 "", trace_string, reg_count);
        sprintf(trace_string, "%s%s", trace_string, register_string);


        register_string[0] = '\0';
        reg_count = 0;
        for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
            if (op.write_regs[i] >= 0) {
                reg_count++;
                sprintf(register_string, "%s %" PRId32 "", register_string, op.write_regs[i]);
            }
        }
        sprintf(trace_string, "%s %" PRId32 "", trace_string, reg_count);
        sprintf(trace_string, "%s%s", trace_string, register_string);


        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.base_reg);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.index_reg);


        register_string[0] = '\0';
        if (op.is_read == true)
            strcat(register_string, " 1");
        else
            strcat(register_string, " 0");

        if (op.is_read2 == true)
            strcat(register_string, " 1");
        else
            strcat(register_string, " 0");

        if (op.is_write == true)
            strcat(register_string, " 1");
        else
            strcat(register_string, " 0");

        sprintf(register_string, "%s %" PRId32 "", register_string, op.branch_type);
        if (op.is_indirect == true)
            strcat(register_string, " 1");
        else
            strcat(register_string, " 0");


        if (op.is_predicated == true)
            strcat(register_string, " 1");
        else
            strcat(register_string, " 0");

        // if (op.is_prefetch == true)
        //     strcat(register_string, " 1");
        // else
        //     strcat(register_string, " 0");

        if (op.is_hive == true)
            strcat(register_string, " 1");
        else
            strcat(register_string, " 0");

        sprintf(trace_string, "%s%s", trace_string, register_string);

        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.hive_read1);
        sprintf(trace_string, "%s %" PRId32 "", trace_string, op.hive_read2);
        sprintf(trace_string, "%s %" PRId32 "\n", trace_string, op.hive_write);
    }

}
#endif