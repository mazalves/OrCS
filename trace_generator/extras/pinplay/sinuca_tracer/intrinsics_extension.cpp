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
///  intrinsics_extension.cpp
///###########################################################################
///  This file contains the bodies of all intrinsics extension functions.
///###########################################################################
#include "intrinsics_extension.hpp"
#include <string>

//============================================================================
// Intrinsics Functions
//============================================================================

VOID write_dynamic_char(char *dyn_str, THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("write_dynamic_char()\n");
    /// If the pin-points disabled this region
    if (!is_instrumented) {
        return;
    } else {
        // This lock is necessary because when using a parallel program
        // the thread master may write on multiple threads
        // ex: omp_parallel_start / omp_parallel_end
        PIN_GetLock(&thread_data[threadid].dyn_lock, threadid);
        gzwrite(thread_data[threadid].gzDynamicTraceFile,
                dyn_str, strlen(dyn_str));
        PIN_ReleaseLock(&thread_data[threadid].dyn_lock);
    }
}

// ===========================================================================

VOID write_static_char(char *stat_str) {
    TRACE_GENERATOR_DEBUG_PRINTF("write_static_char()\n");
    PIN_GetLock(&lock, 1);
        gzwrite(gzStaticTraceFile, stat_str, strlen(stat_str));
    PIN_ReleaseLock(&lock);
}

// ===========================================================================

VOID hmc_write_memory_1param(ADDRINT read, UINT32 size, UINT32 bbl,
                                THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("hmc_write_memory()\n");

    if (thread_data[threadid].is_instrumented_bbl == false)
        return;     // If the pin-points disabled this region

    char mem_str[TRACE_LINE_SIZE];

    snprintf(mem_str, TRACE_LINE_SIZE,
            "%c %d %" PRIu64 " %d\n", 'R', size, (uint64_t)read, bbl);
    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));

    snprintf(mem_str, TRACE_LINE_SIZE,
            "%c %d %" PRIu64 " %d\n", 'W', size, (uint64_t)read, bbl);
    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));
}

VOID hmc_write_memory_2param(ADDRINT read, ADDRINT write, UINT32 size,
                                UINT32 bbl, THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("hmc_write_memory()\n");

    if (thread_data[threadid].is_instrumented_bbl == false)
        return;     // If the pin-points disabled this region

    char mem_str[TRACE_LINE_SIZE];

    snprintf(mem_str, TRACE_LINE_SIZE,
            "%c %d %" PRIu64 " %d\n", 'R', size, (uint64_t)read, bbl);
    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));

    snprintf(mem_str, TRACE_LINE_SIZE,
            "%c %d %" PRIu64 " %d\n", 'W', size, (uint64_t)write, bbl);
    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));
}

VOID hmc_write_memory_3param(ADDRINT read1, ADDRINT read2, ADDRINT write,
                                UINT32 size, UINT32 bbl, THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("hmc_write_memory_3param()\n");

    if (thread_data[threadid].is_instrumented_bbl == false)
        return;     // If the pin-points disabled this region

    char mem_str[TRACE_LINE_SIZE];

    snprintf(mem_str, TRACE_LINE_SIZE,
            "%c %d %" PRIu64 " %d\n", 'R', size, (uint64_t)read1, bbl);
    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));

    snprintf(mem_str, TRACE_LINE_SIZE,
            "%c %d %" PRIu64 " %d\n", 'R', size, (uint64_t)read2, bbl);
    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));

    snprintf(mem_str, TRACE_LINE_SIZE,
            "%c %d %" PRIu64 " %d\n", 'W', size, (uint64_t)write, bbl);
    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));
}

// ===========================================================================

VOID arch_x86_set_data_instr(data_instr *arch_x86_data, char const *rtn_name,
                                char const *hmc_instr_name,
                                char const *x86_instr_name,
                                UINT32 instr_len) {
    arch_x86_data->rtn_name = rtn_name;
    arch_x86_data->arch_instr_name = hmc_instr_name;
    arch_x86_data->x86_instr_name = x86_instr_name;
    arch_x86_data->instr_len = instr_len;
}

