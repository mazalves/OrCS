#include "./../simulator.hpp"

instruction_set_t::instruction_set_t() {}
instruction_set_t::~instruction_set_t() {}

void instruction_set_t::allocate() {
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();

    libconfig::Setting &cfg_uops = cfg_root["UOPS"];
    libconfig::Setting &cfg_instructions = cfg_root["INSTRUCTIONS"];
    libconfig::Setting &cfg_fu = cfg_root["FUNCTIONAL_UNITS"];


    // Parse functional units
    uint32_t num_fu = cfg_fu.getLength();
    functional_units.resize(num_fu);

    for (uint32_t i = 0; i < num_fu; ++i) {
        uint32_t size = cfg_fu[i]["SIZE"];
        uint32_t wait_next = cfg_fu[i]["WAIT_NEXT"];

        std::string name = cfg_fu[i]["NAME"];

        fu_id[name] = i;
        functional_units[i] = {size, wait_next};
    }
    
    // Parse uops
    uint32_t num_uops = cfg_uops.getLength();
    uops.resize(num_uops);

    for (uint32_t i = 0; i < num_uops; ++i) {
        uint32_t lat = cfg_uops[i]["LATENCY"];

        std::string name = cfg_uops[i]["NAME"];
        std::string fu_name  = cfg_uops[i]["FU"];

        uops_id[name] = i;
        uops[i] = uop_info_t{lat, fu_id[fu_name]};
    }

    // Parse instructions
    uint32_t num_instructions = cfg_instructions.getLength();
    uops_per_instruction.resize(num_instructions + 1);

    for (uint32_t i = 0; i < num_instructions; ++i) {
        string name = cfg_instructions[i]["NAME"];
        instructions_id[name] = i;

        libconfig::Setting &cfg_inst_uops = cfg_instructions[i]["UOPS"];
        
        uint32_t n_uops = cfg_inst_uops.getLength();
        uops_per_instruction[i].resize(n_uops);

        for (uint32_t j = 0; j < n_uops; ++j) {
            uint32_t u_id = uops_id[cfg_inst_uops[j]];
            uops_per_instruction[i][j] = u_id;
        }
    }
}
