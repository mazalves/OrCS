#include "./../simulator.hpp"
#include <string>

vima_controller_t::vima_controller_t(){

}

vima_controller_t::~vima_controller_t(){
    delete this->cache;
}

void vima_controller_t::print_vima_instructions(){
    ORCS_PRINTF ("=======VIMA INSTRUCTIONS=========\n")
    for (size_t i = 0; i < vima_instructions.size(); i++){
        ORCS_PRINTF ("%lu %s %lu %s\n", vima_instructions[i]->uop_number, get_enum_memory_operation_char (vima_instructions[i]->memory_operation), vima_instructions[i]->readyAt, get_enum_package_state_char (vima_instructions[i]->status))
    }
    ORCS_PRINTF ("=================================\n")
}

void vima_controller_t::instruction_ready (size_t index){
    vima_instructions[index]->status = PACKAGE_STATE_READY;
    vima_instructions[index]->readyAt = orcs_engine.get_global_cycle();
    /*if (VIMA_DEBUG) {
        ORCS_PRINTF ("VIMA Controller instruction_ready(): instruction %lu, %s ready at cycle %lu.\n", vima_instructions[index]->uop_number, get_enum_memory_operation_char (vima_instructions[index]->memory_operation), vima_instructions[index]->readyAt)
    }*/
    vima_instructions.erase (std::remove (vima_instructions.begin(), vima_instructions.end(), vima_instructions[index]), vima_instructions.end());

    this->reset_sub_requests (index + this->vima_buffer_start);
}

void vima_controller_t::reset_sub_requests(size_t index){
    for (size_t j = 0; j < 3; j++){
        vima_sub_requests[index][j].memory_address = 0;
        vima_sub_requests[index][j].status = PACKAGE_STATE_FREE;
        vima_sub_requests[index][j].memory_operation = MEMORY_OPERATION_FREE;
        vima_sub_requests[index][j].sent_to_ram = false;
    }
    
    //if (VIMA_DEBUG) ORCS_PRINTF ("VIMA Controller reset_sub_requests(): resetting sub-requests.\n")
}

void vima_controller_t::set_sub_requests (memory_package_t* request){
    for (size_t i = 0; i < 3; i++){
        vima_sub_requests[vima_buffer_end][i].uop_number = request->uop_number;
        if (i == 0){
            vima_sub_requests[vima_buffer_end][i].memory_operation = MEMORY_OPERATION_READ;
            vima_sub_requests[vima_buffer_end][i].memory_address = request->vima_read1;
        } else if (i == 1){
            vima_sub_requests[vima_buffer_end][i].memory_operation = MEMORY_OPERATION_READ;
            vima_sub_requests[vima_buffer_end][i].memory_address = request->vima_read2;
        } else {
            vima_sub_requests[vima_buffer_end][i].memory_operation = MEMORY_OPERATION_WRITE;
            vima_sub_requests[vima_buffer_end][i].memory_address = request->vima_write;
        }
        vima_sub_requests[vima_buffer_end][i].status = PACKAGE_STATE_TRANSMIT;
        vima_sub_requests[vima_buffer_end][i].sent_to_ram = false;
        vima_sub_requests[vima_buffer_end][i].is_vima = true;
        vima_sub_requests[vima_buffer_end][i].readyAt = orcs_engine.get_global_cycle();
        //vima_sub_requests[vima_buffer_end][i].printPackage();
    }

    //if (VIMA_DEBUG) ORCS_PRINTF ("VIMA Controller set_sub_requests(): setting sub-requests, instruction %lu, %s, %lu.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation), request->memory_address)
}

