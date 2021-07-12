#include "./../simulator.hpp"
#include <string>

// ============================================================================
memory_order_buffer_line_t::memory_order_buffer_line_t() {
    this->package_clean();
    this->mem_deps_ptr_array = NULL;
}

// ============================================================================
memory_order_buffer_line_t::~memory_order_buffer_line_t() {
    //utils_t::template_delete_array<memory_order_buffer_line_t*>(mem_deps_ptr_array);
}


// ============================================================================
void memory_order_buffer_line_t::package_clean() {
        this->opcode_address=0;
        this->memory_address=0;
        this->memory_size=0;
        this->is_hive = false;
        this->hive_read1 = 0;
        this->hive_read2 = 0;
        this->hive_write = 0;
        this->is_vima = false;
        this->rob_ptr=NULL;                 /// rob pointer
        this->uop_executed=false;
        this->uop_number = 0;
        this->readyAt = orcs_engine.get_global_cycle();  
        this->status = PACKAGE_STATE_FREE;
        this->memory_operation = MEMORY_OPERATION_FREE;
        this->readyToGo = orcs_engine.get_global_cycle();
        this->cycle_send_request = orcs_engine.get_global_cycle();
        this->wait_mem_deps_number = 0;
        //control variables
        this->processed = false;
        this->sent=false;
        this->forwarded_data=false;
        this->waiting_DRAM=false;
        this->core_generate_miss=false;
        this->processor_id = 0;
        this->cycle_sent_to_DRAM = orcs_engine.get_global_cycle();
}

// ============================================================================
std::string memory_order_buffer_line_t::content_to_string() {
    std::string content_string;
    content_string = "";
    if(this->status == PACKAGE_STATE_FREE){
        return content_string;
    }
    content_string = content_string + " |Uop Number:" + utils_t::uint64_to_string(this->uop_number);
    content_string = content_string + " |Exec:" + utils_t::uint64_to_string(this->opcode_address);
    content_string = content_string + " |Executed:" + utils_t::bool_to_string(this->uop_executed);
    content_string = content_string + " |Mem. Operation:" +  get_enum_memory_operation_char(this->memory_operation);
    content_string = content_string + " |Mem. Address:" +  utils_t::uint64_to_string(this->memory_address);
    content_string = content_string + " |Mem. Size:" +  utils_t::uint64_to_string(this->memory_size);
    content_string = content_string + " |Status:" +  get_enum_package_state_char(this->status);
    content_string = content_string + " |Wait Mem Deps:" + utils_t::int32_to_string(this->wait_mem_deps_number);
    content_string = content_string + " |Ready At:" +  utils_t::uint64_to_string(this->readyAt);
    content_string = content_string + " |Ready To Go:" +  utils_t::uint64_to_string(this->readyToGo) + "\n";
    content_string = content_string + " | CONTROL FLAGS: \n";
    content_string = content_string + " |Sent:"+utils_t::bool_to_string(this->sent);
    content_string = content_string + " |Processed:"+utils_t::bool_to_string(this->processed);
    content_string = content_string + " |Forwarded_data:"+utils_t::bool_to_string(this->forwarded_data);
    content_string = content_string + " |Waiting_DRAM:"+utils_t::bool_to_string(this->waiting_DRAM);
    content_string = content_string + " |Core Miss:"+utils_t::bool_to_string(this->core_generate_miss);
    content_string = content_string + " |Core ID:" + utils_t::int32_to_string(this->processor_id);
    return content_string;
}


// ============================================================================
/// STATIC METHODS
// ============================================================================
int32_t memory_order_buffer_line_t::find_free(memory_order_buffer_line_t *input_array, uint32_t size_array) {
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].status == PACKAGE_STATE_FREE){
            return i;
        }
    }
    return POSITION_FAIL;
}
// ============================================================================
int32_t memory_order_buffer_line_t::find_old_request_state_ready(memory_order_buffer_line_t *input_array, uint32_t size_array, package_state_t state) {
    int32_t old_pos = POSITION_FAIL;
    // uint64_t old_uop_number = UINT64_MAX;    
    uint64_t old_uop_number = std::numeric_limits<uint64_t>::max();
    /// Find the oldest UOP inside the MOB.... and it have 0 deps.
    for (uint32_t i = 0; i < size_array ; i++) {
        if (input_array[i].status == state &&
        input_array[i].uop_number < old_uop_number &&
        input_array[i].wait_mem_deps_number <= 0 &&
        input_array[i].uop_executed == true 
        && input_array[i].readyToGo <= orcs_engine.get_global_cycle()
        ) {
            old_uop_number = input_array[i].uop_number;
            old_pos = i;
        }
    }
    return old_pos;
}

// =========================================================================
// Print all strutcure of mob array
// =========================================================================
void memory_order_buffer_line_t::printAllOrder(memory_order_buffer_line_t* input_array, uint32_t size_array,uint32_t start, uint32_t used){
    uint32_t pos = start;
    for (uint32_t i = 0; i< used; i++){
		ORCS_PRINTF("%s\n",input_array[pos].content_to_string().c_str())
        pos++;
		if(pos>=size_array)pos=0;
    }
}
