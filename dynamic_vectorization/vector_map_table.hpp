class vector_map_table_t {
    public:
        vector_map_table_entry_t *entries;
        int32_t entries_size;
        int32_t next_replacement;
        table_of_loads_t *TL;
        Vectorizer_t *vectorizer;
        register_rename_table_t *register_rename_table;
        circular_buffer_t <opcode_package_t> *inst_list;

        vector_map_table_t  (int32_t num_entries, 
                             register_rename_table_t *RRT,
                             Vectorizer_t *vectorizer, 
                             table_of_loads_t *TL, 
                             circular_buffer_t <opcode_package_t> *inst_list);
        ~vector_map_table_t ();
        int32_t allocate_entry ();
        bool compare_registers (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry);
        DV::DV_ERROR convert_to_validation (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry, int32_t validation_index);

        vector_map_table_entry_t *find_pc (uint64_t pc);
        void invalidate (vector_map_table_entry_t *vrmt_entry);
        bool new_store (opcode_package_t *inst);

        DV::DV_ERROR validate (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry);
        void fill_vectorial_part (opcode_package_t *inst, bool is_load, int32_t vr_id, int32_t num_part);
        DV::DV_ERROR vectorize (opcode_package_t * inst, vector_map_table_entry_t **vrmt_entry, bool forward);

        void list_contents();
};