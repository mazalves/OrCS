class mshr_entry_t {
    public:
        bool valid;
        uint64_t tag;
        bool issued;
        uint64_t latency;
        int32_t *cache_indexes;
        cacheId_t cache_type;
        std::vector<memory_order_buffer_line_t*> requests;

        mshr_entry_t();
        ~mshr_entry_t();
        void updateRequests();
        void addRequest (memory_order_buffer_line_t* mob_line);
};