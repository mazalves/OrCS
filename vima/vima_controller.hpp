class vima_controller_t {
    private:
        uint32_t VIMA_BUFFER;
        uint32_t VIMA_VECTOR_SIZE;
        uint32_t VIMA_CACHE_ASSOCIATIVITY;
        uint32_t VIMA_CACHE_LATENCY;
        uint32_t VIMA_CACHE_SIZE;
        uint32_t VIMA_UNBALANCED;
        uint32_t VIMA_FU_INT_COUNT;
        uint32_t VIMA_FU_FP_COUNT;
        uint32_t VIMA_FU_INT_PER_INST;
        uint32_t VIMA_FU_FP_PER_INST;
        float CORE_TO_BUS_CLOCK_RATIO;
        
        uint32_t lines;
        uint32_t sets;
        uint32_t* vima_op_latencies;
        uint64_t current_cache_access_latency;
        uint64_t last_heartbeat;
        vima_vector_t** cache;
        memory_package_t** vima_buffer;
        
        uint32_t vima_buffer_start;
        uint32_t vima_buffer_end;
        uint32_t vima_buffer_count;
        uint16_t* store_hash;
        uint32_t bits_shift;
        uint32_t free_lines;

        uint32_t vima_fu_int_free;
        uint32_t vima_fu_fp_free;

        uint64_t index_bits_mask;
        uint64_t index_bits_shift;

        uint64_t tag_bits_mask;
        uint64_t tag_bits_shift;

        uint64_t cache_reads;
        uint64_t cache_writes;
        uint64_t cache_hits;
        uint64_t cache_misses;
        uint64_t cache_accesses;
        uint64_t cache_writebacks;

        uint64_t i;
        uint64_t current_index;
        uint64_t request_count;
        uint64_t total_wait;
        uint64_t ready_cycle;

        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_BUFFER)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_VECTOR_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_CACHE_ASSOCIATIVITY)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_CACHE_LATENCY)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_CACHE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_UNBALANCED)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_FU_INT_COUNT)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_FU_FP_COUNT)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_FU_INT_PER_INST)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_FU_FP_PER_INST)
        INSTANTIATE_GET_SET_ADD (float, CORE_TO_BUS_CLOCK_RATIO)

        INSTANTIATE_GET_SET_ADD (uint32_t, lines)
        INSTANTIATE_GET_SET_ADD (uint32_t, sets)
        INSTANTIATE_GET_SET_ADD (uint64_t, cache_reads)
        INSTANTIATE_GET_SET_ADD (uint64_t, cache_writes)
        INSTANTIATE_GET_SET_ADD (uint64_t, cache_hits)
        INSTANTIATE_GET_SET_ADD (uint64_t, cache_misses)
        INSTANTIATE_GET_SET_ADD (uint64_t, cache_accesses)
        INSTANTIATE_GET_SET_ADD (uint64_t, cache_writebacks)
        
        void print_vima_instructions();
        vima_vector_t* search_cache (uint64_t address, cache_status_t* result);
        void check_completion (int index);
        void process_instruction (uint32_t index);
        void write (int index);
        void print_buffer();
        uint64_t hash (uint64_t address);
        
        // Get channel to access DATA
        inline uint64_t get_index(uint64_t addr) {
            return (addr & this->index_bits_mask) >> this->index_bits_shift;
        }

        inline uint64_t get_tag(uint64_t addr) {
	    return addr >> this->index_bits_shift;
            //return (addr & this->tag_bits_mask) >> this->tag_bits_shift;
        }
        
    public:
        vima_controller_t();
        ~vima_controller_t();
        void clock();
        void allocate();
        bool addRequest (memory_package_t* request);
        void instruction_ready (size_t index);
        void statistics();
        void reset_statistics();
};
