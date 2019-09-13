#include "./../simulator.hpp"
#include <string>
// ============================================================================
memory_controller_t::memory_controller_t(){
    this->requests_made = 0; //Data Requests made
    this->operations_executed = 0; // number of operations executed
    this->requests_llc = 0; //Data Requests made to LLC
    this->requests_prefetcher = 0; //Data Requests made by prefetcher
    this->row_buffer_miss = 0; //Counter row buffer misses
    this->row_buffer_hit = 0;
}
// ============================================================================
memory_controller_t::~memory_controller_t(){
    
}
// ============================================================================

// ============================================================================
// @allocate objects to EMC
void memory_controller_t::allocate(){
    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_memory_ctrl = cfg_root[0]["MEMORY_CONTROLLER"];
    set_CHANNEL (cfg_memory_ctrl["CHANNEL"]);
    set_LINE_SIZE (cfg_memory_ctrl["LINE_SIZE"]);
    set_WAIT_CYCLE (cfg_memory_ctrl["WAIT_CYCLE"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);
    
    this->channels = new memory_channel_t[CHANNEL];
    
    this->set_masks();
}
// ============================================================================
void memory_controller_t::statistics(){
    FILE *output = stdout;
    bool close = false;

    if(orcs_engine.output_file_name != NULL){
        close=true;
		output = fopen(orcs_engine.output_file_name,"a+");
    }
	if (output != NULL){
        utils_t::largestSeparator(output);
        fprintf(output,"#Memory Controller\n");
        utils_t::largestSeparator(output);
        fprintf(output,"Requests_Made:              %lu\n",this->get_requests_made());
        fprintf(output,"Requests_from_Prefetcher:   %lu\n",this->get_requests_prefetcher());
        fprintf(output,"Requests_from_LLC:          %lu\n",this->get_requests_llc());
        for (uint32_t i = 0; i < CHANNEL; i++){
            fprintf(output,"Row_Buffer_Hit, Channel %u:  %lu\n",i,this->channels[i].get_stat_row_buffer_hit());
            fprintf(output,"Row_Buffer_Miss, Channel %u: %lu\n",i,this->channels[i].get_stat_row_buffer_miss());
        }
        utils_t::largestSeparator(output);
        if(close) fclose(output);
    }
}
// ============================================================================
void memory_controller_t::clock(){
    for (uint32_t i = 0; i < this->CHANNEL; i++) this->channels[i].clock();
}
// ============================================================================
void memory_controller_t::set_masks(){
        
    ERROR_ASSERT_PRINTF(CHANNEL > 1 && utils_t::check_if_power_of_two(CHANNEL),"Wrong number of memory_channels (%u).\n",CHANNEL);
    uint64_t i;
    // =======================================================
    // Setting to zero
    // =======================================================
    this->channel_bits_shift = 0;
    this->channel_bits_mask = 0;
    // =======================================================
    this->channel_bits_shift = utils_t::get_power_of_two(LINE_SIZE);
    
    /// CHANNEL MASK
    for (i = 0; i < utils_t::get_power_of_two(CHANNEL); i++) {
        this->channel_bits_mask |= 1 << (i + this->channel_bits_shift);
    }
}
//=====================================================================
uint64_t memory_controller_t::requestDRAM (mshr_entry_t* request, uint64_t address){
    this->add_requests_made();
    memory_operation_t operation;
    if (request != NULL) {
        this->channels[get_channel (address)].addRequest (request);
        return 0;
    } else operation = MEMORY_OPERATION_READ;
    //initializes in latency burst
    uint64_t latency_request = this->channels[get_channel (address)].latencyCalc (operation, address);
    latency_request = latency_request * CORE_TO_BUS_CLOCK_RATIO;
    return latency_request;
}