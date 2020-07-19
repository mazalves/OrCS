#include "./../simulator.hpp"
#include <string>

hive_controller_t::hive_controller_t(){
    this->HIVE_BUFFER = 0;
    this->HIVE_REGISTERS = 0;
    this->HIVE_REGISTER_SIZE = 0;
    this->CORE_TO_BUS_CLOCK_RATIO = 0.0;
        
    this->LINE_SIZE = 0;
    this->HIVE_DEBUG = 0;
    this->data_cache = NULL;

    this->i = 0;
        
    this->offset = 0;
    this->last_instruction = 0;
    this->hive_lock = false;
    this->hive_register_free = NULL;
    this->nano_requests_number = 0;
    this->nano_requests_ready = NULL;
    this->hive_op_latencies = NULL;
    this->hive_register_state = NULL;
    this->hive_sub_requests = NULL;
}

hive_controller_t::~hive_controller_t(){
    delete[] this->nano_requests_ready;
    delete[] this->hive_register_state;
    delete[] this->hive_op_latencies;
    for (i = 0; i < this->HIVE_REGISTERS; i++) delete[] this->hive_sub_requests[i];
    delete[] this->hive_sub_requests;
    std::vector<memory_package_t*>().swap (hive_instructions);
}

void hive_controller_t::print_hive_instructions(){
    ORCS_PRINTF ("=======HIVE INSTRUCTIONS=========\n")
    for (size_t i = 0; i < hive_instructions.size(); i++){
        ORCS_PRINTF ("%lu %s %lu %s R1 %lu R2 %lu W %lu\n", hive_instructions[i]->uop_number, get_enum_memory_operation_char (hive_instructions[i]->memory_operation), hive_instructions[i]->readyAt, get_enum_package_state_char (hive_instructions[i]->status), hive_instructions[i]->hive_read1, hive_instructions[i]->hive_read2, hive_instructions[i]->hive_write)
    }
    ORCS_PRINTF ("=================================\n")
}

void hive_controller_t::instruction_ready (size_t index){
    hive_instructions[index]->updatePackageWait(0);
    if (HIVE_DEBUG) {
        ORCS_PRINTF ("%lu HIVE Controller clock(): instruction %lu, %s ready at cycle %lu.\n", orcs_engine.get_global_cycle(), hive_instructions[index]->uop_number, get_enum_memory_operation_char (hive_instructions[index]->memory_operation), hive_instructions[index]->readyAt)
    }
    hive_instructions.erase (std::remove (hive_instructions.begin(), hive_instructions.end(), hive_instructions[index]), hive_instructions.end());
}

void hive_controller_t::set_sub_requests (memory_package_t* request){
    uint64_t memory_address = request->memory_address >> this->offset;
    size_t hive_register;
    memory_operation_t op;
    if (request->memory_operation == MEMORY_OPERATION_HIVE_LOAD) {
        hive_register = request->hive_write;
        op = MEMORY_OPERATION_READ;
    } else if (request->memory_operation == MEMORY_OPERATION_HIVE_STORE) {
        hive_register = request->hive_read1;
        op = MEMORY_OPERATION_WRITE;
    } else return;

    for (size_t i = 0; i < this->nano_requests_number; i++){
        hive_sub_requests[hive_register][i].uop_number = request->uop_number;
        hive_sub_requests[hive_register][i].memory_operation = op;
        hive_sub_requests[hive_register][i].memory_address = memory_address << this->offset;
        hive_sub_requests[hive_register][i].status = PACKAGE_STATE_TRANSMIT;
        hive_sub_requests[hive_register][i].is_hive = true;
        hive_sub_requests[hive_register][i].readyAt = orcs_engine.get_global_cycle();
        
        memory_address += 1;
    }

    if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller set_sub_requests(): setting sub-requests on register %lu, instruction %lu, %s, %lu.\n", orcs_engine.get_global_cycle(), hive_register, request->uop_number, get_enum_memory_operation_char (request->memory_operation), request->memory_address)
}

