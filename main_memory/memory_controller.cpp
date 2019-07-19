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
    // ======================= Configurando DRAM ======================= 
    this->latency_burst = LINE_SIZE/BURST_WIDTH;
    this->set_masks();
    this->ram = (RAM_t*)malloc(CHANNEL*BANK*sizeof(RAM_t));
    std::memset(this->ram,0,CHANNEL*BANK*sizeof(RAM_t));
    // ======================= Configurando data BUS ======================= 
    this->data_bus = new bus_t[CHANNEL];
    for(uint8_t i = 0; i < CHANNEL; i++){
        this->data_bus[i].requests.reserve(CHANNEL*NUMBER_OF_PROCESSORS*MAX_PARALLEL_REQUESTS_CORE);
    }
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
uint64_t memory_controller_t::requestDRAM(uint64_t address){
    //initializes in latency burst
    uint64_t latency_request = 0;
    //
    uint64_t channel;
    channel = this->get_channel(address);
    uint64_t bank;
    bank = (channel*BANK) + this->get_bank(address);
    // Get the row where "data" is
    uint64_t current_row = this->get_row(address);
    if (MEM_CONTROLLER_DEBUG){
        if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
            ORCS_PRINTF("Request Address %lu\n",address)
            ORCS_PRINTF("Memory Channel Accessed:%lu\n",channel)
            ORCS_PRINTF("Bank Mask %lu, Accessed: %lu\n",this->get_bank(address),bank)
            ORCS_PRINTF("Row %lu\n",current_row)
        }
    }
    // ====================================================
    // verifies if last request issued to this bank has already been served
    if( orcs_engine.get_global_cycle() >= this->ram[bank].cycle_ready){
        if (MEM_CONTROLLER_DEBUG){
            if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                ORCS_PRINTF("C1 Cycle ready %lu, last row %lu, actual row %lu\n",this->ram[bank].cycle_ready, this->ram[bank].last_row_accessed,current_row)
            }
        }
        // if current row being accessed is the same as the last row, latency is CAS
        if(current_row == this->ram[bank].last_row_accessed){
            //como está havendo um novo acesso à mesma linha que a do último, a linha já está carregada nos sense amplifiers, então não há PRECHARGE nem RAS
            //a latência a calcular agora é a de acesso a uma coluna diferente da mesma linha, CAS, e o burst de dados nos barramentos
            latency_request += CAS; 
            latency_request += this->latency_burst; //CAS + latency burst já inclui o CCD (Column-to-Column Delay)
            this->ram[bank].cycle_ready = orcs_engine.get_global_cycle()+latency_request;
            this->add_row_buffer_hit();
            if (MEM_CONTROLLER_DEBUG){
                if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                    ORCS_PRINTF("Same Row Cycle ready %lu, last row %lu, actual row %lu\n",this->ram[bank].cycle_ready, this->ram[bank].last_row_accessed,current_row)
                }
            }
        }else{
        // else, must precharge, then access new row and column
            latency_request += ROW_PRECHARGE; //precharge, preparando sense amplifiers para receber nova linha
            latency_request += RAS; //RAS = RCD (Row Command Delay, que é o tempo de carregar os dados dos arrays DRAM para os sense amplifiers) + o tempo de restaurar os dados para a células DRAM
            this->ram[bank].last_row_accessed = current_row; //atualizamos qual foi a última linha acessada neste banco
            latency_request += CAS; //o tempo de acessar a coluna em questão da linha presente nos sense amplifiers
            latency_request += latency_burst; //o tempo que os dados passarão no barramento de dados

            //no ciclo cycle_ready, o banco estará pronto para atender uma nova solicitação
            this->ram[bank].cycle_ready = orcs_engine.get_global_cycle()+latency_request;
            this->add_row_buffer_miss();
            if (MEM_CONTROLLER_DEBUG){
                if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                    ORCS_PRINTF("Diff Row Cycle ready %lu, last row %lu, actual row %lu\n",this->ram[bank].cycle_ready, this->ram[bank].last_row_accessed,current_row)
                }
            }
        }
        // if the request has not been served yet
    }else{
        // if new request is on same row
         if(current_row == this->ram[bank].last_row_accessed){
            // latency is when row is ready(completed last req) + CAS
            this->ram[bank].cycle_ready += CAS+this->latency_burst;
            latency_request += (this->ram[bank].cycle_ready-orcs_engine.get_global_cycle()); 
            this->add_row_buffer_hit();
            if (MEM_CONTROLLER_DEBUG){
                if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                    ORCS_PRINTF("Cycle ready %lu, last row %lu, actual row %lu\n",this->ram[bank].cycle_ready, this->ram[bank].last_row_accessed,current_row)
                }
            }
        }else{
            // Get time to complete all pending requests+RP+RAS+CAS
            this->ram[bank].cycle_ready +=(ROW_PRECHARGE+RAS+CAS)+this->latency_burst;
            this->ram[bank].last_row_accessed = current_row;
            latency_request += (this->ram[bank].cycle_ready-orcs_engine.get_global_cycle())+(ROW_PRECHARGE+RAS+CAS);
            this->add_row_buffer_miss();           
            if (MEM_CONTROLLER_DEBUG){
                if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
                    ORCS_PRINTF("Cycle ready %lu, last row %lu, actual row %lu\n",this->ram[bank].cycle_ready, this->ram[bank].last_row_accessed,current_row)
                }
            }
        }
    }
    if (MEM_CONTROLLER_DEBUG){
        if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
            ORCS_PRINTF("Latency Request Before %lu\n",latency_request)
        }
    }
    latency_request= this->add_channel_bus(address,latency_request);
    this->ram[bank].cycle_ready = latency_request;
    if (MEM_CONTROLLER_DEBUG){
        if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
            ORCS_PRINTF("Latency Request After %lu\n",latency_request)
        }
    }
    this->add_requests_made();
    return this->ram[bank].cycle_ready-orcs_engine.get_global_cycle();
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
    return request_start;
}
// ============================================================================
