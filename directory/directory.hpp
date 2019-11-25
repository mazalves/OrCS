class directory_t {
    
    public:
        uint32_t n_sets;
        directory_set_t *sets;

        directory_t();
        ~directory_t();
        void allocate(cache_t llc, uint32_t POINTER_LEVELS);
        void setCachePointers(line_t *cache_lines, uint32_t cache_level, memory_operation_t mem_op);
        int32_t getDirectoryLine(uint32_t idx, uint64_t tag);
        void removeCachePointers(uint32_t cache_levels, uint32_t idx, int32_t line);
        void tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag, uint32_t offset);
        void installCachePointers(line_t ***cache_lines, uint32_t n_proc, uint32_t cache_levels, uint32_t idx, int32_t line, memory_operation_t mem_op, uint64_t *cache_tags);
        uint32_t validCacheLine(uint64_t address, uint32_t cache_level);
        uint32_t dirtyCacheLine(uint64_t address, uint32_t cache_level);
        void copyCacheInfo(uint64_t address, uint32_t to_cache_level, uint32_t cache_level);
};