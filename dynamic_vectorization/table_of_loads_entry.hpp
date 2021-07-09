class table_of_loads_entry_t {
    public:
        int32_t confidence;
        uint64_t last_address;
        uint64_t pc;
        int32_t stride;
        uint64_t last_use;

        table_of_loads_entry_t () {
            this->confidence = 0;
            this->last_address = 0;
            this->pc = 0;
            this->stride = -1;
            this->last_use = 0;
        }
};