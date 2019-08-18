#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

class cache_manager_t {
    private:
        uint64_t read_miss;
        uint64_t read_hit;
        uint64_t write_miss;
        uint64_t write_hit;
        uint64_t offset;

        uint32_t L1_DATA_LATENCY;
        uint32_t L2_LATENCY;
        uint32_t LLC_LATENCY;
        uint32_t LINE_SIZE;

        uint32_t PREFETCHER_ACTIVE;

        uint32_t DATA_LEVELS;
        uint32_t INSTRUCTION_LEVELS;
        uint32_t CACHE_LEVELS;
        uint32_t POINTER_LEVELS;
        uint32_t CACHE_MANAGER_DEBUG;
        uint32_t WAIT_CYCLE;

        uint32_t SIZE_OF_L1_CACHES_ARRAY;     // Numero de caches L1
        uint32_t SIZE_OF_L2_CACHES_ARRAY;     // Numero de caches L2
        uint32_t SIZE_OF_LLC_CACHES_ARRAY;

        std::vector<mshr_entry_t*> mshr_table;

        void check_cache(uint32_t cache_size, uint32_t cache_level);
        mshr_entry_t* add_mshr_entry(memory_order_buffer_line_t* mob_line, uint64_t latency_request, int32_t* cache_indexes, cacheId_t cache_type);
        void installCacheLines(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type);
        uint32_t searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc);
        uint32_t llcMiss(memory_order_buffer_line_t* mob_line, uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, cacheId_t cache_type);
        uint32_t recursiveInstructionSearch(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level);
        uint32_t recursiveDataSearch(memory_order_buffer_line_t *mob_line, uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type);
        uint32_t recursiveDataWrite(memory_order_buffer_line_t *mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type);

    public:
        // instruction and data caches dynamically allocated
        cache_t **data_cache;
        cache_t **instruction_cache;


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
        
        // Getters and setters
        INSTANTIATE_GET_SET_ADD(uint64_t, read_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, read_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, write_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t, write_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t, offset)

        // Prefetcher
        // ==========================================
        prefetcher_t *prefetcher;
};  

#endif // !CACHE_MANAGER_H
