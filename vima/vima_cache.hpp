class vima_cache_t {
    struct vima_cache_line_t {
        uint64_t cycle_ready;
        uint64_t cycle_used;
        uint32_t tag;
        bool dirty;
    } typedef vima_cache_line_t;

    private:
        uint32_t VIMA_CACHE_LINE_SIZE;
        uint32_t VIMA_CACHE_SIZE;
        uint32_t VIMA_CACHE_ASSOCIATIVITY;
        uint32_t VIMA_CACHE_LATENCY;

        uint32_t cache_accesses;
        uint32_t cache_hits;
        uint32_t cache_misses;

        uint32_t offset;
        uint32_t index_bits_shift;
        uint32_t tag_bits_shift;

        uint32_t index_bits_mask;
        uint32_t tag_bits_mask;

        vima_cache_line_t** cache;

        // Get channel to access DATA
        inline uint64_t get_index(uint64_t addr) {
            return (addr & this->index_bits_mask) >> this->index_bits_shift;
        }

        inline uint64_t get_tag(uint64_t addr) {
            return (addr & this->tag_bits_mask) >> this->tag_bits_shift;
        }
        
    public:
        vima_cache_t();
        ~vima_cache_t();
        void allocate();
        bool searchAddress (uint64_t address);
        void installLine (uint64_t address);

        INSTANTIATE_GET_SET_ADD (uint32_t,VIMA_CACHE_LINE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t,VIMA_CACHE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t,VIMA_CACHE_ASSOCIATIVITY)
        INSTANTIATE_GET_SET_ADD (uint32_t,VIMA_CACHE_LATENCY)

        INSTANTIATE_GET_SET_ADD (uint32_t,cache_accesses)
        INSTANTIATE_GET_SET_ADD (uint32_t,cache_hits)
        INSTANTIATE_GET_SET_ADD (uint32_t,cache_misses)
};