#ifndef STRIDE_PREFETCHER_H
#define STRIDE_PREFETCHER_H

class stride_prefetcher_t{
    private:

        uint32_t DISTANCE;
        uint32_t STRIDE_TABLE_SIZE;


    public:
        stride_prefetcher_t();
        ~stride_prefetcher_t();

        stride_table_t *stride_table;
        inline uint32_t searchLRU(); //Replacement
        void allocate(uint32_t NUMBER_OF_PROCESSORS);
        int32_t searchPattern(uint64_t pc);//search entry 
        uint32_t installStride(uint64_t pc, uint64_t address); //install new entry
        uint32_t updateStride(uint64_t pc, uint64_t address, status_stride_prefetcher_t status); //update entry
        int64_t verify(uint64_t pc,uint64_t address);//returrn address to prefetch
        void statistics();

        INSTANTIATE_GET_SET_ADD (uint32_t, DISTANCE)
        INSTANTIATE_GET_SET_ADD (uint32_t, STRIDE_TABLE_SIZE)
        
    };
#endif // !STRIDE_PREFETCHER_