// ===========================================================================

VOID initialize_intrinsics_hmc(data_instr hmc_x86_data[HMC_INS_COUNT]) {
    for (int i = 0; i < HMC64_INS_COUNT; i++) {
        arch_x86_set_data_instr(&hmc_x86_data[i],
                                hmc64_inst_names[i].c_str(),
                                hmc64_mnem_names[i].c_str(),
                                hmc64_x86mnem_names[i].c_str(),
                                HMC64_INS_SIZE);
    }
    for (int i = 0; i < HMC128_INS_COUNT; i++) {
        arch_x86_set_data_instr(&hmc_x86_data[i],
                                hmc128_inst_names[i].c_str(),
                                hmc128_mnem_names[i].c_str(),
                                hmc128_x86mnem_names[i].c_str(),
                                HMC128_INS_SIZE);
    }
}

// ===========================================================================

VOID initialize_intrinsics_vima(data_instr vim_x86_data[VIMA_INS_COUNT]) {
    for (int i = 0; i < VIMA_INS_COUNT; i++) {
        uint32_t instr_len = (i % 2 == 0 ? 256 : 8192);
        arch_x86_set_data_instr(&vim_x86_data[i],
                                vima_inst_names[i].c_str(),
                                vima_mnem_names[i].c_str(),
                                vima_x86mnem_names[i].c_str(),
                                instr_len);
    }
}

// ===========================================================================

VOID initialize_intrinsics_mips(data_instr mps_x86_data[MPS_INS_COUNT]) {
    for (int i = 0; i < MPS_INS_COUNT; i++) {
        arch_x86_set_data_instr(&mps_x86_data[i],
                                mps_inst_names[i].c_str(),
                                mps_mnem_names[i].c_str(),
                                mps_x86mnem_names[i].c_str(),
                                MPS_INS_SIZE);
    }
}

// ===========================================================================

VOID initialize_intrinsics(data_instr hmc_x86_data[HMC_INS_COUNT],
                            data_instr vim_x86_data[VIMA_INS_COUNT],
                            data_instr mps_x86_data[MPS_INS_COUNT]) {
    // HMC instructions
    initialize_intrinsics_hmc(hmc_x86_data);
    // VIMA instructions
    initialize_intrinsics_vima(vim_x86_data);
    // MIPS instructions
    initialize_intrinsics_mips(mps_x86_data);
}



// ===========================================================================

INT icheck_conditions_hmc(std::string rtn_name) {
    for (int i = 0; i < HMC_INS_COUNT; i++) {
        if (rtn_name.compare(4,
                            hmc_inst_names[i].size(),
                            hmc_inst_names[i].c_str()) == 0)
            return 1;
    }
    return 0;
}

// ===========================================================================

INT icheck_conditions_vima(std::string rtn_name) {
    for (int i = 0; i < VIMA_INS_COUNT; i++) {
        if (rtn_name.compare(4,
                            vima_inst_names[i].size(),
                            vima_inst_names[i].c_str()) == 0)
            return 1;
    }
    return 0;
}

// ===========================================================================

INT icheck_conditions_mips(std::string rtn_name) {
    for (int i = 0; i < MPS_INS_COUNT; i++) {
        if (rtn_name.compare(4,
                            mps_inst_names[i].size(),
                            mps_inst_names[i].c_str()) == 0)
            return 1;
    }
    return 0;
}

// ===========================================================================

INT icheck_conditions(std::string rtn_name) {
    if ((rtn_name.size() > 4)
    && (icheck_conditions_hmc(rtn_name)
    || icheck_conditions_vima(rtn_name)
    || icheck_conditions_mips(rtn_name))) {
        return 1;
    }
    return 0;
}

