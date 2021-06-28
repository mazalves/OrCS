class vector_map_table_t {
    public:
        set_associative_t<vector_map_table_entry_t> entries;
        table_of_loads_t *TL;
        Vectorizer_t *vectorizer;
        register_rename_table_t *register_rename_table;
        circular_buffer_t <opcode_package_t> *inst_list;


        vector_map_table_t (int32_t num_entries,
                             int32_t associativity,
                             register_rename_table_t *RRT,
                             Vectorizer_t *vectorizer, 
                             table_of_loads_t *TL, 
                             circular_buffer_t <opcode_package_t> *inst_list);
        ~vector_map_table_t ();
        vector_map_table_entry_t* allocate_entry (uint64_t pc);
        bool compare_registers (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry);
        DV::DV_ERROR convert_to_validation (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry, int32_t validation_index);

        vector_map_table_entry_t *find_pc (uint64_t pc);
        void invalidate (vector_map_table_entry_t *vrmt_entry);
        bool new_store (opcode_package_t *inst);

        DV::DV_ERROR validate (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry);
        void fill_vectorial_part (opcode_package_t *inst, bool is_load, int32_t vr_id, int32_t num_part, int32_t end_vectorial_part);
        DV::DV_ERROR vectorize (opcode_package_t * inst, vector_map_table_entry_t **vrmt_entry, bool forward);

        void list_contents();

        // Check if two addresses are in the same cache line
        inline bool same_block(uint64_t addr_a, uint64_t addr_b);

        // Returns id from part that will start the next block
        inline int32_t next_block_part(int32_t current_part, uint64_t current_address, int32_t stride); 

        // Check for conflicts with vectorizations
        inline DV::DV_ERROR new_memory_operation (memory_order_buffer_line_t *mem_access);

};

inline bool vector_map_table_t::same_block(uint64_t addr_a, uint64_t addr_b) {
    // Considerando caches de 64 bytes
    uint64_t block_a = addr_a >> 6;
    uint64_t block_b = addr_b >> 6;
    if (block_a == block_b) return true;
    return false;
}

inline int32_t vector_map_table_t::next_block_part(int32_t current_part, 
                                                 uint64_t current_address, 
                                                 int32_t stride)
{
    if (stride == 0) {
        return VECTORIZATION_SIZE;
    }
    for (int32_t i = 1; i < (VECTORIZATION_SIZE - current_part); ++i) {
        if (!this->same_block(current_address, current_address + stride * i)) {
            return current_part + i;
        }
    }
    return VECTORIZATION_SIZE;

}

// Check for conflicts with vectorizations
// Retornar ENTRIES_INVALIDATED caso invalide alguma entrada
// Retornar SUCCESS caso nenhuma entrada seja invalidada
// TODO
inline DV::DV_ERROR vector_map_table_t::new_memory_operation (memory_order_buffer_line_t *mem_access) {
    //... // Preciso definir o endereço do destino para verificar também
    (void) mem_access;
    printf("ALERT: vector_map_table_t::new_memory_operation - Não implementado\n");
    return DV::SUCCESS;
}
