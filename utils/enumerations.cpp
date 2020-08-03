#include "../simulator.hpp"

const char* get_enum_instruction_operation_char(instruction_operation_t type) {
    switch (type) {
        // ====================================================================
        /// INTEGERS
        case INSTRUCTION_OPERATION_INT_ALU:     return "OP_IN_ALU"; break;
        case INSTRUCTION_OPERATION_INT_MUL:     return "OP_IN_MUL"; break;
        case INSTRUCTION_OPERATION_INT_DIV:     return "OP_IN_DIV"; break;
        // ====================================================================
        /// FLOAT POINT
        case INSTRUCTION_OPERATION_FP_ALU:      return "OP_FP_ALU"; break;
        case INSTRUCTION_OPERATION_FP_MUL:      return "OP_FP_MUL"; break;
        case INSTRUCTION_OPERATION_FP_DIV:      return "OP_FP_DIV"; break;
        // ====================================================================
        /// BRANCHES
        case INSTRUCTION_OPERATION_BRANCH:      return "OP_BRANCH"; break;
        // ====================================================================
        /// MEMORY OPERATIONS
        case INSTRUCTION_OPERATION_MEM_LOAD:    return "OP_LOAD  "; break;
        case INSTRUCTION_OPERATION_MEM_STORE:   return "OP_STORE "; break;
        // ====================================================================
        /// NOP or NOT IDENTIFIED
        case INSTRUCTION_OPERATION_NOP:         return "OP_NOP   "; break;
        case INSTRUCTION_OPERATION_OTHER:       return "OP_OTHER "; break;
        // ====================================================================
        /// SYNCHRONIZATION
        case INSTRUCTION_OPERATION_BARRIER:     return "OP_BARRIER"; break;
        // ====================================================================
        /// HMC
        case INSTRUCTION_OPERATION_HMC_ROA:     return "HMC_ROA"; break;
        case INSTRUCTION_OPERATION_HMC_ROWA:    return "HMC_ROWA"; break;
        ///HIVE
        case INSTRUCTION_OPERATION_HIVE_FP_MUL: return "HIVE_FP_MUL"; break;
        case INSTRUCTION_OPERATION_HIVE_FP_DIV: return "HIVE_FP_DIV"; break;
        case INSTRUCTION_OPERATION_HIVE_FP_ALU: return "HIVE_FP_ALU"; break;
        case INSTRUCTION_OPERATION_HIVE_INT_ALU: return "HIVE_INT_ALU"; break;
        case INSTRUCTION_OPERATION_HIVE_INT_DIV: return "HIVE_INT_DIV"; break;
        case INSTRUCTION_OPERATION_HIVE_INT_MUL: return "HIVE_INT_MUL"; break;
        case INSTRUCTION_OPERATION_HIVE_LOAD:    return "HIVE_LOAD"; break;
        case INSTRUCTION_OPERATION_HIVE_STORE:  return "HIVE_STORE"; break;
        case INSTRUCTION_OPERATION_HIVE_LOCK: return "HIVE_LOCK"; break;
        case INSTRUCTION_OPERATION_HIVE_UNLOCK: return "HIVE_UNLOCK"; break;
        ///VIMA
        case INSTRUCTION_OPERATION_VIMA_FP_MUL: return "VIMA_FP_MUL"; break;
        case INSTRUCTION_OPERATION_VIMA_FP_DIV: return "VIMA_FP_DIV"; break;
        case INSTRUCTION_OPERATION_VIMA_FP_ALU: return "VIMA_FP_ALU"; break;
        case INSTRUCTION_OPERATION_VIMA_INT_ALU: return "VIMA_INT_ALU"; break;
        case INSTRUCTION_OPERATION_VIMA_INT_DIV: return "VIMA_INT_DIV"; break;
        case INSTRUCTION_OPERATION_VIMA_INT_MUL: return "VIMA_INT_MUL"; break;
        case INSTRUCTION_OPERATION_VIMA_INT_MLA: return "VIMA_INT_MLA"; break;
        case INSTRUCTION_OPERATION_VIMA_FP_MLA: return "VIMA_FP_MLA"; break;
        case INSTRUCTION_OPERATION_LAST: return "LAST"; break;
        
    }
    ERROR_PRINTF("Wrong INSTRUCTION_OPERATION\n");
    return "FAIL";
}
// ============================================================================
/// Enumerates the processor stages, used to indicate when the branch will be solved
const char *get_enum_processor_stage_char(processor_stage_t type) {
    switch (type) {
        case PROCESSOR_STAGE_FETCH:     return "FETCH    "; break;
        case PROCESSOR_STAGE_DECODE:    return "DECODE   "; break;
        case PROCESSOR_STAGE_RENAME:    return "RENAME   "; break;
        case PROCESSOR_STAGE_DISPATCH:  return "DISPATCH "; break;
        case PROCESSOR_STAGE_EXECUTION: return "EXECUTION"; break;
        case PROCESSOR_STAGE_COMMIT:    return "COMMIT   "; break;
    }
    ERROR_PRINTF("Wrong PROCESSOR_STAGE\n");
    return "FAIL";
}
// ============================================================================
/// Enumerates the MEMORY OPERATION OF MOB,debug only
const char *get_enum_memory_operation_char(memory_operation_t type) {
    switch (type) {
        case MEMORY_OPERATION_READ:     return "READ"; break;
        case MEMORY_OPERATION_WRITE:    return "WRITE"; break;
        case MEMORY_OPERATION_FREE:    return "FREE"; break;
        case MEMORY_OPERATION_INST:     return "INST"; break;
        case MEMORY_OPERATION_HIVE_UNLOCK:  return "HIVE_UNLOCK"; break;
        case MEMORY_OPERATION_HIVE_LOCK:    return "HIVE_LOCK"; break;
        case MEMORY_OPERATION_HIVE_LOAD:    return "HIVE_LOAD"; break;
        case MEMORY_OPERATION_HIVE_STORE:   return "HIVE_STORE"; break;
        case MEMORY_OPERATION_HIVE_INT_ALU: return "HIVE_INT_ALU"; break;
        case MEMORY_OPERATION_HIVE_INT_DIV: return "HIVE_INT_DIV"; break;
        case MEMORY_OPERATION_HIVE_INT_MUL: return "HIVE_INT_MUL"; break;
        case MEMORY_OPERATION_HIVE_FP_ALU:  return "HIVE_FP_ALU"; break;
        case MEMORY_OPERATION_HIVE_FP_DIV:  return "HIVE_FP_DIV"; break;
        case MEMORY_OPERATION_HIVE_FP_MUL:  return "HIVE_FP_MUL"; break;
        case MEMORY_OPERATION_VIMA_INT_ALU: return "VIMA_INT_ALU"; break;
        case MEMORY_OPERATION_VIMA_INT_DIV: return "VIMA_INT_DIV"; break;
        case MEMORY_OPERATION_VIMA_INT_MUL: return "VIMA_INT_MUL"; break;
        case MEMORY_OPERATION_VIMA_FP_ALU:  return "VIMA_FP_ALU"; break;
        case MEMORY_OPERATION_VIMA_FP_DIV:  return "VIMA_FP_DIV"; break;
        case MEMORY_OPERATION_VIMA_FP_MUL:  return "VIMA_FP_MUL"; break;
        case MEMORY_OPERATION_VIMA_INT_MLA: return "VIMA_INT_MLA"; break;
        case MEMORY_OPERATION_VIMA_FP_MLA:  return "VIMA_FP_MLA"; break;
        case MEMORY_OPERATION_LAST:         return "LAST"; break;
    }
    ERROR_PRINTF("Wrong MEMORY_OPERATION\n");
    return "FAIL";
}
// ============================================================================
/// Enumerates the package state, 
const char *get_enum_package_state_char(package_state_t type) {
    switch (type) {
        case PACKAGE_STATE_FREE:     return "FREE"; break;
        case PACKAGE_STATE_READY:     return "READY"; break;
        case PACKAGE_STATE_TRANSMIT:     return "TRANSMIT"; break;
        case PACKAGE_STATE_UNTREATED:     return "UNTREATED"; break;
        case PACKAGE_STATE_WAIT:     return "WAIT"; break;
        case PACKAGE_STATE_HIVE:    return "HIVE"; break;
        case PACKAGE_STATE_VIMA:    return "VIMA"; break;
        case PACKAGE_STATE_DRAM_FETCH:  return "DRAM_FETCH"; break;
        case PACKAGE_STATE_DRAM_READY:  return "DRAM_READY"; break;
    }
    ERROR_PRINTF("Wrong PACKAGE_STATE\n");
    return "FAIL";
}
// ============================================================================
/// Enumerates the cache level, 
const char *get_enum_cache_level_char(cacheLevel_t type) {
    switch (type) {
        case  INST_CACHE:     return "INST_CACHE"; break;
        case L1:     return "L1_DATA_CACHE"; break;
        case L2:     return "L2_UNIFIED_CACHE"; break;
        case LLC:     return "LLC"; break;
    }
    ERROR_PRINTF("Wrong CACHE_LEVEL\n");
    return "FAIL";
} 
const char *get_enum_status_stride_prefetcher_char(status_stride_prefetcher_t type) {
    switch (type) {
        case  INVALID:     return "INVALID"; break;
        case TRAINING:     return "TRAINING"; break;
        case ACTIVE:     return "ACTIVE"; break;
    }
    ERROR_PRINTF("Wrong Status Prefetcher STRIDE\n");
    return "FAIL";
}  
// ============================================================================
/// Enumerates the types of hash function
const char *get_enum_hash_function_char(hash_function_t type)  {
    switch (type) {
        case HASH_FUNCTION_XOR_SIMPLE:  return "HASH_FUNCTION_XOR_SIMPLE"; break;
        case HASH_FUNCTION_INPUT1_ONLY: return "HASH_FUNCTION_INPUT1_ONLY"; break;
        case HASH_FUNCTION_INPUT2_ONLY: return "HASH_FUNCTION_INPUT2_ONLY"; break;
        case HASH_FUNCTION_INPUT1_2BITS: return "HASH_FUNCTION_INPUT1_2BITS"; break;
        case HASH_FUNCTION_INPUT1_4BITS: return "HASH_FUNCTION_INPUT1_4BITS"; break;
        case HASH_FUNCTION_INPUT1_8BITS: return "HASH_FUNCTION_INPUT1_8BITS"; break;
        case HASH_FUNCTION_INPUT1_16BITS: return "HASH_FUNCTION_INPUT1_16BITS"; break;
        case HASH_FUNCTION_INPUT1_32BITS: return "HASH_FUNCTION_INPUT1_32BITS"; break;
    }
    ERROR_PRINTF("Wrong HASH_FUNCTION\n");
    return "FAIL";
}

const char *get_enum_request_priority_char (request_priority_t type){
    switch (type) {
        case REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE:  return "FIRST_COME_FIRST_SERVE"; break;
        case REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST: return "ROW_BUFFER_HITS_FIRST"; break;
    }
    ERROR_PRINTF("Wrong REQUEST_PRIORITY\n");
    return "FAIL";
}

const char *get_enum_memory_controller_command_char(memory_controller_command_t type){
    switch (type){
        case MEMORY_CONTROLLER_COMMAND_NUMBER: return "NUMBER"; break;
        case MEMORY_CONTROLLER_COMMAND_PRECHARGE: return "PRECHARGE"; break;
        case MEMORY_CONTROLLER_COMMAND_ROW_ACCESS: return "ROW_ACCESS"; break;
        case MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE: return "COLUMN_WRITE"; break;
        case MEMORY_CONTROLLER_COMMAND_COLUMN_READ: return "COLUMN_READ"; break;
    }
    return "FAIL";
}