#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

class cache_manager_t {
    private:
        uint64_t read_miss;
        uint64_t read_hit;
        uint64_t write_miss;
        uint64_t write_hit;

        void check_cache(uint32_t cache_size, uint32_t cache_level);
        void installCacheLines(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type);
        uint32_t searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc);
        uint32_t llcMiss(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, cacheId_t cache_type);
        uint32_t recursiveInstructionSearch(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level);
        uint32_t recursiveDataSearch(memory_order_buffer_line_t *mob_line, uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type);
        uint32_t recursiveDataWrite(memory_order_buffer_line_t *mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type);

    public:
        // instruction and data caches dynamically allocated
        cache_t **data_cache = new cache_t*[DATA_LEVELS];
        cache_t **instruction_cache = new cache_t*[INSTRUCTION_LEVELS];


        // Constructor
        cache_manager_t();

        // Desctructor
        ~cache_manager_t();

        void allocate();
        void clock();//for prefetcher
        void statistics(uint32_t core_id);
        void generateIndexArray(uint32_t processor_id, int32_t *cache_indexes);
        uint32_t searchInstruction(uint32_t processor_id, uint64_t instructionAddress);
        uint32_t searchData(memory_order_buffer_line_t *mob_line);
        uint32_t writeData(memory_order_buffer_line_t *mob_line);

        // Getters and setters
        INSTANTIATE_GET_SET_ADD(uint64_t, read_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, read_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, write_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, write_hit)

        // Prefetcher
        #if PREFETCHER_ACTIVE
            prefetcher_t *prefetcher;
        #endif
        //
        // // EMC Data Collect
        // uint32_t search_EMC_Data(memory_order_buffer_line_t *mob_line);
};

#endif // !CACHE_MANAGER_H