void hive_controller_t::reset_sub_requests(size_t hive_register){
    for (size_t j = 0; j < this->nano_requests_number; j++){
        hive_sub_requests[hive_register][j].memory_address = 0;
        hive_sub_requests[hive_register][j].status = PACKAGE_STATE_FREE;
        hive_sub_requests[hive_register][j].memory_operation = MEMORY_OPERATION_FREE;
        hive_sub_requests[hive_register][j].sent_to_ram = false;
        hive_sub_requests[hive_register][j].row_buffer = false;
    }
    this->nano_requests_ready[hive_register] = 0;

    if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller reset_sub_requests(): resetting sub-requests on register %lu.\n", orcs_engine.get_global_cycle(), hive_register)
}

void hive_controller_t::check_sub_requests (size_t hive_register){
    for (size_t i = 0; i < this->nano_requests_number; i++){
        if (this->hive_sub_requests[hive_register][i].sent_to_ram){
            if (this->hive_sub_requests[hive_register][i].status == PACKAGE_STATE_WAIT){
                this->nano_requests_ready[hive_register]++;
                this->hive_sub_requests[hive_register][i].updatePackageFree(0);
                if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller check_sub_requests(): sub-request %lu finished from register %lu, %lu, %lu.\n", orcs_engine.get_global_cycle(), i, hive_register, hive_sub_requests[hive_register][i].memory_address, hive_sub_requests[hive_register][i].uop_number)
            }
        } else if (this->hive_sub_requests[hive_register][i].status == PACKAGE_STATE_TRANSMIT) {
            orcs_engine.memory_controller->requestDRAM (&hive_sub_requests[hive_register][i]);
            this->hive_sub_requests[hive_register][i].sent_to_ram = true;
            if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller check_sub_requests(): sub-request %lu issued from register %lu, %lu, %lu.\n", orcs_engine.get_global_cycle(), i, hive_register, this->hive_sub_requests[hive_register][i].memory_address, hive_sub_requests[hive_register][i].uop_number)
            break;
        }
    }
    if (this->nano_requests_ready[hive_register] == this->nano_requests_number) {
        this->reset_sub_requests (hive_register);
        hive_register_state[hive_register] = PACKAGE_STATE_READY;
        if (HIVE_DEBUG) {
            ORCS_PRINTF ("%lu HIVE Controller check_sub_requests(): register %lu is ready, cycle %lu!\n", orcs_engine.get_global_cycle(), hive_register, orcs_engine.get_global_cycle());
        }
    }
}

void hive_controller_t::check_transmit(){
    package_state_t wait_r1, wait_r2;
    if (hive_instructions[0]->hive_read1 != -1) wait_r1 = hive_register_state[hive_instructions[0]->hive_read1];
    else wait_r1 = PACKAGE_STATE_READY;
    
    if (hive_instructions[0]->hive_read2 != -1) wait_r2 = hive_register_state[hive_instructions[0]->hive_read2];
    else wait_r2 = PACKAGE_STATE_READY;

    if ((wait_r1 == PACKAGE_STATE_READY || wait_r1 == PACKAGE_STATE_FREE) && (wait_r2 == PACKAGE_STATE_READY || wait_r2 == PACKAGE_STATE_FREE)){
        if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller check_transmit(): instruction %s %lu: readyAt %lu -> ", orcs_engine.get_global_cycle(), get_enum_memory_operation_char(hive_instructions[0]->memory_operation), hive_instructions[0]->uop_number, hive_instructions[0]->readyAt)
        hive_instructions[0]->updatePackageHive(hive_op_latencies[hive_instructions[0]->memory_operation]);
        if (HIVE_DEBUG) ORCS_PRINTF ("%lu.\n", hive_instructions[0]->readyAt)
    } else {
        if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller check_transmit(): instruction %lu depends on registers %lu %s and %lu %s.\n", orcs_engine.get_global_cycle(), hive_instructions[0]->uop_number, hive_instructions[0]->hive_read1, get_enum_package_state_char (wait_r1), hive_instructions[0]->hive_read2, get_enum_package_state_char (wait_r2))
    }
}

