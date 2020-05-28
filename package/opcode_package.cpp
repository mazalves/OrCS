#ifndef __PIN__
#include "../simulator.hpp"
#else
#include "opcode_package.hpp"
#endif

#include <string>
// =====================================================================
opcode_package_t::opcode_package_t() {

    /// TRACE Variables
    sprintf(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0;
    this->opcode_size = 0;

	for (uint32_t i=0; i < MAX_REGISTERS; i++){
		read_regs[i] = 0;
		write_regs[i] = 0;
	}
	
    this->base_reg = 0;
    this->index_reg = 0;

    this->is_read = false;
    this->read_address = 0;
    this->read_size = 0;

    this->is_read2 = false;
    this->read2_address = 0;
    this->read2_size = 0;

    this->is_write = false;
    this->write_address = 0;
    this->write_size = 0;

    this->branch_type = BRANCH_UNCOND;
    this->is_indirect = false;

    this->is_predicated = false;
    this->is_prefetch = false;

    this->is_hive = false;
    this->is_vima = false;

    this->status = PACKAGE_STATE_FREE;
    #ifndef __PIN__
    this->readyAt = orcs_engine.get_global_cycle();
    #else
    this->readyAt = 0;
    #endif  
}

opcode_package_t::~opcode_package_t() {
    
}
#ifndef __PIN__
// =============================================================================
void opcode_package_t::package_clean() {
    /// TRACE Variables
    sprintf(this->opcode_assembly, "N/A");
    this->opcode_operation = INSTRUCTION_OPERATION_NOP;
    this->opcode_address = 0;
    this->opcode_size = 0;

    memset(this->read_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);
    memset(this->write_regs, POSITION_FAIL, sizeof(int32_t) * MAX_REGISTERS);

    this->base_reg = 0;
    this->index_reg = 0;

    this->is_read = false;
    this->read_address = 0;
    this->read_size = 0;

    this->is_read2 = false;
    this->read2_address = 0;
    this->read2_size = 0;

    this->is_write = false;
    this->write_address = 0;
    this->write_size = 0;
    this->is_predicated = false;
    this->is_prefetch = false;

    this->is_hive = false;
    this->hive_read1 = 0;
    this->hive_read2 = 0;
    this->hive_write = 0;
    
    //====Control
    this->readyAt = 0;
    this->status = PACKAGE_STATE_FREE;
    this->opcode_number = 0;
}

/// Convert Instruction variables into String
std::string opcode_package_t::content_to_string() {
    std::string content_string;
    content_string = "";
    content_string = content_string + " " + get_enum_instruction_operation_char(this->opcode_operation);
    content_string = content_string + " $" + utils_t::big_uint64_to_string(this->opcode_address);
    content_string = content_string + " Size:" + utils_t::uint32_to_string(this->opcode_size);

    content_string = content_string + " | R1 $" + utils_t::big_uint64_to_string(this->read_address);
    content_string = content_string + " Size:" + utils_t::uint32_to_string(this->read_size);

    content_string = content_string + " | R2 $" + utils_t::big_uint64_to_string(this->read2_address);
    content_string = content_string + " Size:" + utils_t::uint32_to_string(this->read2_size);

    content_string = content_string + " | W $" + utils_t::big_uint64_to_string(this->write_address);
    content_string = content_string + " Size:" + utils_t::uint32_to_string(this->write_size);


    content_string = content_string + " | RRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->read_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->read_regs[i]);
        }
    }

    content_string = content_string + " ] | WRegs[";
    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        if (this->write_regs[i] >= 0) {
            content_string = content_string + " " + utils_t::uint32_to_string(this->write_regs[i]);
        }
    }
    content_string = content_string + " ]";

    return content_string;
}
std::string opcode_package_t::content_to_string2() {
    std::string content_string;
    content_string = "";
    content_string = content_string + " " + get_enum_instruction_operation_char(this->opcode_operation);
    content_string = content_string + " $" + utils_t::big_uint64_to_string(this->opcode_address);
    content_string = content_string + " Size:" + utils_t::uint32_to_string(this->opcode_size);

    content_string = content_string + " Status:" + get_enum_package_state_char(this->status);
    content_string = content_string + " ready At:" + utils_t::uint64_to_string(this->readyAt);



    return content_string;
}

#endif // ifndef __PIN__
