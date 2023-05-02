
class vima_prefetcher_t {
        circular_buffer_t<conversion_status_t> prefetches;
        uint32_t CONTIGUOUS_CONVERSIONS_TO_PREFETCH;
        uint32_t PREFETCH_SIZE;

    public:
        vima_prefetcher_t() {
            
        }

        void initialize(uint32_t PREFETCH_BUFFER_SIZE, uint32_t CONTIGUOUS_CONVERSIONS_TO_PREFETCH, uint32_t PREFETCH_SIZE)
        {
            assert(PREFETCH_BUFFER_SIZE >= PREFETCH_SIZE);
            prefetches.allocate(PREFETCH_BUFFER_SIZE);
            this->CONTIGUOUS_CONVERSIONS_TO_PREFETCH = CONTIGUOUS_CONVERSIONS_TO_PREFETCH;
            this->PREFETCH_SIZE = PREFETCH_SIZE;
        }

        void make_prefetch(conversion_status_t *prev_conversion);

        conversion_status_t* get_prefetch();

        void pop_prefetch();

        void vima_execution_completed(memory_package_t *vima_package, uint64_t readyAt);

        void shift_sequential_conversion(conversion_status_t *status);

};