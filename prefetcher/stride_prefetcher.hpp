#ifndef STRIDE_PREFETCHER_H
#define STRIDE_PREFETCHER_H

class stride_prefetcher_t{

public:
    stride_prefetcher_t();
    ~stride_prefetcher_t();

    stride_table_t *stride_table;
    inline uint32_t searchLRU(); //Replacement
    void allocate();
    int32_t searchPattern(uint64_t pc);//search entry 
    uint32_t installStride(uint64_t pc, uint64_t address); //install new entry
    uint32_t updateStride(uint64_t pc, uint64_t address, status_stride_prefetcher_t status); //update entry
    int64_t verify(uint64_t pc,uint64_t address);//returrn address to prefetch
    void statistics();
    
};
#endif // !STRIDE_PREFETCHER_
