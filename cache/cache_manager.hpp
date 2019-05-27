#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H
class cache_manager_t{
    private:
        uint64_t instructionSearched;
        uint64_t instructionLLCSearched;
        uint64_t readMiss;
        uint64_t readHit;
        uint64_t writeMiss;
        uint64_t writeHit;

        uint32_t L1_DATA_LATENCY;
        uint32_t L2_LATENCY;
        uint32_t LLC_LATENCY;

        //uint32_t PREFETCHER_ACTIVE = 0;

        uint32_t CACHE_MANAGER_DEBUG;
        uint32_t WAIT_CYCLE;

        uint32_t SIZE_OF_L1_CACHES_ARRAY;     // Numero de caches L1
        uint32_t SIZE_OF_L2_CACHES_ARRAY;     // Numero de caches L2
        uint32_t SIZE_OF_LLC_CACHES_ARRAY;
    public:
        cache_t *L1_data_cache;
        cache_t *L2_data_cache;
        cache_t *LLC_data_cache;
        cache_t *inst_cache;

        cache_manager_t();
        ~cache_manager_t();
        void allocate();
        void clock();//for prefetcher
        void statistics(uint32_t core_id);
        uint32_t searchInstruction(uint32_t processor_id,uint64_t instructionAddress);
        uint32_t searchData(memory_order_buffer_line_t *mob_line);
        uint32_t writeData(memory_order_buffer_line_t *mob_line);
        int32_t generate_index_array(uint32_t processor_id,cacheLevel_t level);
        INSTANTIATE_GET_SET_ADD(uint64_t,instructionSearched)
        INSTANTIATE_GET_SET_ADD(uint64_t,instructionLLCSearched)
        INSTANTIATE_GET_SET_ADD(uint64_t,readMiss)
        INSTANTIATE_GET_SET_ADD(uint64_t,readHit)
        INSTANTIATE_GET_SET_ADD(uint64_t,writeMiss)
        INSTANTIATE_GET_SET_ADD(uint64_t,writeHit)
        // ==========================================
        // Prefetcher
        // ==========================================
        prefetcher_t *prefetcher;
};  

#endif // !CACHE_MANAGER_H