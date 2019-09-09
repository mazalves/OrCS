#ifndef PREFETCHER_H
#define PREFETCHER_H


class prefetcher_t{
    private:
        uint32_t totalPrefetched;
        uint32_t usefulPrefetches;
        uint32_t latePrefetches;
        uint64_t totalCycleLate;
        stride_prefetcher_t *prefetcher;

        uint32_t PARALLEL_PREFETCH;
    public:
        // constructors
        prefetcher_t();
        ~prefetcher_t();
        //Aux objects
        std::vector<uint64_t> prefetch_waiting_complete;
        //Object prefetcher
        // methods statistics
        INSTANTIATE_GET_SET_ADD(uint32_t,totalPrefetched)
        INSTANTIATE_GET_SET_ADD(uint32_t,usefulPrefetches)
        INSTANTIATE_GET_SET_ADD(uint32_t,latePrefetches)
        INSTANTIATE_GET_SET_ADD(uint32_t,totalCycleLate)
        INSTANTIATE_GET_SET_ADD(uint32_t,PARALLEL_PREFETCH)

        void allocate(uint32_t NUMBER_OF_PROCESSORS);
        void statistics();
        void prefecht(memory_order_buffer_line_t *mob_line,cache_t *cache);//endereco cache alvo
};
#endif