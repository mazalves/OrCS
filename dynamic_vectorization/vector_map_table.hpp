class vector_map_table_t {
    public:
        vector_map_table_entry_t *entries;
        int32_t entries_size;
        int32_t next_replacement;
        table_of_loads_t *TL;
        Vectorizer_t *vectorizer;
        reorder_buffer_line_t **register_alias_table;
        circular_buffer_t <uop_package_t> *inst_list;

        vector_map_table_t  (int32_t num_entries, 
                             reorder_buffer_line_t **RAT,
                             Vectorizer_t *vectorizer, 
                             table_of_loads_t *TL, 
                             circular_buffer_t <uop_package_t> *inst_list);
        ~vector_map_table_t ();
        int32_t allocate_entry ();
        bool compare_registers (uop_package_t *inst, vector_map_table_entry_t *vrmt_entry);
        DV::DV_ERROR convert_to_validation (uop_package_t *inst, vector_map_table_entry_t *vrmt_entry);

        vector_map_table_entry_t *find_pc (uint64_t pc);
        void invalidate (vector_map_table_entry_t *vrmt_entry);
        void new_store (uop_package_t *inst);

        DV::DV_ERROR validate (uop_package_t *inst, vector_map_table_entry_t *vrmt_entry);
        void fill_vectorial_part (uop_package_t *inst, char *signature, int32_t vr_id, int32_t num_part);
        DV::DV_ERROR vectorize (uop_package_t * inst, vector_map_table_entry_t **vrmt_entry, bool forward);
};