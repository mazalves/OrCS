class memory_channel_t {
    public:
        // =================================================
        // attr DRAM
        // =================================================
        uint64_t channel_bits_mask;
        uint64_t rank_bits_mask;
        uint64_t bank_bits_mask;
        uint64_t row_bits_mask;
        uint64_t colrow_bits_mask;
        uint64_t colbyte_bits_mask;
        uint64_t not_column_bits_mask;
        
        // Shifts bits
        uint64_t channel_bits_shift;
        uint64_t colbyte_bits_shift;
        uint64_t colrow_bits_shift;
        uint64_t bank_bits_shift;
        uint64_t row_bits_shift;
        uint64_t controller_bits_shift;

        /// Keep track of the last command in each bank
        memory_controller_command_t *bank_last_command; /// Last command sent to each bank

        bool *bank_is_ready;
        uint64_t *bank_last_row;
        bool *bank_is_drain_write;
        uint64_t bank_last_transmission;
        uint64_t **bank_last_command_cycle;      /// Cycle of the Last command sent to each bank
        uint64_t *channel_last_command_cycle;      /// Cycle of the last command type
        std::vector<memory_package_t*> *bank_read_requests;
        std::vector<memory_package_t*> *bank_write_requests;

        uint32_t last_bank_selected;

        uint64_t stat_row_buffer_hit;
        uint64_t stat_row_buffer_miss;

        uint64_t latency_burst;
        uint64_t data_bus_availability;

        uint64_t i;

        uint32_t RANK;
        uint32_t BANK;
        uint32_t BANK_BUFFER_SIZE;
        uint32_t BANK_ROW_BUFFER_SIZE;
        uint32_t CHANNEL;
        uint32_t CLOSED_ROW;
        uint32_t LINE_SIZE;
        uint32_t DEBUG;

        uint32_t TIMING_AL;     // Added Latency for column accesses
        uint32_t TIMING_CAS;    // Column Access Strobe (CL) latency
        uint32_t TIMING_CCD;    // Column to Column Delay
        uint32_t TIMING_CWD;    // Column Write Delay (CWL) or simply WL
        uint32_t TIMING_FAW;   // Four (row) Activation Window
        uint32_t TIMING_RAS;   // Row Access Strobe
        uint32_t TIMING_RC;    // Row Cycle
        uint32_t TIMING_RCD;    // Row to Column comand Delay
        uint32_t TIMING_RP;     // Row Precharge
        uint32_t TIMING_RRD;    // Row activation to Row activation Delay
        uint32_t TIMING_RTP;    // Read To Precharge
        uint32_t TIMING_WR;    // Write Recovery time
        uint32_t TIMING_WTR;

        request_priority_t REQUEST_PRIORITY;
        write_priority_t WRITE_PRIORITY;
        
        // Get channel to access DATA
        inline uint64_t get_column(uint64_t addr) {
            return (addr & this->colrow_bits_mask) >> this->colrow_bits_shift;
        }

        inline uint64_t get_channel(uint64_t addr) {
            return (addr & this->channel_bits_mask) >> this->channel_bits_shift;
        }

        inline uint64_t get_bank(uint64_t addr) {
            return (addr & this->bank_bits_mask) >> this->bank_bits_shift;
        }
        //get row accessed
        inline uint64_t get_row(uint64_t address){
            return (address & this->not_column_bits_mask);
        }

        memory_channel_t();
        ~memory_channel_t();

        uint64_t get_minimum_latency(uint32_t bank, memory_controller_command_t next_command);
        memory_package_t* findNext (uint32_t bank);
        memory_package_t* findNextRead (uint32_t bank);
        memory_package_t* findNextWrite (uint32_t bank);
        void allocate();
        bool addRequest (memory_package_t* request);
        void set_masks();
        void clock();

        INSTANTIATE_GET_SET_ADD (uint64_t, latency_burst)

        INSTANTIATE_GET_SET_ADD (uint64_t, channel_bits_mask)
        INSTANTIATE_GET_SET_ADD (uint64_t, rank_bits_mask)
        INSTANTIATE_GET_SET_ADD (uint64_t, bank_bits_mask)
        INSTANTIATE_GET_SET_ADD (uint64_t, row_bits_mask)
        INSTANTIATE_GET_SET_ADD (uint64_t, colrow_bits_mask)
        INSTANTIATE_GET_SET_ADD (uint64_t, colbyte_bits_mask)
        
        // Shifts bits
        INSTANTIATE_GET_SET_ADD (uint64_t, channel_bits_shift)
        INSTANTIATE_GET_SET_ADD (uint64_t, colbyte_bits_shift)
        INSTANTIATE_GET_SET_ADD (uint64_t, colrow_bits_shift)
        INSTANTIATE_GET_SET_ADD (uint64_t, bank_bits_shift)
        INSTANTIATE_GET_SET_ADD (uint64_t, row_bits_shift)


        INSTANTIATE_GET_SET_ADD (uint64_t, stat_row_buffer_hit)
        INSTANTIATE_GET_SET_ADD (uint64_t, stat_row_buffer_miss)

        INSTANTIATE_GET_SET_ADD(uint32_t, RANK)
        INSTANTIATE_GET_SET_ADD(uint32_t, BANK)
        INSTANTIATE_GET_SET_ADD(uint32_t, BANK_BUFFER_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t, BANK_ROW_BUFFER_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t, CHANNEL)
        INSTANTIATE_GET_SET_ADD(uint32_t, CLOSED_ROW)
        INSTANTIATE_GET_SET_ADD(uint32_t, LINE_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t, DEBUG)

        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_AL)     // Added Latency for column accesses
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_CAS)    // Column Access Strobe (CL) latency
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_CCD)    // Column to Column Delay
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_CWD)    // Column Write Delay (CWL) or simply WL
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_FAW)   // Four (row) Activation Window
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_RAS)   // Row Access Strobe
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_RC)    // Row Cycle
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_RCD)    // Row to Column comand Delay
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_RP)     // Row Precharge
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_RRD)    // Row activation to Row activation Delay
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_RTP)    // Read To Precharge
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_WR)    // Write Recovery time
        INSTANTIATE_GET_SET_ADD(uint32_t, TIMING_WTR)
};