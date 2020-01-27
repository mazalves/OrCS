#include "./../simulator.hpp"
#include <string>

vima_controller_t::vima_controller_t(){

}

vima_controller_t::~vima_controller_t(){

}

void vima_controller_t::print_vima_instructions(){
    ORCS_PRINTF ("=======VIMA INSTRUCTIONS=========\n")
    for (size_t i = 0; i < vima_buffer.size(); i++){
        ORCS_PRINTF ("uop %lu %s readyAt %lu status %s\n", vima_buffer[i]->uop_number, get_enum_memory_operation_char (vima_buffer[i]->memory_operation), vima_buffer[i]->readyAt, get_enum_package_state_char (vima_buffer[i]->status))
    }
    ORCS_PRINTF ("=================================\n")
}

void vima_controller_t::instruction_ready (size_t index){
    vima_buffer[index]->status = PACKAGE_STATE_READY;
    vima_buffer[index]->readyAt = orcs_engine.get_global_cycle();
    if (VIMA_DEBUG) {
        ORCS_PRINTF ("VIMA Controller clock(): instruction %lu, %s ready at cycle %lu.\n", vima_buffer[index]->uop_number, get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->readyAt)
    }
    vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[index]), vima_buffer.end());
}

vima_vector_t* vima_controller_t::search_cache (uint64_t address){
    uint32_t index = get_index (address);
    uint64_t lru_cycle = UINT64_MAX;
    uint32_t lru_way = 0;
    for (uint32_t i = 0; i < VIMA_CACHE_ASSOCIATIVITY; i++){
        if (cache[index][i].tag == get_tag (address)) {
            //ORCS_PRINTF (" way %u\n", i)
            return &cache[index][i];
        }
        else if (cache[index][i].lru < lru_cycle) {
            lru_cycle = cache[index][i].lru;
            lru_way = i;
        }
    }
    //ORCS_PRINTF (" way %u\n", lru_way)
    return &cache[index][lru_way];
}

void vima_controller_t::check_cache(){
    if (read1->get_address() != vima_buffer[0]->vima_read1){
        //ORCS_PRINTF ("R1 VIMA Cache: index %lu, tag %lu ", get_index (vima_buffer[0]->vima_read1), get_tag (vima_buffer[0]->vima_read1))
        read1 = search_cache (vima_buffer[0]->vima_read1);
        read1->set_lru (orcs_engine.get_global_cycle());
        if (read1->get_tag() == get_tag (vima_buffer[0]->vima_read1)) read1->status = PACKAGE_STATE_READY;
        else {
            read1->set_tag (get_tag (vima_buffer[0]->vima_read1));
            read1->status = PACKAGE_STATE_TRANSMIT;
        }
        read1->set_address (vima_buffer[0]->vima_read1);
    }
    ORCS_PRINTF ("R1 ")
    read1->print_vector();
    if (read2->get_address() != vima_buffer[0]->vima_read2){
        //ORCS_PRINTF ("R2 VIMA Cache: index %lu, tag %lu ", get_index (vima_buffer[0]->vima_read2), get_tag (vima_buffer[0]->vima_read2))
        read2 = search_cache (vima_buffer[0]->vima_read2);
        read2->set_lru (orcs_engine.get_global_cycle());
        if (read2->get_tag() == get_tag (vima_buffer[0]->vima_read2)) read2->status = PACKAGE_STATE_READY;
        else {
            read2->set_tag (get_tag (vima_buffer[0]->vima_read2));
            read2->status = PACKAGE_STATE_TRANSMIT;
        }
        read2->set_address (vima_buffer[0]->vima_read2);
    }
    ORCS_PRINTF ("R2 ")
    read2->print_vector();
    if (write->get_address() != vima_buffer[0]->vima_write){
        //ORCS_PRINTF ("W VIMA Cache: index %lu, tag %lu ", get_index (vima_buffer[0]->vima_write), get_tag (vima_buffer[0]->vima_write))
        write = search_cache (vima_buffer[0]->vima_write);
        write->set_lru (orcs_engine.get_global_cycle());
        if (write->get_tag() == get_tag (vima_buffer[0]->vima_write)) write->status = PACKAGE_STATE_READY;
        else {
            write->set_tag (get_tag (vima_buffer[0]->vima_write));
            write->status = PACKAGE_STATE_TRANSMIT;
        }
        write->set_address (vima_buffer[0]->vima_write);
    }
    ORCS_PRINTF ("W  ")
    write->print_vector();

    if (read1->status == PACKAGE_STATE_READY && read2->status == PACKAGE_STATE_READY){
        vima_buffer[0]->status = PACKAGE_STATE_READY;
    }
}

