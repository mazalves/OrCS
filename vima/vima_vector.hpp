class vima_vector_t {
    public:
        uint64_t cycle_ready;
        uint64_t cycle_used;
        uint32_t tag;
        bool dirty, ready;

        uint64_t no_sub_requests;

        uint64_t LINE_SIZE;
        uint64_t VIMA_VECTOR_SIZE;

        memory_package_t* sub_requests;

        INSTANTIATE_GET_SET_ADD (uint64_t,cycle_ready)
        INSTANTIATE_GET_SET_ADD (uint64_t,cycle_used)
        INSTANTIATE_GET_SET_ADD (uint32_t,tag)

        INSTANTIATE_GET_SET_ADD (uint64_t,LINE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint64_t,VIMA_VECTOR_SIZE)

        INSTANTIATE_GET_SET_ADD (uint64_t,no_sub_requests)
        
        vima_vector_t();
        ~vima_vector_t();
        void clock();
        void allocate();
};