// ===========================================================================

#define CMP_2PARAM_COUNT 32

static const int cmp_2param_idx[CMP_2PARAM_COUNT] = { 8, 9, 14, 15, 16,
                                                    17, 24, 25, 56, 57,
                                                    58, 59, 68, 69, 74,
                                                    75, 84, 85, 92, 93,
                                                    98, 99, 108, 109, 128, 
                                                    129, 130, 131, 132, 
                                                    133, 134, 135
                                                    };

INT icheck_2parameters(std::string rtn_name) {
    for (int i = 0; i < CMP_2PARAM_COUNT; i++) {
        if (rtn_name.compare(4, vima_inst_names[cmp_2param_idx[i]].size(),
                            vima_inst_names[cmp_2param_idx[i]].c_str()) == 0)
            return 1;
    }
    return 0;
}
// ===========================================================================

#define CMP_1PARAM_COUNT 12
static const int cmp_1param_idx[CMP_1PARAM_COUNT] = { 60, 61, 62, 63,
                                                    86, 87, 110, 111,
                                                    164, 165, 166, 167
                                                    };

INT icheck_1parameter(std::string rtn_name) {
    for (int i = 0; i < CMP_1PARAM_COUNT; i++) {
        if (rtn_name.compare(4, vima_inst_names[cmp_1param_idx[i]].size(),
                             vima_inst_names[cmp_1param_idx[i]].c_str()) == 0)
                                 return 1;
    }
    return 0;
}

