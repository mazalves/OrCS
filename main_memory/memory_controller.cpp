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
    free (this->ram);
    delete[] data_bus;
}
// ============================================================================

// ============================================================================
// @allocate objects to EMC
void memory_controller_t::allocate(){
    LINE_SIZE = orcs_engine.configuration->getSetting("LINE_SIZE");

     CHANNEL = orcs_engine.configuration->getSetting("CHANNEL");
     RANK = orcs_engine.configuration->getSetting("RANK");
     BANK = orcs_engine.configuration->getSetting("BANK");
     ROW_BUFFER = (RANK*BANK)*1024;
    // =====================Parametes Comandd=======================
     BURST_WIDTH = orcs_engine.configuration->getSetting("BURST_WIDTH");
     RAS = orcs_engine.configuration->getSetting("RAS");
     CAS = orcs_engine.configuration->getSetting("CAS");
     ROW_PRECHARGE = orcs_engine.configuration->getSetting("ROW_PRECHARGE");
    // ============================================

    //uint64_t RAM_SIZE = 4 * MEGA * KILO;
     PARALLEL_LIM_ACTIVE = orcs_engine.configuration->getSetting("PARALLEL_LIM_ACTIVE");
     MAX_PARALLEL_REQUESTS_CORE = orcs_engine.configuration->getSetting("MAX_PARALLEL_REQUESTS_CORE");
     MEM_CONTROLLER_DEBUG = orcs_engine.configuration->getSetting("MEM_CONTROLLER_DEBUG");
     WAIT_CYCLE = orcs_engine.configuration->getSetting("WAIT_CYCLE");

    TIMING_AL = orcs_engine.configuration->getSetting("TIMING_AL");     // Added Latency for column accesses
    TIMING_CAS = orcs_engine.configuration->getSetting("TIMING_CAS");    // Column Access Strobe (CL) latency
    TIMING_CCD = orcs_engine.configuration->getSetting("TIMING_CCD");    // Column to Column Delay
    TIMING_CWD = orcs_engine.configuration->getSetting("TIMING_CWD");    // Column Write Delay (CWL) or simply WL
    TIMING_FAW = orcs_engine.configuration->getSetting("TIMING_FAW");   // Four (row) Activation Window
    TIMING_RAS = orcs_engine.configuration->getSetting("TIMING_RAS");   // Row Access Strobe
    TIMING_RC = orcs_engine.configuration->getSetting("TIMING_RC");    // Row Cycle
    TIMING_RCD = orcs_engine.configuration->getSetting("TIMING_RCD");    // Row to Column comand Delay
    TIMING_RP = orcs_engine.configuration->getSetting("TIMING_RP");     // Row Precharge
    TIMING_RRD = orcs_engine.configuration->getSetting("TIMING_RRD");    // Row activation to Row activation Delay
    TIMING_RTP = orcs_engine.configuration->getSetting("TIMING_RTP");    // Read To Precharge
    TIMING_WR = orcs_engine.configuration->getSetting("TIMING_WR");    // Write Recovery time
    TIMING_WTR = orcs_engine.configuration->getSetting("TIMING_WTR");
    // ======================= Configurando DRAM ======================= 
    this->latency_burst = LINE_SIZE/BURST_WIDTH;
    this->set_masks();
    this->ram = (RAM_t**) malloc (CHANNEL*sizeof (RAM_t*));
    std::memset (this->ram,0,CHANNEL*sizeof (RAM_t*));
    for (uint32_t i = 0; i < CHANNEL; i++){
        this->ram[i] = (RAM_t*) malloc (BANK*sizeof (RAM_t));
        std::memset(this->ram[i],0,BANK*sizeof(RAM_t));
    }
    // ======================= Configurando data BUS ======================= 
    this->data_bus = new bus_t[CHANNEL];
    for(uint8_t i = 0; i < CHANNEL; i++){
        this->data_bus[i].requests.reserve(CHANNEL*NUMBER_OF_PROCESSORS*MAX_PARALLEL_REQUESTS_CORE);
    }

    this->bank_last_command = utils_t::template_allocate_initialize_array<memory_controller_command_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS);
    this->bank_last_command_cycle = utils_t::template_allocate_initialize_matrix<uint64_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_NUMBER, 0);
    this->channel_last_command_cycle = utils_t::template_allocate_initialize_array<uint64_t>(MEMORY_CONTROLLER_COMMAND_NUMBER, 0);
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
        fprintf(output,"Row_Buffer_Hit:             %lu\n",this->get_row_buffer_hit());
        fprintf(output,"Row_Buffer_Miss:            %lu\n",this->get_row_buffer_miss());
        utils_t::largestSeparator(output);
        if(close) fclose(output);
    }
}
// ============================================================================
void memory_controller_t::clock(){
    
}
// ============================================================================
void memory_controller_t::set_masks(){
        
    ERROR_ASSERT_PRINTF(CHANNEL > 1 && utils_t::check_if_power_of_two(CHANNEL),"Wrong number of memory_channels (%u).\n",CHANNEL);
    uint64_t i;
    // =======================================================
    // Setting to zero
    // =======================================================
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
    this->col_row_bits_mask = 0;
    this->col_byte_bits_mask = 0;
    this->latency_burst = 0;
    // =======================================================
    this->channel_bits_shift = utils_t::get_power_of_two(LINE_SIZE);
    this->bank_bits_shift = this->channel_bits_shift+  utils_t::get_power_of_two(CHANNEL);
    this->colrow_bits_shift = this->bank_bits_shift +utils_t::get_power_of_two(BANK);
    this->row_bits_shift = this->colrow_bits_shift + utils_t::get_power_of_two(ROW_BUFFER / LINE_SIZE);

    /// COLBYTE MASK
    for (i = 0; i < utils_t::get_power_of_two(LINE_SIZE); i++) {
        this->col_byte_bits_mask |= 1 << (i + this->colbyte_bits_shift);
    }

    /// CHANNEL MASK
    for (i = 0; i < utils_t::get_power_of_two(CHANNEL); i++) {
        this->channel_bits_mask |= 1 << (i + this->channel_bits_shift);
    }
    /// BANK MASK
    for (i = 0; i < utils_t::get_power_of_two(BANK); i++) {
        this->bank_bits_mask |= 1 << (i + this->bank_bits_shift);
    }
    /// COLROW MASK
    for (i = 0; i < utils_t::get_power_of_two((ROW_BUFFER / LINE_SIZE)); i++) {
        this->col_row_bits_mask |= 1 << (i + this->colrow_bits_shift);
    }
    /// ROW MASK
    for (i = row_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->row_bits_mask |= 1 << i;
    }

    if (MEM_CONTROLLER_DEBUG){
            ORCS_PRINTF("ColByte Shitf %03lu -> ColByte Mask %07lu - %s\n",this->colbyte_bits_shift,this->col_byte_bits_mask,utils_t::address_to_binary(this->col_byte_bits_mask).c_str())
            ORCS_PRINTF("Channel Shift %03lu -> Channel Mask %07lu - %s\n",this->channel_bits_shift,this->channel_bits_mask,utils_t::address_to_binary(this->channel_bits_mask).c_str())
            ORCS_PRINTF("Bank Shift    %03lu -> Bank Mask    %07lu - %s\n",this->bank_bits_shift,this->bank_bits_mask,utils_t::address_to_binary(this->bank_bits_mask).c_str())
            ORCS_PRINTF("ColRow Shift  %03lu -> ColRow Mask  %07lu - %s\n",this->colrow_bits_shift,this->col_row_bits_mask,utils_t::address_to_binary(this->col_row_bits_mask).c_str())
            ORCS_PRINTF("Row Shift     %03lu -> Row Mask     %07lu - %s\n",this->row_bits_shift,this->row_bits_mask,utils_t::address_to_binary(this->row_bits_mask).c_str())
    }
}
// ============================================================================
uint64_t memory_controller_t::latencyCalc (memory_operation_t op, uint64_t address){
    uint64_t channel = this->get_channel(address);
    uint64_t bank = this->get_bank(address);
    uint64_t current_row = this->get_row(address);
    uint64_t latency_request = 0;

    if (this->ram[channel][bank].last_row_accessed != current_row){
        this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_PRECHARGE;
        this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
        this->add_row_buffer_miss();
    } else this->add_row_buffer_hit();

    this->ram[channel][bank].last_row_accessed = current_row;

    if (bank_last_command[bank] == MEMORY_CONTROLLER_COMMAND_PRECHARGE){
        this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_ROW_ACCESS;
        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
        this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
        latency_request = get_minimum_latency (bank, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS);
    } else if (bank_last_command[bank] == MEMORY_CONTROLLER_COMMAND_ROW_ACCESS ||
        bank_last_command[bank] == MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE ||
        bank_last_command[bank] == MEMORY_CONTROLLER_COMMAND_COLUMN_READ){
            switch (op){
                case MEMORY_OPERATION_READ: {
                    this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_READ;
                    this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                    this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                    latency_request = get_minimum_latency (bank, MEMORY_CONTROLLER_COMMAND_COLUMN_READ);
                    break;
                }
                case MEMORY_OPERATION_WRITE: {
                    this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE;
                    this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                        latency_request = get_minimum_latency (bank, MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE);
                    break;
                }
                case MEMORY_OPERATION_FREE: {
                    break;
                }
            }
    }
    return latency_request;
}
//=====================================================================
uint64_t memory_controller_t::requestDRAM (mshr_entry_t* request, uint64_t address){
    memory_operation_t operation;
    if (request != NULL) {
        address = request->requests[0]->memory_address;
        operation = request->requests[0]->memory_operation;
    } else operation = MEMORY_OPERATION_READ;
    //initializes in latency burst
    uint64_t latency_request = latencyCalc (operation, address);
    latency_request = latency_request * CORE_TO_BUS_CLOCK_RATIO;
    latency_request = this->add_channel_bus(address,latency_request);
    this->ram[get_channel(address)][get_bank(address)].cycle_ready = latency_request;
    if (MEM_CONTROLLER_DEBUG){
        if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
            ORCS_PRINTF("Latency Request After %lu\n",latency_request)
        }
    }
    this->add_requests_made();
    latency_request = latency_request-orcs_engine.get_global_cycle();
    if (request != NULL) {
        request->latency += (latency_request);
    }
    return latency_request;
}

