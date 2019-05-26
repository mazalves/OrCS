
class disambiguation_hashed_t
{
private:
    uint32_t ROB_SIZE = 168;
    uint32_t LOAD_HASH_SIZE = 512;
    uint32_t STORE_HASH_SIZE = 512;
    uint32_t DESAMBIGUATION_BLOCK_SIZE = 4;
    uint32_t ADDRESS_TO_ADDRESS = 1;
    uint32_t REGISTER_FORWARD = 1;
    uint32_t MOB_DEBUG = 0;
    uint32_t WAIT_CYCLE = 0;

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


