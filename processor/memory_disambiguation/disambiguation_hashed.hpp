
class disambiguation_hashed_t: public desambiguation_t
{
private:
    uint32_t ROB_SIZE;
    uint32_t LOAD_HASH_SIZE;
    uint32_t STORE_HASH_SIZE;
    uint32_t DESAMBIGUATION_BLOCK_SIZE;
    uint32_t ADDRESS_TO_ADDRESS;
    uint32_t REGISTER_FORWARD;
    uint32_t MOB_DEBUG;
    uint32_t WAIT_CYCLE;
    uint32_t DEBUG;

public:
    disambiguation_hashed_t(/* args */);
    ~disambiguation_hashed_t();
    void allocate();
    void make_memory_dependences(memory_order_buffer_line_t *mob_line);
    void solve_memory_dependences(memory_order_buffer_line_t *mob_line);
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

    INSTANTIATE_GET_SET_ADD(uint32_t,ROB_SIZE)
    INSTANTIATE_GET_SET_ADD(uint32_t,LOAD_HASH_SIZE)
    INSTANTIATE_GET_SET_ADD(uint32_t,STORE_HASH_SIZE)
    INSTANTIATE_GET_SET_ADD(uint32_t,DESAMBIGUATION_BLOCK_SIZE)
    INSTANTIATE_GET_SET_ADD(uint32_t,ADDRESS_TO_ADDRESS)
    INSTANTIATE_GET_SET_ADD(uint32_t,REGISTER_FORWARD)
    INSTANTIATE_GET_SET_ADD(uint32_t,MOB_DEBUG)
    INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_CYCLE)
    INSTANTIATE_GET_SET_ADD(uint32_t,DEBUG)
};


