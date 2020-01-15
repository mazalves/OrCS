class directory_t
{

private:
    uint32_t OFFSET;
    uint32_t DATA_LEVELS;
    uint32_t INSTRUCTION_LEVELS;
    uint32_t POINTER_LEVELS;

public:
    uint32_t n_sets;
    directory_set_t *sets;
    uint32_t *LATENCIES;

    directory_t();
    ~directory_t();

    int32_t getDirectoryLine(uint32_t idx, uint64_t tag);
    uint32_t dirtyCacheLine(uint64_t address, uint32_t cache_level);
    uint32_t validCacheLine(uint64_t address, uint32_t cache_level);
    int32_t read(directory_way_t *way, int32_t *cache_indexes, memory_operation_t mem_op);
    int32_t searchAddress(memory_package_t *mob_line, int32_t *cache_indexes, uint32_t &latency_request);
    uint32_t checkInclusionPolicy(directory_way_t *way, int32_t *cache_indexes, memory_operation_t mem_op, uint32_t level);
    void allocate(cache_t llc, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS, uint32_t *ICACHE_AMOUNT, uint32_t *DCACHE_AMOUNT, uint32_t *CACHE_LATENCIES);
    void setCachePointers(line_t *cache_ways, uint32_t cache_level, memory_operation_t mem_op);
    void nullingCaches(uint64_t address, uint32_t cache_levels);
    void tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag);
    void installCachePointers(line_t ***cache_ways, int32_t *cache_indexes, memory_operation_t mem_op, uint32_t idx, int32_t way);
    void copyCacheInfo(uint64_t address, uint32_t to_cache_level, uint32_t from_cache_level);
    void nullCachePointer(uint64_t address, uint32_t cache_level);
    uint32_t cache_read(level_way_t *cache, uint32_t cache_level);
    
    INSTANTIATE_GET_SET_ADD(uint32_t, OFFSET)
    INSTANTIATE_GET_SET_ADD(uint32_t, DATA_LEVELS)
    INSTANTIATE_GET_SET_ADD(uint32_t, INSTRUCTION_LEVELS)
    INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};