class directory_t {
    
    public:
        uint32_t OFFSET;

        uint32_t n_sets;
        directory_set_t *sets;

        directory_t();
        ~directory_t();

        void allocate(cache_t llc, uint32_t POINTER_LEVELS);
        void setCachePointers(way_t *cache_ways, uint32_t cache_level, memory_operation_t mem_op);
        void nullingCaches(uint64_t address, uint32_t cache_levels);
        void tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag);
        void installCachePointers(way_t ***cache_ways, uint32_t n_proc, uint32_t cache_levels, uint32_t idx, int32_t way, memory_operation_t mem_op);
        void copyCacheInfo(uint64_t address, uint32_t to_cache_level, uint32_t from_cache_level);
        void nullCachePointer(uint64_t address, uint32_t cache_level);
        int32_t getDirectoryLine(uint32_t idx, uint64_t tag);
        uint32_t dirtyCacheLine(uint64_t address, uint32_t cache_level);
        uint32_t validCacheLine(uint64_t address, uint32_t cache_level);

        INSTANTIATE_GET_SET_ADD(uint32_t, OFFSET)
};