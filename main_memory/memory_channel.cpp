#include "../simulator.hpp"

memory_channel_t::memory_channel_t(){
    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();

    set_RANK (cfg_root[0]["RANK"]);
    set_BANK (cfg_root[0]["BANK"]);
    set_BANK (cfg_root[0]["BANK_BUFFER_SIZE"]);
    set_CHANNEL (cfg_root[0]["CHANNEL"]);
    set_LINE_SIZE (cfg_root[0]["LINE_SIZE"]);
    set_BURST_WIDTH (cfg_root[0]["BURST_WIDTH"]);
    set_ROW_BUFFER ((RANK*BANK)*1024);
    // =====================Parametes Comandd=======================
    set_TIMING_AL (cfg_root[0]["TIMING_AL"]);     // Added Latency for column accesses
    set_TIMING_CAS (cfg_root[0]["TIMING_CAS"]);    // Column Access Strobe (CL]) latency
    set_TIMING_CCD (cfg_root[0]["TIMING_CCD"]);    // Column to Column Delay
    set_TIMING_CWD (cfg_root[0]["TIMING_CWD"]);    // Column Write Delay (CWL]) or simply WL
    set_TIMING_FAW (cfg_root[0]["TIMING_FAW"]);   // Four (row]) Activation Window
    set_TIMING_RAS (cfg_root[0]["TIMING_RAS"]);   // Row Access Strobe
    set_TIMING_RC (cfg_root[0]["TIMING_RC"]);    // Row Cycle
    set_TIMING_RCD (cfg_root[0]["TIMING_RCD"]);    // Row to Column comand Delay
    set_TIMING_RP (cfg_root[0]["TIMING_RP"]);     // Row Precharge
    set_TIMING_RRD (cfg_root[0]["TIMING_RRD"]);    // Row activation to Row activation Delay
    set_TIMING_RTP (cfg_root[0]["TIMING_RTP"]);    // Read To Precharge
    set_TIMING_WR (cfg_root[0]["TIMING_WR"]);    // Write Recovery time
    set_TIMING_WTR (cfg_root[0]["TIMING_WTR"]);

    set_latency_burst (LINE_SIZE/BURST_WIDTH);

    this->bank_is_ready = utils_t::template_allocate_initialize_array<bool>(this->BANK, 0);
    this->bank_last_row = utils_t::template_allocate_initialize_array<uint64_t>(this->BANK, 0);
    this->bank_last_command = utils_t::template_allocate_initialize_array<memory_controller_command_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS);
    this->bank_last_command_cycle = utils_t::template_allocate_initialize_matrix<uint64_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_NUMBER, 0);
    this->channel_last_command_cycle = utils_t::template_allocate_initialize_array<uint64_t>(MEMORY_CONTROLLER_COMMAND_NUMBER, 0);

    this->bank_requests = (std::vector<mshr_entry_t*>*) malloc (this->BANK*sizeof (std::vector<mshr_entry_t*>));

    this->set_masks();
}

memory_channel_t::~memory_channel_t(){
    utils_t::template_delete_array<uint64_t>(this->bank_last_row);
    utils_t::template_delete_array<memory_controller_command_t>(this->bank_last_command);
    utils_t::template_delete_matrix<uint64_t>(this->bank_last_command_cycle, this->BANK);
    utils_t::template_delete_array<uint64_t>(this->channel_last_command_cycle);
}

void memory_channel_t::set_masks(){       
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
}

void memory_channel_t::addRequest (mshr_entry_t* request){
    uint64_t bank = this->get_bank(request->requests[0]->memory_address);
    if(std::find(bank_requests[bank].begin(), bank_requests[bank].end(), request) == bank_requests[bank].end()) {
        bank_requests[bank].push_back (request);
    }
}