VOID arch_x86_trace_instruction(RTN arch_rtn, data_instr archx_x86_data) {
    opcode_package_t NewInstruction;
    char bbl_count_str[TRACE_LINE_SIZE];
    char opcode_str[TRACE_LINE_SIZE];
    uint32_t i, j, bicount;
    int ireg, breg, rreg, wreg;
    std::string rtn_name;
    bool read_regs[MAX_REGISTER_NUMBER], write_regs[MAX_REGISTER_NUMBER];
    // Open HMC routine
    if (RTN_Valid(arch_rtn)) {
        RTN_Open(arch_rtn);
        rtn_name = RTN_Name(arch_rtn);
        snprintf(bbl_count_str, TRACE_LINE_SIZE, "#%s\n", archx_x86_data.rtn_name);
        write_static_char(bbl_count_str);
        count_trace++;
        snprintf(bbl_count_str, TRACE_LINE_SIZE, "@%u\n", count_trace);
        write_static_char(bbl_count_str);
        bicount = 0;
        char *hmc_bbl_str = new char[32];
        snprintf(hmc_bbl_str, TRACE_LINE_SIZE, "%u\n", count_trace);

        RTN_InsertCall(arch_rtn, IPOINT_BEFORE,
                        (AFUNPTR)write_dynamic_char,
                        IARG_PTR, hmc_bbl_str,
                        IARG_THREAD_ID,
                        IARG_END);

        if (icheck_1parameter(rtn_name) == 1) {
            RTN_InsertCall(arch_rtn, IPOINT_BEFORE, (AFUNPTR)hmc_write_memory_1param, 
                            IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                            IARG_UINT32, archx_x86_data.instr_len, 
                            IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
        } else if (icheck_2parameters(rtn_name) == 1) {
            RTN_InsertCall(arch_rtn, IPOINT_BEFORE,
                            (AFUNPTR)hmc_write_memory_2param,
                            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                            IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                            IARG_UINT32, archx_x86_data.instr_len,
                            IARG_UINT32, count_trace,
                            IARG_THREAD_ID,
                            IARG_END);
        } else {
            RTN_InsertCall(arch_rtn, IPOINT_BEFORE,
                            (AFUNPTR)hmc_write_memory_3param,
                            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                            IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
                            IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
                            IARG_UINT32, archx_x86_data.instr_len,
                            IARG_UINT32, count_trace,
                            IARG_THREAD_ID,
                            IARG_END);
        }

        for (i = 0; i < MAX_REGISTER_NUMBER; i++) {
            read_regs[i] = false;
            write_regs[i] = false;
        }

        // Identify read/write registers
        for (INS ins = RTN_InsHead(arch_rtn);
                INS_Valid(ins); ins = INS_Next(ins)) {
            // record all read registers
            for (i = 0; i < INS_MaxNumRRegs(ins); i++) {
                rreg = INS_RegR(ins, i);
                if (rreg > 0
                && read_regs[rreg] == false
                && write_regs[rreg] == false) {
                    read_regs[rreg] = true;
                }
            }
            // record all write registers
            for (j = 0; j < INS_MaxNumWRegs(ins); j++) {
                wreg = INS_RegW(ins, j);
                if (wreg > 0 && write_regs[wreg] == false) {
                    write_regs[wreg] = true;
                }
            }

            breg = INS_MemoryBaseReg(ins);
            ireg = INS_MemoryIndexReg(ins);

            // Record base register if it is not into read and write register's lists
            if (breg > 0
            && read_regs[breg] == false
            && write_regs[breg] == false) {
                read_regs[breg] = true;
                bicount++;
            }
            // Record index register if it is not into read and write register's lists
            if (ireg > 0
            && read_regs[ireg] == false
            && write_regs[ireg] == false) {
                read_regs[ireg] = true;
                bicount++;
            }
        }

        // Record all read and write registers into read and write register's list
        int r_index = 0, w_index = 0;
        for (i = 0; i < MAX_REGISTER_NUMBER; i++) {
            if (read_regs[i] == true) {
                NewInstruction.read_regs[r_index++] = i;
            }
            if (write_regs[i] == true) {
                NewInstruction.write_regs[w_index++] = i;
            }
        }

        // Record opcode address
        NewInstruction.opcode_address = RTN_Address(arch_rtn);

        // Selects between x86, HMC and VIMA simulation
        if (KnobTrace.Value().compare(0, 4, "ix86") == 0) {
            snprintf(NewInstruction.opcode_assembly,
                    TRACE_LINE_SIZE,
                    "%s",
                    archx_x86_data.x86_instr_name);
        } else {
            snprintf(NewInstruction.opcode_assembly,
                    TRACE_LINE_SIZE,
                    "%s",
                    archx_x86_data.arch_instr_name);
        }

        // Record opcode size
        NewInstruction.opcode_size = archx_x86_data.instr_len;

        for (int i = 0; i < VIMA_INT_ALU_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_int_alu_names[i].size(),
                                vima_int_alu_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_INT_ALU;
            }
        }
        for (int i = 0; i < VIMA_FP_ALU_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_fp_alu_names[i].size(),
                                vima_fp_alu_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_FP_ALU;
            }
        }
        for (int i = 0; i < VIMA_INT_MUL_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_int_mul_names[i].size(),
                                vima_int_mul_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_INT_MUL;
            }
        }
        for (int i = 0; i < VIMA_FP_MUL_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_fp_mul_names[i].size(),
                                vima_fp_mul_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_FP_MUL;
            }
        }
        for (int i = 0; i < VIMA_INT_DIV_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_int_div_names[i].size(),
                                vima_int_div_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_INT_DIV;
            }
        }
        for (int i = 0; i < VIMA_FP_DIV_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_fp_div_names[i].size(),
                                vima_fp_div_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_FP_DIV;
            }
        }
        for (int i = 0; i < VIMA_INT_MLA_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_int_mla_names[i].size(),
                                vima_int_mla_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_INT_MLA;
            }
        }
        for (int i = 0; i < VIMA_FP_MLA_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_fp_mla_names[i].size(),
                                vima_fp_mla_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_FP_MLA;
            }
        }
        for (int i = 0; i < VIMA_GATHER_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_gather_names[i].size(),
                                vima_gather_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_GATHER;
            }
        }
        for (int i = 0; i < VIMA_SCATTER_COUNT; i++) {
            if (rtn_name.compare(4,
                                vima_scatter_names[i].size(),
                                vima_scatter_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_VIMA_SCATTER;
            }
        }
        //  revisar esses 2 laços. antes o default era pra ser
        //  HMC_ROWA em um else.
        for (int i = 0; i < HMC_INS_COUNT; i++) {
            if (rtn_name.compare(4,
                                hmc_inst_names[i].size(),
                                hmc_inst_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_HMC_ROWA;
            }
        }
        for (int i = 0; i < HMC_ROA_COUNT; i++) {
            if (rtn_name.compare(4,
                                hmc_roa_names[i].size(),
                                hmc_roa_names[i].c_str()) == 0) {
                NewInstruction.opcode_operation = INSTRUCTION_OPERATION_HMC_ROA;
            }
        }

        if (icheck_2parameters(rtn_name) == 1
        || icheck_1parameter(rtn_name)) {
            NewInstruction.num_reads = 1;
            NewInstruction.num_writes = 1;
            /*NewInstruction.is_read = 1;
            NewInstruction.is_read2 = 0;
            NewInstruction.is_write = 1;*/
        } else {
            NewInstruction.num_reads = 2;
            NewInstruction.num_writes = 1;
            /*NewInstruction.is_read2 = 1;
            NewInstruction.is_read = 1;
            NewInstruction.is_write = 1;*/
        }

        NewInstruction.is_indirect = 0;
        NewInstruction.is_predicated = 0;
        // NewInstruction.is_prefetch = 0;
        opcodes::opcode_to_trace_string(NewInstruction, opcode_str);
        write_static_char(opcode_str);
        RTN_Close(arch_rtn);
    }
}

