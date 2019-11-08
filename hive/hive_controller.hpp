class hive_controller_t {
    private:
        uint32_t HIVE_REGISTERS;
        cache_t **data_cache;
        
        uint32_t last_instruction;
        bool hive_lock;
        bool* hive_register_free;
        uint32_t* hive_op_latencies;
        std::vector<memory_package_t*> hive_instructions;
        hive_register_t* hive_registers;

        INSTANTIATE_GET_SET_ADD (uint32_t, HIVE_REGISTERS)
        
    public:
        hive_controller_t();
        ~hive_controller_t();
        void clock();
        void allocate();
        void addRequest (memory_package_t* request);
};