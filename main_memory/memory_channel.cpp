#include "../simulator.hpp"

memory_channel_t::memory_channel_t(){
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

    this->last_bank_selected = 0;

    this->stat_row_buffer_hit = 0;
    this->stat_row_buffer_miss = 0;

    this->bank_is_ready = NULL;
    this->bank_last_row = NULL;
    this->bank_is_drain_write = NULL;
    this->bank_last_command_cycle = NULL;      /// Cycle of the Last command sent to each bank
    this->channel_last_command_cycle = NULL;      /// Cycle of the last command type

    this->data_bus_availability = 0;
    this->latency_burst = 0;
    this->i = 0;
    this->RANK = 0;
    this->BANK = 0;
    this->BANK_BUFFER_SIZE = 0;
    this->BANK_ROW_BUFFER_SIZE = 0;
    this->CHANNEL = 0;
    this->CLOSED_ROW = 0;
    this->LINE_SIZE = 0;
    this->DEBUG = 0;

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
}

memory_channel_t::~memory_channel_t(){
    delete[] this->bank_last_row;
    delete[] this->bank_last_command;
    for (i = 0; i < this->BANK; i++) delete[] this->bank_last_command_cycle[i];
    delete[] this->bank_last_command_cycle;
    delete[] this->channel_last_command_cycle;
    delete[] this->bank_is_ready;
    delete[] this->bank_is_drain_write;
    
    delete[] this->bank_read_requests;
    delete[] this->bank_write_requests;
}

void memory_channel_t::allocate() {
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    
    set_DEBUG (cfg_processor["DEBUG"]);
    set_RANK (cfg_memory_ctrl["RANK"]);
    set_BANK (cfg_memory_ctrl["BANK"]);
    set_BANK_BUFFER_SIZE (cfg_memory_ctrl["BANK_BUFFER_SIZE"]);
    set_LINE_SIZE (cfg_memory_ctrl["LINE_SIZE"]);
    set_BANK_ROW_BUFFER_SIZE (cfg_memory_ctrl["BANK_ROW_BUFFER_SIZE"]);
    set_CHANNEL (cfg_memory_ctrl["CHANNEL"]);
    set_CLOSED_ROW (cfg_memory_ctrl["CLOSED_ROW"]);
    
    if (!strcmp (cfg_memory_ctrl["REQUEST_PRIORITY"], "ROW_BUFFER_HITS_FIRST")){
        this->REQUEST_PRIORITY = REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST;
    } else if (!strcmp (cfg_memory_ctrl["REQUEST_PRIORITY"], "FIRST_COME_FIRST_SERVE")){
        this->REQUEST_PRIORITY = REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE;
    }

    if (!strcmp (cfg_memory_ctrl["WRITE_PRIORITY"], "DRAIN_WHEN_FULL")){
        this->WRITE_PRIORITY = WRITE_PRIORITY_DRAIN_WHEN_FULL;
    } else if (!strcmp (cfg_memory_ctrl["WRITE_PRIORITY"], "SERVICE_AT_NO_READ")){
        this->WRITE_PRIORITY = WRITE_PRIORITY_SERVICE_AT_NO_READ;
    }

    this->bank_last_transmission = 0;
    this->bank_is_ready = new bool[BANK]();
    this->bank_is_drain_write = new bool[BANK]();
    this->bank_last_row = new uint64_t[BANK]();
    this->bank_last_command = new memory_controller_command_t[BANK]();
    this->channel_last_command_cycle = new uint64_t[BANK]();
    this->bank_last_command_cycle = new uint64_t*[BANK]();
    for (i = 0; i < BANK; i++) this->bank_last_command_cycle[i] = new uint64_t[5]();

    this->bank_read_requests = new std::vector<memory_package_t*>[BANK]();
    this->bank_write_requests = new std::vector<memory_package_t*>[BANK]();
    this->set_masks();
}

