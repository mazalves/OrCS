#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

class memory_controller_t{

    private:
        // Statistics DRAM
        uint64_t requests_made; //Data Requests made
        uint64_t operations_executed; // number of operations executed
        uint64_t requests_llc; //Data Requests made to LLC
        uint64_t requests_hive;
        uint64_t requests_vima;

        uint64_t requests_prefetcher; //Data Requests made by prefetcher
        uint64_t row_buffer_miss; //Counter row buffer misses
        uint64_t row_buffer_hit; //Counter row buffer hits

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

        std::vector<memory_package_t*> working;

        uint64_t data_bus_availability;
        uint64_t* channel_bus_availability;
        uint64_t latency_burst;
        uint64_t max_requests;
        
        uint32_t BANK;
        uint32_t BANK_ROW_BUFFER_SIZE;
        uint32_t BURST_WIDTH;
        uint32_t CHANNEL;
        uint32_t WAIT_CYCLE;
        uint32_t LINE_SIZE;
        uint32_t DEBUG;

        float CORE_TO_BUS_CLOCK_RATIO;

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

        memory_channel_t *channels;

        uint64_t i;

    public:
        // ==========================================================================
        // Memory Controller Atributes
        // ==========================================================================
        // ==========================================================================
        // Memory Controller Methods
        // ==========================================================================
        void allocate();    //Aloca recursos do Memory Controller
        // Get channel to access DATA
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

        // ==========================================================================
        memory_controller_t();
        ~memory_controller_t();
        void clock();
        void statistics();
        void set_masks();
        uint64_t requestDRAM (memory_package_t* request);
        //statistiscs methods
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
        INSTANTIATE_GET_SET_ADD(uint64_t,operations_executed)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_llc)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_hive)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_vima)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_prefetcher)
        INSTANTIATE_GET_SET_ADD(uint64_t,row_buffer_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t,row_buffer_hit)

        INSTANTIATE_GET_SET_ADD(uint64_t,latency_burst)
        
        INSTANTIATE_GET_SET_ADD(uint32_t,LINE_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t,BANK)
        INSTANTIATE_GET_SET_ADD(uint32_t,BANK_ROW_BUFFER_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t,BURST_WIDTH)
        INSTANTIATE_GET_SET_ADD(uint32_t,CHANNEL)
        INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_CYCLE)
        INSTANTIATE_GET_SET_ADD(uint32_t,DEBUG)

        INSTANTIATE_GET_SET_ADD(float,CORE_TO_BUS_CLOCK_RATIO)

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

#endif // MEMORY_CONTROLLER_H