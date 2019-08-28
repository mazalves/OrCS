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

        uint64_t channel_bits_mask;
        uint64_t channel_bits_shift;
        
        uint32_t CHANNEL;
        uint32_t WAIT_CYCLE;
        uint32_t LINE_SIZE;

        float CORE_TO_BUS_CLOCK_RATIO;

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
        inline  uint64_t get_channel(uint64_t address){
                return (address&this->channel_bits_mask)>>this->channel_bits_shift;
        }
        // ==========================================================================
        memory_controller_t();
        ~memory_controller_t();
        void clock();
        void statistics();
        void set_masks();
        uint64_t requestDRAM (mshr_entry_t* request, uint64_t address);
        //statistiscs methods
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_made)
        INSTANTIATE_GET_SET_ADD(uint64_t,operations_executed)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_llc)
        INSTANTIATE_GET_SET_ADD(uint64_t,requests_prefetcher)
        INSTANTIATE_GET_SET_ADD(uint64_t,row_buffer_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t,row_buffer_hit)
        
        INSTANTIATE_GET_SET_ADD(uint32_t,LINE_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t,CHANNEL)
        INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_CYCLE)

        INSTANTIATE_GET_SET_ADD(float,CORE_TO_BUS_CLOCK_RATIO)
};

#endif // MEMORY_CONTROLLER_H