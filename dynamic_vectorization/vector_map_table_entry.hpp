class vector_map_table_entry_t {
    public:
        int32_t correspondent_VR;
        bool is_load;
        int32_t offset;
        uint64_t pc;
        uint64_t source_operand_1;
        uint64_t source_operand_2;
        int64_t value;

};