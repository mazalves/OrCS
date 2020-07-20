#include "./../simulator.hpp"
#include <string>
// ============================================================================
memory_controller_t::memory_controller_t(){
    this->requests_made = 0; //Data Requests made
    this->operations_executed = 0; // number of operations executed
    this->requests_llc = 0; //Data Requests made to LLC
    this->requests_hive = 0;
    this->requests_vima = 0;
    this->requests_prefetcher = 0; //Data Requests made by prefetcher
    this->row_buffer_miss = 0; //Counter row buffer misses
    this->row_buffer_hit = 0;

    this->channel_bits_mask = 0;
    this->rank_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->row_bits_mask = 0;
    this->colrow_bits_mask = 0;
    this->colbyte_bits_mask = 0;
    this->not_column_bits_mask = 0;
        
    // Shifts bits
    this->channel_bits_shift = 0;
    this->colbyte_bits_shift = 0;
    this->colrow_bits_shift = 0;
    this->bank_bits_shift = 0;
    this->row_bits_shift = 0;
    this->controller_bits_shift = 0;
        
    this->CHANNEL = 0;
    this->WAIT_CYCLE = 0;
    this->LINE_SIZE = 0;
    this->DEBUG = 0;

    this->CORE_TO_BUS_CLOCK_RATIO = 0.0;
    this->TIMING_AL = 0;     // Added Latency for column accesses
    this->TIMING_CAS = 0;    // Column Access Strobe (CL) latency
    this->TIMING_CCD = 0;    // Column to Column Delay
    this->TIMING_CWD = 0;    // Column Write Delay (CWL) or simply WL
    this->TIMING_FAW = 0;   // Four (row) Activation Window
    this->TIMING_RAS = 0;   // Row Access Strobe
    this->TIMING_RC = 0;    // Row Cycle
    this->TIMING_RCD = 0;    // Row to Column comand Delay
    this->TIMING_RP = 0;     // Row Precharge
    this->TIMING_RRD = 0;    // Row activation to Row activation Delay
    this->TIMING_RTP = 0;    // Read To Precharge
    this->TIMING_WR = 0;    // Write Recovery time
    this->TIMING_WTR = 0;

    this->channels = NULL;
    this->i = 0;
}
// ============================================================================
memory_controller_t::~memory_controller_t(){
    delete[] this->channels;
}
// ============================================================================

