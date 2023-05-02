#ifndef OPCODE_PACKAGE_H
#define OPCODE_PACKAGE_H
#include <vector>
// ============================================================================
// ============================================================================
class opcode_package_t : public memory_request_client_t {
    public:
        /// TRACE Variables
        char opcode_assembly[TRACE_LINE_SIZE];
        instruction_operation_t opcode_operation;

        uint32_t instruction_id;
        uint64_t opcode_address;
        uint32_t opcode_size;

        int32_t read_regs[MAX_REGISTERS];
        int32_t write_regs[MAX_REGISTERS];

        uint32_t base_reg;
        uint32_t index_reg;


        uint64_t reads_addr[MAX_MEM_OPERATIONS];
        uint32_t reads_size[MAX_MEM_OPERATIONS];
        uint32_t num_reads;
        uint64_t writes_addr[MAX_MEM_OPERATIONS];
        uint32_t writes_size[MAX_MEM_OPERATIONS];
        uint32_t num_writes;

        branch_t branch_type;
        bool is_indirect;

        bool is_predicated;
        bool is_prefetch;

        bool is_hive;
        int32_t hive_read1 = -1;
        int32_t hive_read2 = -1;
        int32_t hive_write = -1;

        bool is_vima;

        trace_checkpoint_t checkpoint;
   

        // ====================================================================
        /// Status Control
        // ====================================================================
        uint64_t opcode_number;
        // ====================================================================
        /// Methods
        // ====================================================================
        
        opcode_package_t();
        ~opcode_package_t();


        #ifndef __PIN__
        void package_clean();
        std::string content_to_string();
        std::string content_to_string2();
        #endif
       
};
#endif // OPCODE_PACKAGE_H