void hive_controller_t::check_wait(){
    if (hive_instructions[0]->readyAt <= orcs_engine.get_global_cycle()) {
        switch (hive_instructions[0]->memory_operation){
            case MEMORY_OPERATION_HIVE_UNLOCK:
                this->hive_lock = false;
                for (size_t i = 0; i < this->HIVE_REGISTERS; i++) hive_register_state[i] = PACKAGE_STATE_FREE;
                if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller clock(): HIVE IS UNLOCKED, ALL REGISTERS FREED!\n", orcs_engine.get_global_cycle())
                this->instruction_ready (0);
            break;
            case MEMORY_OPERATION_HIVE_LOCK:
                this->hive_lock = true;
                if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller clock(): HIVE IS LOCKED!\n", orcs_engine.get_global_cycle())
                this->instruction_ready (0);
                break;
            case MEMORY_OPERATION_HIVE_FP_ALU:
            case MEMORY_OPERATION_HIVE_FP_DIV:
            case MEMORY_OPERATION_HIVE_FP_MUL:
            case MEMORY_OPERATION_HIVE_INT_ALU:
            case MEMORY_OPERATION_HIVE_INT_DIV:
            case MEMORY_OPERATION_HIVE_INT_MUL:
                if (hive_instructions[0]->hive_read1 == -1 && hive_instructions[0]->hive_read2 == -1){
                    hive_register_state[hive_instructions[0]->hive_write] = PACKAGE_STATE_READY;
                }
                if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller clock(): %s executed! %lu %s\n", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (hive_instructions[0]->memory_operation), hive_instructions[0]->hive_write, get_enum_package_state_char (hive_register_state[hive_instructions[0]->hive_write]))
                this->instruction_ready (0);
                break;
            case MEMORY_OPERATION_HIVE_LOAD:
                if (hive_register_state[hive_instructions[0]->hive_write] == PACKAGE_STATE_READY) this->instruction_ready (0);
                break;
            case MEMORY_OPERATION_HIVE_STORE:
                if (hive_register_state[hive_instructions[0]->hive_read1] == PACKAGE_STATE_READY) this->instruction_ready (0);
                break;
            default:
                break;
        }
    }
}

void hive_controller_t::hive_dispatch(){
    switch (hive_instructions[0]->memory_operation){
        case MEMORY_OPERATION_HIVE_LOCK:
            this->hive_lock = true;
            hive_instructions[0]->updatePackageHive (0);
            break;
        case MEMORY_OPERATION_HIVE_UNLOCK:
            hive_instructions[0]->updatePackageHive (10);
            break;
        case MEMORY_OPERATION_HIVE_FP_ALU:
        case MEMORY_OPERATION_HIVE_FP_DIV:
        case MEMORY_OPERATION_HIVE_FP_MUL:
        case MEMORY_OPERATION_HIVE_INT_ALU:
        case MEMORY_OPERATION_HIVE_INT_DIV:
        case MEMORY_OPERATION_HIVE_INT_MUL:
            if (hive_instructions[0]->hive_read1 == -1 && hive_instructions[0]->hive_read2 == -1){
                hive_register_state[hive_instructions[0]->hive_write] = PACKAGE_STATE_READY;
                hive_instructions[0]->updatePackageHive(hive_op_latencies[hive_instructions[0]->memory_operation]);
            } else {
                hive_instructions[0]->updatePackageTransmit(0);
            }
            break;
        case MEMORY_OPERATION_HIVE_LOAD:
            if (hive_register_state[hive_instructions[0]->hive_write] == PACKAGE_STATE_FREE){
                this->set_sub_requests (hive_instructions[0]);
                hive_instructions[0]->updatePackageHive(0);
                hive_register_state[hive_instructions[0]->hive_write] = PACKAGE_STATE_WAIT;
            }
            break;
        case MEMORY_OPERATION_HIVE_STORE:
            if (hive_register_state[hive_instructions[0]->hive_read1] == PACKAGE_STATE_READY){
                this->set_sub_requests (hive_instructions[0]);
                hive_instructions[0]->updatePackageHive(0);
                hive_register_state[hive_instructions[0]->hive_read1] = PACKAGE_STATE_WAIT;
            }
        default:
            break;
    }
}

