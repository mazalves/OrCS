/**
 * Enumerations File to ORCS, SINUCA like
 *
*/

#ifndef _ORCS_TRACER_ENUMERATOR_HPP_
#define _ORCS_TRACER_ENUMERATOR_HPP_
/**
 * Enum Branch Predictor
*/
enum taken_t{
    NOT_TAKEN = 0,
    TAKEN = 1
};
/**
 * Enum  Hit Miss Situations (Cache,BTB, etc)
*/
enum cache_status_t{
    HIT,
    MISS
};
// =====================================================
/**
 * Enum cache Level
*/
enum cacheLevel_t{
    L1=0,
    L2=1,
    LLC=2,
    INST_CACHE=3
};

enum cacheId_t {
    INSTRUCTION,
    DATA
};

enum directoryStatus_t {
    CACHED,
    UNCACHED
};
// ======================================================
/// Enumerates the INSTRUCTION (Opcode and Uop) operation type
enum instruction_operation_t {
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
    /// NOT IDENTIFIED
    INSTRUCTION_OPERATION_OTHER,
    /// SYNCHRONIZATION
    INSTRUCTION_OPERATION_BARRIER,
    /// HMC
    INSTRUCTION_OPERATION_HMC_ROA,     //#12 READ+OP +Answer
    INSTRUCTION_OPERATION_HMC_ROWA,     //#13 READ+OP+WRITE +Answer

    /// HIVE
    INSTRUCTION_OPERATION_HIVE_LOCK,     //14
    INSTRUCTION_OPERATION_HIVE_UNLOCK,   //15

    INSTRUCTION_OPERATION_HIVE_LOAD,     //16
    INSTRUCTION_OPERATION_HIVE_STORE,    //17

    INSTRUCTION_OPERATION_HIVE_INT_ALU,  //18
    INSTRUCTION_OPERATION_HIVE_INT_MUL,  //19
    INSTRUCTION_OPERATION_HIVE_INT_DIV,  //20

    INSTRUCTION_OPERATION_HIVE_FP_ALU,   //21
    INSTRUCTION_OPERATION_HIVE_FP_MUL,   //22
    INSTRUCTION_OPERATION_HIVE_FP_DIV,    //23

    INSTRUCTION_OPERATION_VIMA_INT_ALU,  //24
    INSTRUCTION_OPERATION_VIMA_INT_MUL,  //25
    INSTRUCTION_OPERATION_VIMA_INT_DIV,  //26

    INSTRUCTION_OPERATION_VIMA_FP_ALU,   //27
    INSTRUCTION_OPERATION_VIMA_FP_MUL,   //28
    INSTRUCTION_OPERATION_VIMA_FP_DIV,   //29
    INSTRUCTION_OPERATION_VIMA_INT_MLA, //30
    INSTRUCTION_OPERATION_VIMA_FP_MLA,  //31
    INSTRUCTION_OPERATION_LAST
};

// ============================================================================
/// Enumerates the types of branches
enum branch_t {
    BRANCH_SYSCALL,
    BRANCH_CALL,
    BRANCH_RETURN,
    BRANCH_UNCOND,
    BRANCH_COND
};

// ============================================================================
//Packages State
enum package_state_t {
    PACKAGE_STATE_FREE,
    PACKAGE_STATE_UNTREATED,
    PACKAGE_STATE_READY,
    PACKAGE_STATE_WAIT,
    PACKAGE_STATE_TRANSMIT,
    PACKAGE_STATE_HIVE,
    PACKAGE_STATE_VIMA
};

// ============================================================================
//PROCESSOR STAGES
enum processor_stage_t {
    PROCESSOR_STAGE_FETCH,
    PROCESSOR_STAGE_DECODE,
    PROCESSOR_STAGE_RENAME,
    PROCESSOR_STAGE_DISPATCH,
    PROCESSOR_STAGE_EXECUTION,
    PROCESSOR_STAGE_COMMIT
};

// ============================================================================
//MEMORY OPERATIONS
enum memory_operation_t {
    MEMORY_OPERATION_READ,
    MEMORY_OPERATION_WRITE,
    MEMORY_OPERATION_FREE,
    MEMORY_OPERATION_INST,
    MEMORY_OPERATION_HIVE_LOCK,
    MEMORY_OPERATION_HIVE_UNLOCK,
    MEMORY_OPERATION_HIVE_LOAD,
    MEMORY_OPERATION_HIVE_STORE,
    MEMORY_OPERATION_HIVE_INT_ALU,
    MEMORY_OPERATION_HIVE_INT_MUL,
    MEMORY_OPERATION_HIVE_INT_DIV,
    MEMORY_OPERATION_HIVE_FP_ALU,
    MEMORY_OPERATION_HIVE_FP_MUL,
    MEMORY_OPERATION_HIVE_FP_DIV,
    MEMORY_OPERATION_VIMA_INT_ALU,
    MEMORY_OPERATION_VIMA_INT_MUL,
    MEMORY_OPERATION_VIMA_INT_DIV,
    MEMORY_OPERATION_VIMA_FP_ALU,
    MEMORY_OPERATION_VIMA_FP_MUL,
    MEMORY_OPERATION_VIMA_FP_DIV,
    MEMORY_OPERATION_LAST
};

// ============================================================================
// Stages of prefetcher
enum status_stride_prefetcher_t{
    INVALID,
    TRAINING,
    ACTIVE
};

// ============================================================================
/// Enumerates the types of hash function
enum hash_function_t {
    HASH_FUNCTION_XOR_SIMPLE,
    HASH_FUNCTION_INPUT1_ONLY,
    HASH_FUNCTION_INPUT2_ONLY,
    HASH_FUNCTION_INPUT1_2BITS,
    HASH_FUNCTION_INPUT1_4BITS,
    HASH_FUNCTION_INPUT1_8BITS,
    HASH_FUNCTION_INPUT1_16BITS,
    HASH_FUNCTION_INPUT1_32BITS
};

// ============================================================================
enum memory_controller_command_t {
    MEMORY_CONTROLLER_COMMAND_PRECHARGE,
    MEMORY_CONTROLLER_COMMAND_ROW_ACCESS,
    MEMORY_CONTROLLER_COMMAND_COLUMN_READ,
    MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE,
    MEMORY_CONTROLLER_COMMAND_NUMBER
};

// ============================================================================
enum request_priority_t {
    REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST,
    REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE
};

// ============================================================================
enum write_priority_t {
    WRITE_PRIORITY_DRAIN_WHEN_FULL,
    WRITE_PRIORITY_SERVICE_AT_NO_READ
};

// ============================================================================
enum disambiguation_method_t {
    DISAMBIGUATION_METHOD_HASHED,
    DISAMBIGUATION_METHOD_PERFECT
};

enum branch_prediction_method_t {
    BRANCH_PREDICTION_METHOD_TWO_BIT,
    BRANCH_PREDICTION_METHOD_PIECEWISE
};
// ============================================================================
#endif
