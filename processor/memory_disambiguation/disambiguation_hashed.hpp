
class disambiguation_hashed_t
{
public:
    disambiguation_hashed_t(/* args */);
    ~disambiguation_hashed_t();
    void allocate();
    void make_memory_dependencies(memory_order_buffer_line_t *mob_line);
    void solve_memory_dependencies(memory_order_buffer_line_t *mob_line);
    void statistics();

    // ============================================
    memory_order_buffer_line_t* *disambiguation_load_hash;
    uint32_t disambiguation_load_hash_bits_shift;
    uint32_t disambiguation_load_hash_bits_mask;
    // ============================================
    memory_order_buffer_line_t* *disambiguation_store_hash;
    uint32_t disambiguation_store_hash_bits_shift;
    uint32_t disambiguation_store_hash_bits_mask;

    // ===============================
    // Statistics
    uint64_t stat_disambiguation_read_false_positive;
    uint64_t stat_disambiguation_write_false_positive;
    uint64_t stat_address_to_address;


    INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_read_false_positive)
    INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_write_false_positive)
    INSTANTIATE_GET_SET_ADD(uint64_t,stat_address_to_address)
};


