#include "../simulator.hpp"

memory_channel_t::memory_channel_t(){
    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();

    set_RANK (cfg_root[0]["RANK"]);
    set_BANK (cfg_root[0]["BANK"]);
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

    this->bank_last_row = utils_t::template_allocate_initialize_array<uint64_t>(this->BANK, 0);
    this->bank_last_command = utils_t::template_allocate_initialize_array<memory_controller_command_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS);
    this->bank_last_command_cycle = utils_t::template_allocate_initialize_matrix<uint64_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_NUMBER, 0);
    this->channel_last_command_cycle = utils_t::template_allocate_initialize_array<uint64_t>(MEMORY_CONTROLLER_COMMAND_NUMBER, 0);

    this->set_masks();
}

memory_channel_t::~memory_channel_t(){
    this->bank_last_row = utils_t::template_allocate_initialize_array<uint64_t>(this->BANK, 0);
    this->bank_last_command = utils_t::template_allocate_initialize_array<memory_controller_command_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_ROW_ACCESS);
    this->bank_last_command_cycle = utils_t::template_allocate_initialize_matrix<uint64_t>(this->BANK, MEMORY_CONTROLLER_COMMAND_NUMBER, 0);
    this->channel_last_command_cycle = utils_t::template_allocate_initialize_array<uint64_t>(MEMORY_CONTROLLER_COMMAND_NUMBER, 0);
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

uint64_t memory_channel_t::latencyCalc (memory_operation_t op, uint64_t address){
    uint64_t bank = this->get_bank(address);
    uint64_t row = this->get_row(address);
    uint64_t latency_request = 0;

    if (this->bank_last_row[bank] != row){
        this->bank_last_command[bank] = MEMORY_CONTROLLER_COMMAND_PRECHARGE;
        this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
        this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_PRECHARGE] = orcs_engine.get_global_cycle();
        this->add_stat_row_buffer_miss();
    } else this->add_stat_row_buffer_hit();

    this->bank_last_row[bank] = row;

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

uint64_t memory_channel_t::get_minimum_latency(uint32_t bank, memory_controller_command_t next_command) {
    uint64_t a = 0;
    uint64_t b = 0;
    uint64_t c = 0;
    uint64_t d = 0;

    switch (next_command){
        case MEMORY_CONTROLLER_COMMAND_PRECHARGE:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RAS;
            b = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_AL + this->TIMING_RTP - this->TIMING_CCD;
            c = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_AL + this->TIMING_CWD + this->TIMING_WR;
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
            b = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->latency_burst;
            c = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_CCD;
            d = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->TIMING_CWD + this->TIMING_WTR + this->latency_burst;
            if (a > b && a > c && a > d) return this->TIMING_RCD - this->TIMING_AL;
            if (b > a && b > c && b > d) return 0;
            if (c > a && c > b && c > d) return this->TIMING_CCD;
            if (d > a && d > b && d > c) return this->TIMING_CWD + this->TIMING_WTR;
        break;

        case MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE:
            a = this->bank_last_command_cycle[bank][MEMORY_CONTROLLER_COMMAND_ROW_ACCESS] + this->TIMING_RCD - this->TIMING_AL;
            b = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_READ] + this->TIMING_CAS + this->latency_burst;
            c = this->channel_last_command_cycle[MEMORY_CONTROLLER_COMMAND_COLUMN_WRITE] + this->latency_burst;
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
//=====================================================================