void memory_channel_t::set_masks(){       
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
    this->latency_burst = 0;
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

bool memory_channel_t::addRequest (memory_package_t* request){
    bool result = false;
    uint64_t bank = this->get_bank(request->memory_address);
    switch (request->memory_operation){
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_INST:
            if (bank_read_requests[bank].size() < this->BANK_BUFFER_SIZE) {
                bank_read_requests[bank].push_back (request);
                result = true;
                if (DEBUG) ORCS_PRINTF ("Memory Channel addRequest(): receiving memory request from uop %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
            }
            break;
        case MEMORY_OPERATION_WRITE:
            if (bank_write_requests[bank].size() < this->BANK_BUFFER_SIZE) {
                bank_write_requests[bank].push_back (request);
                result = true;
                if (DEBUG) ORCS_PRINTF ("Memory Channel addRequest(): receiving memory request from uop %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
            }
            break;
        default:
            break;
    }
    return result;
}

memory_package_t* memory_channel_t::findNext (uint32_t bank){
    if (bank_read_requests[bank].empty() && bank_write_requests[bank].empty()) return NULL;
    
    if (this->bank_is_drain_write[bank]){
        if (bank_write_requests[bank].size() > 0) return findNextWrite (bank);
        this->bank_is_drain_write[bank] = false;
        return findNextRead (bank);
    } else {
        switch (this->WRITE_PRIORITY){
            case WRITE_PRIORITY_SERVICE_AT_NO_READ:{
                if (bank_read_requests[bank].size() > 0) return findNextRead (bank);
                if (bank_write_requests[bank].size() == this->BANK_BUFFER_SIZE){
                    this->bank_is_drain_write[bank] = true;
                }
                return findNextWrite (bank);
                break;
            }
            case WRITE_PRIORITY_DRAIN_WHEN_FULL:{ //NÃO FUNCIONA :-)
                if (bank_read_requests[bank].size() > 0) return findNextRead (bank);
                if (bank_write_requests[bank].size() == this->BANK_BUFFER_SIZE){
                    this->bank_is_drain_write[bank] = true;
                    return findNextWrite (bank);
                }
                break;
            }
        }
    }
   return NULL;
}

memory_package_t* memory_channel_t::findNextRead (uint32_t bank){
    if (bank_read_requests[bank].empty()) return NULL;
    switch (this->REQUEST_PRIORITY){
        case REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE:{
            return bank_read_requests[bank].front();
            break;
        }
        case REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST:{
            for (i = 0; i < bank_read_requests[bank].size(); i++){
                if (bank_last_row[bank] == get_row (bank_read_requests[bank][i]->memory_address)){
                    return bank_read_requests[bank][i];
                }
            }
            break;
        }
    }
    return bank_read_requests[bank].front();
}

memory_package_t* memory_channel_t::findNextWrite (uint32_t bank){
    if (bank_write_requests[bank].empty()) return NULL;
    switch (this->REQUEST_PRIORITY){
        case REQUEST_PRIORITY_FIRST_COME_FIRST_SERVE:{
            return bank_write_requests[bank].front();
            break;
        }
        case REQUEST_PRIORITY_ROW_BUFFER_HITS_FIRST:{
            for (size_t i = 0; i < bank_write_requests[bank].size(); i++){
                if (bank_last_row[bank] == get_row (bank_write_requests[bank][i]->memory_address)){
                    return bank_write_requests[bank][i];
                }
            }
            break;
        }
    }
    return bank_write_requests[bank].front();
}

void memory_channel_t::clock(){
    uint32_t bank = 0, row = 0;
    this->last_bank_selected = (this->last_bank_selected + 1) % this->BANK;
    memory_package_t* current_entry = this->findNext (this->last_bank_selected);
    if (current_entry != NULL) {
        bank = this->get_bank(current_entry->memory_address);
        row = this->get_row(current_entry->memory_address);
        
        if (!bank_is_ready[bank]){
            switch (bank_last_command[bank]){
                case MEMORY_CONTROLLER_COMMAND_NUMBER:
                    ERROR_PRINTF("Should not receive COMMAND_NUMBER\n")
                break;
                case MEMORY_CONTROLLER_COMMAND_PRECHARGE:
                    if (get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS) > orcs_engine.get_global_cycle()) break;
                    if (!current_entry->row_buffer) {
                        this->add_stat_row_buffer_miss();
                        current_entry->row_buffer = true;
                    }
                    this->bank_last_row[bank] = row;
                    this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_ROW_ACCESS;
                    this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
                    this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] = orcs_engine.get_global_cycle();
                    break;
                case MEMORY_CONTROLLER_COMMAND_ROW_ACCESS:
                case MEMORY_CONTROLLER_COMMAND_COLUMN_READ:
                case MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE:
                    if (bank_last_row[bank] == row){
                        switch (current_entry->memory_operation){
                            case MEMORY_OPERATION_INST:
                            case MEMORY_OPERATION_READ: {
                                if (get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_COLUMN_READ) > orcs_engine.get_global_cycle()) break;
                                if (!current_entry->row_buffer) {
                                    this->add_stat_row_buffer_hit();
                                    current_entry->row_buffer = true;
                                }
                                this->bank_is_ready[bank] = true;
                                break;
                            }
                            case MEMORY_OPERATION_WRITE: {
                                if (get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE) > orcs_engine.get_global_cycle()) break;
                                if (!current_entry->row_buffer) {
                                    this->add_stat_row_buffer_hit();
                                    current_entry->row_buffer = true;
                                }
                                this->bank_is_ready[bank] = true;
                                break;
                            }
                            default: break;
                        }
                    } else {
                        this->bank_last_row[bank] = row;
                        this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_PRECHARGE;
                        this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
                        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
                    }
                    break;
            }
        }
    }

    for (uint32_t i = 0; i < this->BANK; i++){
        this->bank_last_transmission++;
        if (this->bank_last_transmission >= this->BANK) this->bank_last_transmission = 0;
        if (bank_is_ready[this->bank_last_transmission]) {
            break;
        }
    }
    bank = this->bank_last_transmission;

    if (bank_is_ready[bank]){
        current_entry = this->findNext (bank);
        if (this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] > orcs_engine.get_global_cycle() ||
        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] > orcs_engine.get_global_cycle()) return;
        //ORCS_PRINTF ("%lu Request! address: %lu | row: %u | bank: %u | channel: %lu | column: %lu\n", orcs_engine.get_global_cycle(), current_entry->memory_address, row, bank, get_channel (current_entry->memory_address), get_column (current_entry->memory_address))

        switch (current_entry->memory_operation){
            case MEMORY_OPERATION_INST:
            case MEMORY_OPERATION_READ:
                this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_READ;
                this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] = orcs_engine.get_global_cycle() + this->latency_burst;
                current_entry->updatePackageDRAMReady (this->TIMING_CAS + this->latency_burst);
                if (DEBUG) ORCS_PRINTF ("%lu Memory Channel %lu requestDRAM(): bank %lu, finished memory request %lu from uop %lu, %s.\n", orcs_engine.get_global_cycle(), get_channel (current_entry->memory_address), get_bank (current_entry->memory_address), current_entry->memory_address, current_entry->uop_number, get_enum_memory_operation_char (current_entry->memory_operation))
                bank_read_requests[bank].erase(std::remove(bank_read_requests[bank].begin(), bank_read_requests[bank].end(), current_entry), bank_read_requests[bank].end());
                break;
            case MEMORY_OPERATION_WRITE:
                this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE;
                this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] = orcs_engine.get_global_cycle() + this->latency_burst;
                current_entry->updatePackageDRAMReady (this->TIMING_CWD + this->latency_burst);
                if (DEBUG) ORCS_PRINTF ("%lu Memory Channel %lu requestDRAM(): bank %lu, finished memory request %lu from uop %lu, %s.\n", orcs_engine.get_global_cycle(), get_channel (current_entry->memory_address), get_bank (current_entry->memory_address), current_entry->memory_address, current_entry->uop_number, get_enum_memory_operation_char (current_entry->memory_operation))
                bank_write_requests[bank].erase(std::remove(bank_write_requests[bank].begin(), bank_write_requests[bank].end(), current_entry), bank_write_requests[bank].end());
                break;
            default:
                break;
        }
        //if (current_entry->memory_operation != MEMORY_OPERATION_INST) ORCS_PRINTF ("%lu request will be ready, channel %lu, bank %lu\n", current_entry->readyAt, get_channel (current_entry->memory_address), get_bank (current_entry->memory_address))

        bank_is_ready[bank] = false;
        if (this->get_CLOSED_ROW()) {
            /// Select package to be treated
            memory_package_t* next_entry = findNext(bank);
            if (next_entry == NULL) {
                uint64_t latency_ready_cycle = get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS);
                // Row precharge code here!!!!
                this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_ROW_ACCESS;
                this->bank_last_row[bank] = POSITION_FAIL;
                this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] = latency_ready_cycle;
                this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_PRECHARGE] = latency_ready_cycle;
            }
            else {
                uint32_t next_row = get_row(next_entry->memory_address);
                next_entry->row_buffer = true;
                // If next package == row buffer miss --->>> RowPrecharge
                if (this->bank_last_row[bank] != next_row) {
                    uint64_t latency_ready_cycle = get_minimum_latency(bank, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS);
                    this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_PRECHARGE;
                    this->bank_last_row[bank] = next_row;
                    this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] = latency_ready_cycle;
                    this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_PRECHARGE] = latency_ready_cycle;
                    this->add_stat_row_buffer_miss();
                } else this->add_stat_row_buffer_hit();
            }
        }
    }
}

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
            /// Obtain the 4th newer RAS+FAW command among the banks.
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