void vima_controller_t::clock(){
    if (vima_instructions.size() <= 0) return;

    if (vima_sub_requests[this->vima_buffer_start][0].status != PACKAGE_STATE_READY) {
        if (vima_sub_requests[this->vima_buffer_start][0].status == PACKAGE_STATE_TRANSMIT && !vima_sub_requests[this->vima_buffer_start][0].sent_to_ram){
            if (this->cache->searchAddress (vima_sub_requests[this->vima_buffer_start][0].memory_address)){
                vima_sub_requests[this->vima_buffer_start][0].status = PACKAGE_STATE_READY;
                vima_sub_requests[this->vima_buffer_start][0].readyAt = orcs_engine.get_global_cycle() + this->cache->get_VIMA_CACHE_LATENCY();
            }
            else {
                orcs_engine.memory_controller->requestDRAM (&vima_sub_requests[this->vima_buffer_start][0], vima_sub_requests[this->vima_buffer_start][0].memory_address);
                vima_sub_requests[this->vima_buffer_start][0].sent_to_ram = true;
            }
        }
    } else if (vima_sub_requests[this->vima_buffer_start][0].status == PACKAGE_STATE_READY && vima_sub_requests[this->vima_buffer_start][0].sent_to_ram){
        this->cache->installLine (vima_sub_requests[this->vima_buffer_start][0].memory_address);
        vima_sub_requests[this->vima_buffer_start][0].sent_to_ram = false;
    }
    if (vima_sub_requests[this->vima_buffer_start][1].status != PACKAGE_STATE_READY) {
        if (vima_sub_requests[this->vima_buffer_start][1].status == PACKAGE_STATE_TRANSMIT && !vima_sub_requests[this->vima_buffer_start][1].sent_to_ram){
            if (this->cache->searchAddress (vima_sub_requests[this->vima_buffer_start][1].memory_address)){
                vima_sub_requests[this->vima_buffer_start][1].status = PACKAGE_STATE_READY;
                vima_sub_requests[this->vima_buffer_start][1].readyAt = orcs_engine.get_global_cycle() + this->cache->get_VIMA_CACHE_LATENCY();
            }
            else {
                orcs_engine.memory_controller->requestDRAM (&vima_sub_requests[this->vima_buffer_start][1], vima_sub_requests[this->vima_buffer_start][1].memory_address);
                vima_sub_requests[this->vima_buffer_start][1].sent_to_ram = true;
            }
        }
        return;
    } else if (vima_sub_requests[this->vima_buffer_start][1].status == PACKAGE_STATE_READY && vima_sub_requests[this->vima_buffer_start][1].sent_to_ram){
        this->cache->installLine (vima_sub_requests[this->vima_buffer_start][1].memory_address);
        vima_sub_requests[this->vima_buffer_start][1].sent_to_ram = false;
    }

    vima_instructions[0]->readyAt = orcs_engine.get_global_cycle() + vima_op_latencies[vima_instructions[0]->memory_operation];
    
    if (vima_sub_requests[this->vima_buffer_start][2].status != PACKAGE_STATE_READY) {
        if (vima_sub_requests[this->vima_buffer_start][2].status == PACKAGE_STATE_TRANSMIT && !vima_sub_requests[this->vima_buffer_start][2].sent_to_ram){
            if (this->cache->searchAddress (vima_sub_requests[this->vima_buffer_start][2].memory_address)){
                vima_sub_requests[this->vima_buffer_start][2].status = PACKAGE_STATE_READY;
                vima_sub_requests[this->vima_buffer_start][2].readyAt = orcs_engine.get_global_cycle() + this->cache->get_VIMA_CACHE_LATENCY();
            }
            else {
                orcs_engine.memory_controller->requestDRAM (&vima_sub_requests[this->vima_buffer_start][2], vima_sub_requests[this->vima_buffer_start][2].memory_address);
                vima_sub_requests[this->vima_buffer_start][2].sent_to_ram = true;
            }
        }
        return;
    } else if (vima_sub_requests[this->vima_buffer_start][2].status == PACKAGE_STATE_READY && vima_sub_requests[this->vima_buffer_start][2].sent_to_ram){
        this->cache->installLine (vima_sub_requests[this->vima_buffer_start][2].memory_address);
        vima_sub_requests[this->vima_buffer_start][2].sent_to_ram = false;
    }

    ORCS_PRINTF ("%lu VIMA Controller clock(): %s readyAt %lu R1 %lu R2 %lu W %lu\n", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_instructions[0]->memory_operation), vima_instructions[0]->readyAt, vima_instructions[0]->vima_read1, vima_instructions[0]->vima_read2, vima_instructions[0]->vima_write)
    this->instruction_ready (0);

    this->vima_buffer_start++;
    if (this->vima_buffer_start == this->VIMA_BUFFER) this->vima_buffer_start = 0;
    ORCS_PRINTF ("VIMA Controller clock: start %lu, end %lu\n", this->vima_buffer_start, this->vima_buffer_end)
}

void vima_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_VIMA_BUFFER (cfg_processor["VIMA_BUFFER"]);
    set_VIMA_DEBUG (cfg_processor["VIMA_DEBUG"]);

    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);

    vima_op_latencies = utils_t::template_allocate_initialize_array<uint32_t>(MEMORY_OPERATION_VIMA_FP_MUL, 0);

    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = cfg_processor["VIMA_LATENCY_INT_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = cfg_processor["VIMA_LATENCY_INT_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = cfg_processor["VIMA_LATENCY_INT_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = cfg_processor["VIMA_LATENCY_FP_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = cfg_processor["VIMA_LATENCY_FP_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = cfg_processor["VIMA_LATENCY_FP_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);

    this->vima_sub_requests = (memory_package_t**) malloc (sizeof (memory_package_t*)*this->VIMA_BUFFER);
    std::memset (this->vima_sub_requests, 0, this->VIMA_BUFFER*sizeof(memory_package_t*));
    for (size_t i = 0; i < this->VIMA_BUFFER; i++){
        this->vima_sub_requests[i] = (memory_package_t*) malloc (3*sizeof (memory_package_t));
        std::memset (this->vima_sub_requests[i], 0, 3*sizeof(memory_package_t));
    }

    this->vima_buffer_start = 0;
    this->vima_buffer_end = 0;

    this->cache = new vima_cache_t;
    this->cache->allocate();
}

bool vima_controller_t::addRequest (memory_package_t* request){
    if (vima_instructions.size() < this->VIMA_BUFFER) {
        request->status = PACKAGE_STATE_VIMA;
        vima_instructions.push_back (request);

        this->set_sub_requests (request);
        this->vima_buffer_end++;
        if (this->vima_buffer_end == this->VIMA_BUFFER) this->vima_buffer_end = 0;

        //if (VIMA_DEBUG) ORCS_PRINTF ("VIMA Controller addRequest(): received new instruction %lu, %s, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation), get_enum_package_state_char (request->status))
        return true;
    } else if (VIMA_DEBUG) ORCS_PRINTF ("VIMA Controller addRequest(): VIMA buffer is full!\n")
    return false;
}