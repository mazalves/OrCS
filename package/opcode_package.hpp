// ============================================================================
// ============================================================================
class opcode_package_t {
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

        // ====================================================================
        /// Status Control
        // ====================================================================
        package_state_t status;
        uint64_t readyAt;
        uint64_t opcode_number;
        // ====================================================================
        /// Methods
        // ====================================================================
        opcode_package_t();
        void package_clean();
        void updatePackageUntrated(uint32_t stallTime);
        void updatePackageReady(uint32_t stallTime);
        void updatePackageWait(uint32_t stallTime);
        void updatePackageFree(uint32_t stallTime);
        std::string content_to_string();
        std::string content_to_string2();
       
};
