class memory_package_t {
    public:
        uint32_t processor_id;                  /// if (read / write) PROCESSOR.ID   else if (write-back / prefetch) CACHE_MEMORY.ID
        uint64_t opcode_number;                 /// initial opcode number
        uint64_t opcode_address;                /// initial opcode address
        uint64_t uop_number;                    /// initial uop number (Instruction == 0)
        uint64_t memory_address;                /// memory address
        uint32_t memory_size;                   /// operation size after offset

        package_state_t status;                  /// package state
        uint64_t readyAt;                   /// package latency
        uint64_t born_cycle;                    /// package create time
        
        bool sent_to_cache;
        bool sent_to_ram;
        bool is_hive;
        int64_t hive_read1;
        int64_t hive_read2;
        int64_t hive_write;

        bool is_vima;
        uint64_t vima_read1;
        uint64_t vima_read2;
        uint64_t vima_write;

        bool row_buffer;
        cacheId_t type;
        uint64_t* op_count;
        uint32_t latency;

        memory_operation_t memory_operation;    /// memory operation
        std::vector<memory_request_client_t*> clients; ///update these

        uint32_t DEBUG;
        uint32_t MSHR_DEBUG;

        memory_package_t();
        ~memory_package_t();

        void updatePackageUntreated(uint32_t stallTime);
        void updatePackageReady();
        void updatePackageWait(uint32_t stallTime);
        void updatePackageFree(uint32_t stallTime);
        void updatePackageHive(uint32_t stallTime);
        void updatePackageVima(uint32_t stallTime);
        void updatePackageTransmit(uint32_t stallTime);
        void updatePackageDRAMFetch(uint32_t stallTime);
        void updatePackageDRAMReady(uint32_t stallTime);
        void updateClients();
        void printPackage();

        INSTANTIATE_GET_SET_ADD (uint32_t, DEBUG)
        INSTANTIATE_GET_SET_ADD (uint32_t, MSHR_DEBUG)
};