uint64_t memory_controller_t::get_minimum_latency(uint32_t bank, memory_controller_command_t next_command) {
    uint64_t a = 0;
    uint64_t b = 0;
    uint64_t c = 0;
    uint64_t d = 0;

    switch (next_command){
        case MEMORY_CONTROLLER_COMMAND_PRECHARGE:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RAS;
            b = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_AL + this->TIMING_RTP - this->TIMING_CCD; // + this->timing_burst; // tBurst will be controlled separated
            c = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_AL + this->TIMING_CWD + this->TIMING_WR; // + this->timing_burst; // tBurst will be controlled separated
            if (a > b && a > c) return this->TIMING_RAS;
            if (b > a && b > c) return this->TIMING_AL + this->TIMING_RTP - this->TIMING_CCD;
            if (c > a && c > b) return this->TIMING_AL + this->TIMING_CWD + this->TIMING_WR;
        break;

        case MEMORY_CONTROLLER_COMMAND_ROW_ACCESS:
        {
            /// Obtain the 4th newer RAS+FAW command amoung the banks.
            uint64_t last_ras = 0;
            for (uint32_t i = 0; i < this->BANK; i++) {
                last_ras = this->bank_last_command_cycle[i][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_FAW;
                if ((a < last_ras) && (d = c) && (c = b) && (b=a) && (a=last_ras)) continue;
                if ((b < last_ras) && (d = c) && (c = b) && (b=last_ras)) continue;
                if ((c < last_ras) && (d = c) && (c = last_ras)) continue;
                if ((d < last_ras) && (d = last_ras)) continue;
            }
            /// 4th RAS + FAW window
            d += this->TIMING_FAW;

            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] + this->TIMING_RP;
            b = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RC;
            c = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RRD;
            if (a > b && a > c && a > d) return this->TIMING_RP;
            if (b > a && b > c && b > d) return this->TIMING_RC;
            if (c > a && c > b && c > d) return this->TIMING_RRD;
            if (d > a && d > b && d > c) return this->TIMING_FAW;
        }
        break;

        case MEMORY_CONTROLLER_COMMAND_COLUMN_READ:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RCD - this->TIMING_AL;
            b = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ];// + this->timing_burst; // tBurst will be controlled separated
            c = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_CCD;
            d = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_CWD + this->TIMING_WTR;// + this->timing_burst; // tBurst will be controlled separated
            if (a > b && a > c && a > d) return this->TIMING_RCD - this->TIMING_AL;
            if (b > a && b > c && b > d) return 0;
            if (c > a && c > b && c > d) return this->TIMING_CCD;
            if (d > a && d > b && d > c) return this->TIMING_CWD + this->TIMING_WTR;
        break;

        case MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RCD - this->TIMING_AL;
            b = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_CAS;// + this->timing_burst; // tBurst will be controlled separated
            c = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE];// + this->timing_burst; // tBurst will be controlled separated
            d = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_CCD;
            if (a > b && a > c && a > d) return this->TIMING_RCD - this->TIMING_AL;
            if (b > a && b > c && b > d) return this->TIMING_CAS;
            if (c > a && c > b && c > d) return 0;
            if (d > a && d > b && d > c) return this->TIMING_CCD;
        break;

        case MEMORY_CONTROLLER_COMMAND_NUMBER:
            ERROR_PRINTF("Should not receive COMMAND_NUMBER\n")
        break;
    }

    /// Obtain the maximum value to be respected
    return 0;
}
// ============================================================================