void memory_channel_t::clock(){
    this->last_bank_selected = (this->last_bank_selected + 1) % this->BANK;
    if (bank_requests[last_bank_selected].size() == 0) return;
    mshr_entry_t* current_entry = bank_requests[last_bank_selected].front();
    
    uint64_t bank = this->get_bank(current_entry->requests[0]->memory_address);
    uint64_t row = this->get_row(current_entry->requests[0]->memory_address);

    if (!bank_is_ready[bank]){
        switch (bank_last_command[bank]){
            case MEMORY_CONTROLLER_COMMAND_NUMBER:
                ERROR_PRINTF("Should not receive COMMAND_NUMBER\n")
            break;
            case MEMORY_CONTROLLER_COMMAND_PRECHARGE:{
                if (get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS) > orcs_engine.get_global_cycle()) break;
                this->add_stat_row_buffer_miss();
                this->bank_last_row[bank] = row;
                this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_ROW_ACCESS;
                this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
                this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
                break;
            }
            case MEMORY_CONTROLLER_COMMAND_ROW_ACCESS:
            case MEMORY_CONTROLLER_COMMAND_COLUMN_READ:
            case MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE:{
                if (bank_last_row[bank] == row){
                    switch (current_entry->requests[0]->memory_operation){
                        case MEMORY_OPERATION_READ: {
                            if (get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_COLUMN_READ) > orcs_engine.get_global_cycle()) break;
                            this->add_stat_row_buffer_hit();
                            this->bank_is_ready[bank] = true;
                            break;
                        }
                        case MEMORY_OPERATION_WRITE: {
                            if (get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE) > orcs_engine.get_global_cycle()) break;
                            this->add_stat_row_buffer_hit();
                            this->bank_is_ready[bank] = true;
                            break;
                        }
                        case MEMORY_OPERATION_FREE: {
                            break;
                        }
                    }
                } else {
                    this->bank_last_row[bank] = row;
                    this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_PRECHARGE;
                    this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
                    this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
                }
            }
        }
    }

    if (bank_is_ready[bank]){
        if (this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] > orcs_engine.get_global_cycle() ||
        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] > orcs_engine.get_global_cycle()) return;

        switch (current_entry->requests[0]->memory_operation){
            case MEMORY_OPERATION_READ:{
                this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_READ;
                this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                current_entry->latency += this->TIMING_CAS + this->latency_burst;
                current_entry->valid = true;
                break;
            }
            case MEMORY_OPERATION_WRITE:{
                this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE;
                this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                current_entry->latency += this->TIMING_CWD + this->latency_burst;
                current_entry->valid = true;
                break;
            }
            case MEMORY_OPERATION_FREE:{
                break;
            }
        }
        bank_requests[bank].erase(std::remove(bank_requests[bank].begin(), bank_requests[bank].end(), current_entry), bank_requests[bank].end());
        bank_is_ready[bank] = false;
    }
}

uint64_t memory_channel_t::latencyCalc (memory_operation_t op, uint64_t address){
    uint64_t bank = this->get_bank(address);
    uint64_t row = this->get_row(address);
    uint64_t latency_request = 0;
    switch (bank_last_command[bank]){
        case MEMORY_CONTROLLER_COMMAND_NUMBER:
            ERROR_PRINTF("Should not receive COMMAND_NUMBER\n")
        break;
        case MEMORY_CONTROLLER_COMMAND_PRECHARGE:{
            //ERROR_ASSERT_PRINTF (row != bank_last_row[bank], "Sending ROW_ACCESS to wrong row.")
            this->add_stat_row_buffer_miss();
            this->bank_last_row[bank] = row;
            this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_ROW_ACCESS;
            this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
            this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
            latency_request = latencyCalc (op, address);
            break;
        }
        case MEMORY_CONTROLLER_COMMAND_ROW_ACCESS:
        case MEMORY_CONTROLLER_COMMAND_COLUMN_READ:
        case MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE:{
            if (bank_last_row[bank] == row){
                switch (op){
                    case MEMORY_OPERATION_READ: {
                        this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_READ;
                        this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                        latency_request = get_minimum_latency (bank, MEMORY_CONTROLLER_COMMAND_COLUMN_READ);
                        this->add_stat_row_buffer_hit();
                        break;
                    }
                    case MEMORY_OPERATION_WRITE: {
                        this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE;
                        this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                        latency_request = get_minimum_latency (bank, MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE);
                        this->add_stat_row_buffer_hit();
                        break;
                    }
                    case MEMORY_OPERATION_FREE: {
                        break;
                    }
                }
            } else {
                this->bank_last_row[bank] = row;
                this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_PRECHARGE;
                this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
                this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
                latency_request = get_minimum_latency (bank, MEMORY_CONTROLLER_COMMAND_PRECHARGE);
            }
        }
    }
    return latency_request-orcs_engine.get_global_cycle();
}
//=====================================================================

uint64_t memory_channel_t::get_minimum_latency(uint32_t bank, memory_controller_command_t next_command) {
    uint64_t max_cycle = 0;
    uint64_t a = 0;
    uint64_t b = 0;
    uint64_t c = 0;
    uint64_t d = 0;

    switch (next_command){
        case MEMORY_CONTROLLER_COMMAND_PRECHARGE:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RAS;
            b = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_AL + this->TIMING_RTP - this->TIMING_CCD;
            c = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_AL + this->TIMING_CWD + this->TIMING_WR;
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
        }
        break;

        case MEMORY_CONTROLLER_COMMAND_COLUMN_READ:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RCD - this->TIMING_AL;
            b = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->latency_burst;
            c = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_CCD;
            d = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_CWD + this->TIMING_WTR + this->latency_burst;
        break;

        case MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RCD - this->TIMING_AL;
            b = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_CAS + this->latency_burst;
            c = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->latency_burst;
            d = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_CCD;
        break;

        case MEMORY_CONTROLLER_COMMAND_NUMBER:
            ERROR_PRINTF("Should not receive COMMAND_NUMBER\n")
        break;
    }

    /// Obtain the maximum value to be respected
    max_cycle = a;
    (max_cycle < b) && (max_cycle = b);
    (max_cycle < c) && (max_cycle = c);
    (max_cycle < d) && (max_cycle = d);
    return max_cycle;
}
//=====================================================================