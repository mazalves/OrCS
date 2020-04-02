class memory_request_client_t {
    public:
        package_state_t status;
        uint32_t readyAt;
        memory_operation_t memory_operation;
        bool waiting_DRAM;

};