class hive_controller_t {
    private:
        uint32_t LINE_SIZE;
        uint32_t HIVE_REGISTERS;
        uint32_t HIVE_REGISTER_SIZE;

        uint32_t last_instruction;
        bool hive_lock;
        bool** hive_register_ready;
        std::vector<mshr_entry_t*> hive_instructions;
        std::vector<mshr_entry_t*> hive_nano_instructions;

        INSTANTIATE_GET_SET_ADD (uint32_t, LINE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t, HIVE_REGISTERS)
        INSTANTIATE_GET_SET_ADD (uint32_t, HIVE_REGISTER_SIZE)

    public:
        hive_controller_t();
        ~hive_controller_t();
        void clock();
        void allocate();
        void addRequest (mshr_entry_t* request);

};