void vima_controller_t::clock(){
    read1->clock();
    read2->clock();
    write->clock();

    if (vima_buffer.size() <= 0) return;
    switch (vima_buffer[0]->status){
        case PACKAGE_STATE_READY:
            ///ORCS_PRINTF ("%lu %s -> ", vima_buffer[0]->uop_number, get_enum_package_state_char (vima_buffer[0]->status))
            //vima_buffer[0]->status = PACKAGE_STATE_READY;
            ///ORCS_PRINTF ("%s %lu\n", get_enum_package_state_char (vima_buffer[0]->status), orcs_engine.get_global_cycle())
            instruction_ready (0);
            break;
        case PACKAGE_STATE_VIMA:
            check_cache();
            ///ORCS_PRINTF ("%lu %s -> ", vima_buffer[0]->uop_number, get_enum_package_state_char (vima_buffer[0]->status))
            break;
        default:
            ///ORCS_PRINTF ("%lu %s -> \n", vima_buffer[0]->uop_number, get_enum_package_state_char (vima_buffer[0]->status))
            vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[0]), vima_buffer.end());
            ORCS_PRINTF ("LIGHTS UP\n")
            break;
    }
}

void vima_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_VIMA_BUFFER (cfg_processor["VIMA_BUFFER"]);
    set_VIMA_DEBUG (cfg_processor["VIMA_DEBUG"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);
    set_VIMA_CACHE_SIZE (cfg_processor["VIMA_CACHE_SIZE"]);
    set_VIMA_VECTOR_SIZE (cfg_processor["VIMA_VECTOR_SIZE"]);
    set_VIMA_CACHE_ASSOCIATIVITY (cfg_processor["VIMA_CACHE_ASSOCIATIVITY"]);
    set_VIMA_CACHE_LATENCY (cfg_processor["VIMA_CACHE_LATENCY"]);

    uint64_t lines = this->get_VIMA_CACHE_SIZE()/this->get_VIMA_VECTOR_SIZE();
    uint64_t sets = lines/this->get_VIMA_CACHE_ASSOCIATIVITY();

    read1 = (vima_vector_t*) malloc (sizeof (vima_vector_t));
    read2 = (vima_vector_t*) malloc (sizeof (vima_vector_t));
    write = (vima_vector_t*) malloc (sizeof (vima_vector_t));

    this->cache = (vima_vector_t**) malloc (sizeof (vima_vector_t*)*sets);
    std::memset (this->cache, 0, sizeof (vima_vector_t*)*sets);
    for (size_t i = 0; i < sets; i++){
        this->cache[i] = (vima_vector_t*) malloc (this->get_VIMA_CACHE_ASSOCIATIVITY()*sizeof (vima_vector_t));
        std::memset (this->cache[i], 0, this->get_VIMA_CACHE_ASSOCIATIVITY()*sizeof(vima_vector_t));
        for (size_t j = 0; j < get_VIMA_CACHE_ASSOCIATIVITY(); j++) cache[i][j].allocate();
    }

    this->index_bits_shift = utils_t::get_power_of_two(this->get_VIMA_VECTOR_SIZE());
    this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(sets);

    uint64_t i;
    /// INDEX MASK
    for (i = 0; i < utils_t::get_power_of_two(sets); i++) {
        this->index_bits_mask |= 1 << (i + index_bits_shift);
    }

    /// TAG MASK
    for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->tag_bits_mask |= 1 << i;
    }

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
}

bool vima_controller_t::addRequest (memory_package_t* request){
    if (vima_buffer.size() < this->VIMA_BUFFER) {
        request->status = PACKAGE_STATE_VIMA;
        vima_buffer.push_back (request);
        return true;
    } else if (VIMA_DEBUG) ORCS_PRINTF ("VIMA Controller addRequest(): VIMA buffer is full!\n")
    return false;
}