class vima_vector_t {
    private:
        memory_package_t* sub_requests;
        uint32_t no_sub_requests;
        uint32_t sub_ready;
        uint64_t address;

        uint32_t LINE_SIZE;
        uint32_t VIMA_VECTOR_SIZE;

    public:
        package_state_t status;
        uint64_t tag;
        uint64_t lru;

        vima_vector_t();
        ~vima_vector_t();
        void clock();
        void allocate();
        void print_vector();

        INSTANTIATE_GET_SET_ADD (uint32_t,no_sub_requests)
        INSTANTIATE_GET_SET_ADD (uint64_t,address)
        INSTANTIATE_GET_SET_ADD (uint64_t,tag)
        INSTANTIATE_GET_SET_ADD (uint64_t,lru)

        INSTANTIATE_GET_SET_ADD (uint32_t,LINE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t,VIMA_VECTOR_SIZE)

};