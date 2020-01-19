class vima_controller_t {
    private:
        uint32_t VIMA_BUFFER;
        float CORE_TO_BUS_CLOCK_RATIO;
        
        uint32_t LINE_SIZE;
        uint32_t VIMA_DEBUG;
        
        uint32_t* vima_op_latencies;
        memory_package_t** vima_sub_requests;
        int64_t vima_buffer_start;
        int64_t vima_buffer_end;
        int64_t vima_buffer_used;
        std::vector<memory_package_t*> vima_instructions;

        vima_cache_t* cache;

        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_BUFFER)
        INSTANTIATE_GET_SET_ADD (uint32_t, LINE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t, VIMA_DEBUG)
        INSTANTIATE_GET_SET_ADD (float, CORE_TO_BUS_CLOCK_RATIO)
        
        void print_vima_instructions();
        
    public:
        vima_controller_t();
        ~vima_controller_t();
        void clock();
        void allocate();
        bool addRequest (memory_package_t* request);
        void set_sub_requests (memory_package_t* request);
        void reset_sub_requests (size_t index);
        void instruction_ready (size_t index);
};