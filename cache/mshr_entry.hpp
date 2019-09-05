class mshr_entry_t {
    public:
        bool valid;
        uint64_t tag;
        bool issued;
        bool treated;
        uint64_t latency;
        uint64_t cycle_created;
        std::vector<memory_order_buffer_line_t*> requests;

        mshr_entry_t();
        ~mshr_entry_t();
        bool contains (memory_order_buffer_line_t* mob_line);
        void remove (memory_order_buffer_line_t* mob_line);
};