uint64_t memory_controller_t::add_channel_bus(uint64_t address,uint64_t ready){
    uint64_t request_start = orcs_engine.get_global_cycle()+ready;
    uint64_t channel = this->get_channel(address);
    uint32_t i=0,pos=0;
    size_t end = 0;
    if (MEM_CONTROLLER_DEBUG){
        ORCS_PRINTF("Channel %lu Bus Channel Size =  %lu\n",channel,this->data_bus[channel].requests.size())
        ORCS_PRINTF("Request Start %lu\n",request_start)
    }
    end = this->data_bus[channel].requests.size();
    for(i = 0; i < end; i++){
        if(request_start < this->data_bus[channel].requests[i].request_end ){
            if(request_start+this->latency_burst < this->data_bus[channel].requests[i].request_start){
                break;
            }
            request_start = this->data_bus[channel].requests[i].request_end+1;
        }else{
        }
    }
    pos=i;
    if (MEM_CONTROLLER_DEBUG){
        ORCS_PRINTF("Position acquired %u \n",pos)
    }
        
        this->data_bus[channel].requests.insert(this->data_bus[channel].requests.begin()+pos,{request_start,request_start+this->latency_burst});

    if (MEM_CONTROLLER_DEBUG){
        ORCS_PRINTF("\n\nInserted at %u position\n",i)
        for(i = 0; i < this->data_bus[channel].requests.size(); i++){
            ORCS_PRINTF("Start At: %lu End At %lu\n",this->data_bus[channel].requests[i].request_start,this->data_bus[channel].requests[i].request_end)
        }   
    }

    if( (this->data_bus[channel].requests.size()>0) &&
        (this->data_bus[channel].requests[0].request_end <= orcs_engine.get_global_cycle())){
        if (MEM_CONTROLLER_DEBUG){
            ORCS_PRINTF("\n\nDeleted from channel %lu request %lu \n",channel,this->data_bus[channel].requests[0].request_end)
        
        }
        this->data_bus[channel].requests.erase(this->data_bus[channel].requests.begin());
    }
    return request_start+this->latency_burst;
}
// ============================================================================
