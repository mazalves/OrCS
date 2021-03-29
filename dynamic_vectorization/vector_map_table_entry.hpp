class vector_map_table_entry_t {
    public:
        int32_t correspondent_VR;
        bool is_load;
        int32_t offset;
        uint64_t pc;
        uint64_t source_operand_1;
        uint64_t source_operand_2;
        int64_t value;
        uint64_t last_use;
        bool one_register;

        vector_map_table_entry_t () {
            this->correspondent_VR = 0;
            this->is_load = false;
            this->offset = 0;
            this->pc = 0;
            this->source_operand_1 = 0;
            this->source_operand_2 = 0;
            this->value = 0;
            this->last_use = 0;
            this->one_register = false;

        }

};