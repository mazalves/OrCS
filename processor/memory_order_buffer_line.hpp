class memory_order_buffer_line_t : public memory_request_client_t {
    public:
        uint64_t opcode_address;
        uint64_t memory_address;
        uint32_t memory_size;

        //HIVE
        bool is_hive;
        uint64_t hive_read1;
        uint64_t hive_read2;
        uint64_t hive_write;

        //VIMA
        bool is_vima;
        uint64_t vima_read1;
        uint64_t vima_read2;
        uint64_t vima_write;
         
        reorder_buffer_line_t* rob_ptr;                 /// rob pointer
        /// Memory Dependencies Control
        bool uop_executed; //*
        uint64_t uop_number;
        uint64_t cycle_send_request;                       // Cycle of send request
        uint64_t readyToGo;                                 /// Cycles of waiting
        uint32_t wait_mem_deps_number;                      /// Must wait BEFORE execution
        memory_order_buffer_line_t* *mem_deps_ptr_array;    /// Elements to wake-up AFTER execution
        //==========================================================================================
        uint32_t processor_id;                              // id of processor make request 
        uint64_t cycle_sent_to_DRAM;
        //==========================================================================================
        //Control variables
        bool processed; 
        bool sent; //*
        bool forwarded_data;
        bool waiting_DRAM;
        bool core_generate_miss;
        // ====================================================================
        /// Methods
        // ====================================================================
        memory_order_buffer_line_t();
        ~memory_order_buffer_line_t();

        void package_clean();
        std::string content_to_string();
        //select packages
        static int32_t find_free(memory_order_buffer_line_t *input_array, uint32_t size_array);
        static int32_t find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state);
        static void printAllOrder(memory_order_buffer_line_t* input_array, uint32_t size_array,uint32_t start,uint32_t end);
};
