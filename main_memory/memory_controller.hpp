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

        uint64_t channel_bits_mask;
        uint64_t channel_bits_shift;
        
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

    public:
        // ==========================================================================
        // Memory Controller Atributes
        // ==========================================================================
        // ==========================================================================
        // Memory Controller Methods
        // ==========================================================================
        void allocate();    //Aloca recursos do Memory Controller
        // Get channel to access DATA
        inline uint64_t get_channel(uint64_t addr) {
            return (addr & this->channel_bits_mask) >> this->channel_bits_shift;
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
        
        INSTANTIATE_GET_SET_ADD(uint32_t,LINE_SIZE)
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