// ============================================================================
// @allocate objects to EMC
void memory_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    
    set_DEBUG (cfg_processor["DEBUG"]);
    set_BANK (cfg_memory_ctrl["BANK"]);
    set_BANK_ROW_BUFFER_SIZE (cfg_memory_ctrl["BANK_ROW_BUFFER_SIZE"]);
    set_CHANNEL (cfg_memory_ctrl["CHANNEL"]);
    set_LINE_SIZE (cfg_memory_ctrl["LINE_SIZE"]);
    set_WAIT_CYCLE (cfg_memory_ctrl["WAIT_CYCLE"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);

    set_TIMING_AL (cfg_memory_ctrl["TIMING_AL"]);     // Added Latency for column accesses
    set_TIMING_CAS (cfg_memory_ctrl["TIMING_CAS"]);    // Column Access Strobe (CL]) latency
    set_TIMING_CCD (cfg_memory_ctrl["TIMING_CCD"]);    // Column to Column Delay
    set_TIMING_CWD (cfg_memory_ctrl["TIMING_CWD"]);    // Column Write Delay (CWL]) or simply WL
    set_TIMING_FAW (cfg_memory_ctrl["TIMING_FAW"]);   // Four (row]) Activation Window
    set_TIMING_RAS (cfg_memory_ctrl["TIMING_RAS"]);   // Row Access Strobe
    set_TIMING_RC (cfg_memory_ctrl["TIMING_RC"]);    // Row Cycle
    set_TIMING_RCD (cfg_memory_ctrl["TIMING_RCD"]);    // Row to Column comand Delay
    set_TIMING_RP (cfg_memory_ctrl["TIMING_RP"]);     // Row Precharge
    set_TIMING_RRD (cfg_memory_ctrl["TIMING_RRD"]);    // Row activation to Row activation Delay
    set_TIMING_RTP (cfg_memory_ctrl["TIMING_RTP"]);    // Read To Precharge
    set_TIMING_WR (cfg_memory_ctrl["TIMING_WR"]);    // Write Recovery time
    set_TIMING_WTR (cfg_memory_ctrl["TIMING_WTR"]);
    
    this->channels = new memory_channel_t[CHANNEL]();
    for (i = 0; i < this->CHANNEL; i++) this->channels[i].allocate();
    for (i = 0; i < CHANNEL; i++){
        channels[i].set_TIMING_AL (ceil (this->TIMING_AL * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_CAS (ceil (this->TIMING_CAS * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_CCD (ceil (this->TIMING_CCD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_CWD (ceil (this->TIMING_CWD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_FAW (ceil (this->TIMING_FAW * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RAS (ceil (this->TIMING_RAS * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RC (ceil (this->TIMING_RC * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RCD (ceil (this->TIMING_RCD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RP (ceil (this->TIMING_RP * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RRD (ceil (this->TIMING_RRD * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_RTP (ceil (this->TIMING_RTP * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_WR (ceil (this->TIMING_WR * this->CORE_TO_BUS_CLOCK_RATIO));
        channels[i].set_TIMING_WTR (ceil (this->TIMING_WTR * this->CORE_TO_BUS_CLOCK_RATIO));
    }
    
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
        fprintf(output,"Requests_from_HIVE:         %lu\n",this->get_requests_hive());
        fprintf(output,"Requests from VIMA:         %lu\n",this->get_requests_vima());
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
    for (i = 0; i < this->CHANNEL; i++) this->channels[i].clock();
}
// ============================================================================
void memory_controller_t::set_masks(){ 
    ERROR_ASSERT_PRINTF(CHANNEL > 1 && utils_t::check_if_power_of_two(CHANNEL),"Wrong number of memory_channels (%u).\n",CHANNEL);
    
    this->channel_bits_shift=0;
    this->colbyte_bits_shift=0;
    this->colrow_bits_shift=0;
    
    this->bank_bits_shift=0;
    this->row_bits_shift=0;
    this->colbyte_bits_shift = 0;

    this->channel_bits_mask = 0;
    this->bank_bits_mask = 0;
    this->rank_bits_mask = 0;
    this->row_bits_mask = 0;
    this->colrow_bits_mask = 0;
    this->colbyte_bits_mask = 0;
    // =======================================================
    this->controller_bits_shift = 0;
    this->colbyte_bits_shift = 0;
    this->colrow_bits_shift = utils_t::get_power_of_two(this->LINE_SIZE);
    this->channel_bits_shift = utils_t::get_power_of_two(this->BANK_ROW_BUFFER_SIZE);
    this->bank_bits_shift = this->channel_bits_shift + utils_t::get_power_of_two(this->CHANNEL);
    this->row_bits_shift = this->bank_bits_shift + utils_t::get_power_of_two(this->BANK);
    
    /// COLBYTE MASK
    for (i = 0; i < utils_t::get_power_of_two(this->LINE_SIZE); i++) {
        this->colbyte_bits_mask |= 1 << (i + this->colbyte_bits_shift);
    }

    /// COLROW MASK
    for (i = 0; i < utils_t::get_power_of_two(this->BANK_ROW_BUFFER_SIZE / this->LINE_SIZE); i++) {
        this->colrow_bits_mask |= 1 << (i + this->colrow_bits_shift);
    }

    this->not_column_bits_mask = ~(colbyte_bits_mask | colrow_bits_mask);

    /// CHANNEL MASK
    for (i = 0; i < utils_t::get_power_of_two(this->CHANNEL); i++) {
        this->channel_bits_mask |= 1 << (i + channel_bits_shift);
    }

    /// BANK MASK
    for (i = 0; i < utils_t::get_power_of_two(this->BANK); i++) {
        this->bank_bits_mask |= 1 << (i + bank_bits_shift);
    }

    /// ROW MASK
    for (i = row_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->row_bits_mask |= 1 << i;
    }
}
//=====================================================================
uint64_t memory_controller_t::requestDRAM (memory_package_t* request){
    this->add_requests_made();
    if (request->is_hive) this->add_requests_hive();
    if (request->is_vima) this->add_requests_vima();
    if (request != NULL) {
        request->sent_to_ram = true;
        this->channels[get_channel (request->memory_address)].addRequest (request);
        if (DEBUG) ORCS_PRINTF ("Memory Controller requestDRAM(): receiving memory request from uop %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
        return 0;
    }
    return 0;
}