// ===========================================================================

VOID specific_trace_generation(std::string rtn_name,
                                const char *arch_name,
                                int n_instr,
                                data_instr *arch_x86_data,
                                RTN rtn) {
    if (KnobTrace.Value().compare(0, 4, arch_name) == 0
    || KnobTrace.Value().compare(0, 4, "ix86") == 0) {
        for (int n = 0; n < n_instr; n++) {
            std::string arch_x86_cmp_name(arch_x86_data[n].rtn_name);
            if ((4 < rtn_name.size())
                && (4 < arch_x86_cmp_name.size())
                && rtn_name.compare(4, arch_x86_cmp_name.size(),
                                    arch_x86_cmp_name.c_str()) == 0) {
                arch_x86_trace_instruction(rtn, arch_x86_data[n]);
            }
        }
    } else {
        ERROR_PRINTF(
"Cannot execute %s instructions on different architecture!\n",
                    arch_name);
        exit(1);
    }
}

// ===========================================================================

VOID synthetic_trace_generation(std::string rtn_name,
                                data_instr hmc_x86_data[HMC_INS_COUNT],
                                data_instr vim_x86_data[VIMA_INS_COUNT],
                                data_instr mps_x86_data[MPS_INS_COUNT],
                                RTN rtn) {
    //  If the instruction name contains "_hmc",
    //  it can be only an HMC or x86 instruction
    if (rtn_name.size() > 4) {
        if (rtn_name.compare(4, 4, "_hmc") == 0) {
            specific_trace_generation(rtn_name,
                                    "iHMC",
                                    HMC_INS_COUNT,
                                    hmc_x86_data,
                                    rtn);
        } else if (rtn_name.compare(4, 4, "_vim") == 0) {
            specific_trace_generation(rtn_name,
                                    "iVIM",
                                    VIMA_INS_COUNT,
                                    vim_x86_data,
                                    rtn);
        } else if (rtn_name.compare(4, 4, "_mps") == 0) {
            specific_trace_generation(rtn_name,
                                    "iMPS",
                                    MPS_INS_COUNT,
                                    mps_x86_data,
                                    rtn);
        }
    }
}
