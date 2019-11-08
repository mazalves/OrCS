class hive_register_t : public memory_request_client_t {
    private:
        std::vector<memory_package_t*> nano_memory_requests;
        int ready_count;
        int nano_requests_number;
        int offset;
        bool issued;

        uint32_t HIVE_REGISTER_SIZE;
        uint32_t LINE_SIZE;

    public:
        memory_package_t* request;
        hive_register_t();
        ~hive_register_t();
        void clock();
        void allocate();
        bool installRequest (memory_package_t* request);
        void del();
        
        INSTANTIATE_GET_SET_ADD (uint32_t, LINE_SIZE)
        INSTANTIATE_GET_SET_ADD (uint32_t, HIVE_REGISTER_SIZE)
    
};