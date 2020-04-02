#ifndef __ORCS_TRACER_OPCODE_PACKAGE_hpp_
#define __ORCS_TRACER_OPCODE_PACKAGE_hpp_
// ============================================================================
// ============================================================================
class opcode_package_t : public memory_request_client_t {
    public:
        /// TRACE Variables
        char opcode_assembly[TRACE_LINE_SIZE];
        instruction_operation_t opcode_operation;
        uint64_t opcode_address;
        uint32_t opcode_size;

        int32_t read_regs[16];
        int32_t write_regs[16];

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

        bool is_hive;
        int32_t hive_read1;
        int32_t hive_read2;
        int32_t hive_write;

        bool is_vima;

        // ====================================================================
        /// Status Control
        // ====================================================================
        uint64_t opcode_number;
        // ====================================================================
        /// Methods
        // ====================================================================
        opcode_package_t(){
            /// TRACE Variables
            sprintf(this->opcode_assembly, "N/A");
            this->opcode_operation = INSTRUCTION_OPERATION_NOP;
            this->opcode_address = 0;
            this->opcode_size = 0;

            for (uint32_t i=0; i < 16; i++){
                read_regs[i] = 0;
                write_regs[i] = 0;
            }
            
            this->base_reg = 0;
            this->index_reg = 0;

            this->is_read = false;
            this->read_address = 0;
            this->read_size = 0;

            this->is_read2 = false;
            this->read2_address = 0;
            this->read2_size = 0;

            this->is_write = false;
            this->write_address = 0;
            this->write_size = 0;

            this->branch_type = BRANCH_UNCOND;
            this->is_indirect = false;

            this->is_predicated = false;
            this->is_prefetch = false;

            this->status = PACKAGE_STATE_FREE;
            this->readyAt = 0;
        }
        ~opcode_package_t() {}
       
};
#endif