void hive_controller_t::clock(){
    if (hive_instructions.size() == 0) return;
    //else print_hive_instructions();

    for (size_t i = 0; i < this->HIVE_REGISTERS; i++) this->check_sub_requests (i);
    
    switch (hive_instructions[0]->status){
        case PACKAGE_STATE_TRANSMIT:
            check_transmit();
            break;
        case PACKAGE_STATE_HIVE:
            check_wait();
            break;
        case PACKAGE_STATE_UNTREATED:
            hive_dispatch();
            break;
        default:
            break;
    }
}

void hive_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_HIVE_BUFFER (cfg_processor["HIVE_BUFFER"]);
    set_HIVE_REGISTERS (cfg_processor["HIVE_REGISTERS"]);
    set_HIVE_DEBUG (cfg_processor["HIVE_DEBUG"]);

    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_HIVE_REGISTER_SIZE (cfg_processor["HIVE_REGISTER_SIZE"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);
    this->nano_requests_number = this->HIVE_REGISTER_SIZE/this->LINE_SIZE;

    this->nano_requests_ready = new uint32_t[this->HIVE_REGISTERS]();
    this->hive_register_state = new package_state_t[this->HIVE_REGISTERS]();
    this->hive_op_latencies = new uint32_t[MEMORY_OPERATION_LAST]();;
    this->hive_sub_requests = new memory_package_t*[this->HIVE_REGISTERS]();
    for (i = 0; i < this->HIVE_REGISTERS; i++) this->hive_sub_requests[i] = new memory_package_t[this->nano_requests_number]();
    for (i = 0; i < this->HIVE_REGISTERS; i++){
        this->hive_register_state[i] = PACKAGE_STATE_FREE;
        this->nano_requests_ready[i] = 0;
    }

    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_ALU] = cfg_processor["HIVE_LATENCY_INT_ALU"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_ALU] = ceil (this->hive_op_latencies[MEMORY_OPERATION_HIVE_INT_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_DIV] = cfg_processor["HIVE_LATENCY_INT_DIV"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_DIV] = ceil (this->hive_op_latencies[MEMORY_OPERATION_HIVE_INT_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_MUL] = cfg_processor["HIVE_LATENCY_INT_MUL"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_MUL] = ceil (this->hive_op_latencies[MEMORY_OPERATION_HIVE_INT_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_ALU] = cfg_processor["HIVE_LATENCY_FP_ALU"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_ALU] = ceil (this->hive_op_latencies[MEMORY_OPERATION_HIVE_FP_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_DIV] = cfg_processor["HIVE_LATENCY_FP_DIV"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_DIV] = ceil (this->hive_op_latencies[MEMORY_OPERATION_HIVE_FP_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_MUL] = cfg_processor["HIVE_LATENCY_FP_MUL"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_MUL] = ceil (this->hive_op_latencies[MEMORY_OPERATION_HIVE_FP_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);

    this->offset = utils_t::get_power_of_two(LINE_SIZE);
    this->hive_lock = false;
    this->last_instruction = 0;
}

bool hive_controller_t::addRequest (memory_package_t* request){
    request->sent_to_ram = true;
    if (hive_instructions.size() < this->HIVE_BUFFER) {
        hive_instructions.push_back (request);
        if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller addRequest(): received new instruction %lu, %s, %s.\n", orcs_engine.get_global_cycle(), request->uop_number, get_enum_memory_operation_char (request->memory_operation), get_enum_package_state_char (request->status))
        return true;
    } else if (HIVE_DEBUG) ORCS_PRINTF ("%lu HIVE Controller addRequest(): HIVE buffer is full!\n",orcs_engine.get_global_cycle())
    return false;
}
