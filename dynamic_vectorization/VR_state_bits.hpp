class VR_state_bits_t {
    public:
        uint64_t MRBB;
        VR_entry_state_t *positions;
        vector_map_table_entry_t *associated_entry;
        uint32_t associated_not_decoded; // Instruções associadas que ainda não foram 
                                         // decodificadas
};