#ifndef MEMORY_CONTROLLER_H
#define MEMORY_CONTROLLER_H

class memory_controller_t{

    private:
        // Statistics DRAM
        uint64_t requests_made; //Data Requests made
        uint64_t operations_executed; // number of operations executed
        uint64_t requests_llc; //Data Requests made to LLC
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
        uint64_t col_row_bits_mask;
        uint64_t col_byte_bits_mask;
        uint64_t latency_burst;
        // Shifts bits
        uint64_t channel_bits_shift;
        uint64_t colbyte_bits_shift;
        uint64_t colrow_bits_shift;
        uint64_t bank_bits_shift;
        uint64_t row_bits_shift;
        // =================================================
        // Struct object defines RAM
        // =================================================
        
        typedef struct request{
            uint64_t request_start;
            uint64_t request_end;

        }request_t;
        typedef struct bus{
            std::vector<request_t> requests;
        }bus_t;
        typedef struct RAM{
            uint64_t last_row_accessed;
            uint64_t cycle_ready;
        }RAM_t;

        uint32_t LINE_SIZE = 64;

        uint32_t CHANNEL = 2;
        uint32_t RANK = 1;
        uint32_t BANK = 8;
        uint32_t ROW_BUFFER = (RANK*BANK)*1024;
        // =====================Parametes Comandd=======================
        uint32_t BURST_WIDTH = 8;
        uint32_t RAS = 44;
        uint32_t CAS = 44;
        uint32_t ROW_PRECHARGE = 44;
        // ============================================

        //uint64_t RAM_SIZE = 4 * MEGA * KILO;
        uint32_t PARALLEL_LIM_ACTIVE = 1;
        uint32_t MAX_PARALLEL_REQUESTS_CORE = 10;

        uint32_t MEM_CONTROLLER_DEBUG = 0;
        uint32_t WAIT_CYCLE = 0;

    public:
        // ==========================================================================
        // Memory Controller Atributes
        // ==========================================================================
        RAM_t *ram; 
        bus_t *data_bus;//n_channel * N
        // ==========================================================================
        // Methods DRAM
        // ==========================================================================
        void set_masks();
        uint64_t add_channel_bus(uint64_t address, uint64_t ready);
        // ==========================================================================
        // Memory Controller Methods
        // ==========================================================================
        void allocate();    //Aloca recursos do Memory Controller
        // Get channel to access DATA
        inline  uint64_t get_channel(uint64_t address){
            return (address&this->channel_bits_mask)>>this->channel_bits_shift;
        }
        // get memory bank accessed
        inline  uint64_t get_bank(uint64_t address){
            return (address&this->bank_bits_mask)>>this->bank_bits_shift;
        }
        //get row accessed
        inline uint64_t get_row(uint64_t address){
            return (address&this->row_bits_mask)>>this->row_bits_shift;
        }
        // ==========================================================================
        memory_controller_t();
        ~memory_controller_t();
        void clock();
        void statistics();
        //statistiscs methods
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
        INSTANTIATE_GET_SET_ADD(uint64_t,operations_executed)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_llc)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_prefetcher)
        INSTANTIATE_GET_SET_ADD(uint64_t,row_buffer_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t,row_buffer_hit)
        //request DRAM data
        uint64_t requestDRAM(uint64_t address);
        
};

#endif // MEMORY_CONTROLLER_H