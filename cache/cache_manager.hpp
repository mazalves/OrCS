#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H
using namespace std;

class cache_manager_t {

    private:
        uint64_t read_miss;
        uint64_t read_hit;
        uint64_t write_miss;
        uint64_t write_hit;
        uint64_t offset;
        uint64_t mshr_index;

        uint32_t LINE_SIZE;
        uint32_t PREFETCHER_ACTIVE;
        uint32_t DATA_LEVELS;
        uint32_t INSTRUCTION_LEVELS;
        uint32_t POINTER_LEVELS;
        uint32_t LLC_CACHES;
        uint32_t CACHE_MANAGER_DEBUG;
        uint32_t WAIT_CYCLE;
        uint32_t NUMBER_OF_PROCESSORS;
        uint32_t MAX_PARALLEL_REQUESTS_CORE;

        std::vector<mshr_entry_t*> mshr_table;

        void check_cache(uint32_t cache_size, uint32_t cache_level);
        void add_mshr_entry(memory_order_buffer_line_t* mob_line, uint64_t latency_request);
        bool isInMSHR (memory_order_buffer_line_t* mob_line);
        void copy_cache(cache_t **cache, cache_t *aux_cache, uint32_t n_levels, uint32_t *v_levels, uint32_t cache_amount);
        uint32_t *get_cache_levels(std::vector<uint32_t> &v_levels, cache_t *cache, uint32_t cache_amount);
        cache_t *get_cache_info(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs, uint32_t *N_CACHES);
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
        directory_t *directory;
        uint32_t *ICACHE_AMOUNT;
        uint32_t *DCACHE_AMOUNT;

        cache_manager_t();
        ~cache_manager_t();
        void allocate(uint32_t NUMBER_OF_PROCESSORS);
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

        INSTANTIATE_GET_SET_ADD(uint32_t, LINE_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t, PREFETCHER_ACTIVE)

        INSTANTIATE_GET_SET_ADD(uint32_t, DATA_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t, INSTRUCTION_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t, LLC_CACHES)
        INSTANTIATE_GET_SET_ADD(uint32_t, CACHE_MANAGER_DEBUG)
        INSTANTIATE_GET_SET_ADD(uint32_t, WAIT_CYCLE)

        INSTANTIATE_GET_SET_ADD(uint32_t, NUMBER_OF_PROCESSORS)
        INSTANTIATE_GET_SET_ADD(uint32_t, MAX_PARALLEL_REQUESTS_CORE)

        prefetcher_t *prefetcher;
};  

#endif // !CACHE_MANAGER_H
