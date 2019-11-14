#include "../simulator.hpp"

hive_register_t::hive_register_t(){

}

hive_register_t::~hive_register_t(){
    
}

void hive_register_t::del(){
    for (size_t i = 0; i < this->nano_memory_requests.size(); i++){
        delete nano_memory_requests[i];
    }
}

void hive_register_t::clock(){
    if (this->status == PACKAGE_STATE_WAIT){
        if (request != NULL && request->status != PACKAGE_STATE_WAIT){
            uint64_t memory_address = 0;
            memory_address = request->memory_address;
            memory_address = memory_address >> this->offset;

            for (int i = 0; i < this->nano_requests_number; i++){
                memory_package_t* new_nano = new memory_package_t;

                new_nano->clients.push_back (this);
                new_nano->opcode_address = request->opcode_address;
                if (request->memory_operation == MEMORY_OPERATION_HIVE_LOAD) new_nano->memory_operation = MEMORY_OPERATION_READ;
                else if (request->memory_operation == MEMORY_OPERATION_HIVE_STORE) new_nano->memory_operation = MEMORY_OPERATION_WRITE;
                new_nano->memory_size = request->memory_size;
                new_nano->memory_address = memory_address << this->offset;
                new_nano->status = PACKAGE_STATE_TRANSMIT;
                new_nano->is_hive = true;
                new_nano->readyAt = orcs_engine.get_global_cycle();
                new_nano->uop_number = request->uop_number;
                new_nano->processor_id = request->processor_id;
                nano_memory_requests.push_back (new_nano);

                memory_address += 1;
            }

            if (DEBUG){
                ORCS_PRINTF ("HIVE Register clock(): generated sub-requests for instruction %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
            }

            request->status = PACKAGE_STATE_WAIT;
        } else if (request->status == PACKAGE_STATE_WAIT){
            if (!this->issued){
                for (size_t i = 0; i < nano_memory_requests.size(); i++){
                    orcs_engine.memory_controller->requestDRAM (nano_memory_requests[i], nano_memory_requests[i]->memory_address);
                }
                if (DEBUG){
                    ORCS_PRINTF ("HIVE Register clock(): issued sub-requests for instruction %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
                }
                this->issued = true;
            }
            else {
                for (size_t i = 0; i < nano_memory_requests.size(); i++){
                    if (this->nano_memory_requests[i]->status == PACKAGE_STATE_READY) {
                        this->ready_count++;
                        this->nano_memory_requests[i]->status = PACKAGE_STATE_FREE;
                        this->nano_memory_requests.erase (std::remove (nano_memory_requests.begin(), nano_memory_requests.end(), nano_memory_requests[i]), nano_memory_requests.end());
                        if (DEBUG){
                            ORCS_PRINTF ("HIVE Register clock(): sub-request %d for instruction %lu, %s finished and removed.\n", this->ready_count, request->uop_number, get_enum_memory_operation_char (request->memory_operation))
                        }
                    }
                }
                if (this->ready_count == 128) {
                    this->issued = false;
                    this->ready_count = 0;
                    this->status = PACKAGE_STATE_READY;
                    this->request->updatePackageReady (0);
                    if (DEBUG){
                        ORCS_PRINTF ("HIVE Register clock(): instruction %lu, %s is READY.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
                    }
                }
            }
        }
    }
}

void hive_register_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_HIVE_REGISTER_SIZE (cfg_processor["HIVE_REGISTER_SIZE"]);
    set_DEBUG (cfg_processor["DEBUG"]);
    this->nano_requests_number = this->HIVE_REGISTER_SIZE/this->LINE_SIZE;

    this->issued = false;
    this->ready_count = 0;
    this->status = PACKAGE_STATE_FREE;
    this->offset = utils_t::get_power_of_two(LINE_SIZE);
}

bool hive_register_t::installRequest (memory_package_t* request){
    if (this->status != PACKAGE_STATE_FREE) {
        ORCS_PRINTF ("%d\n", this->status)
        return false;
    }
    if (DEBUG){
        ORCS_PRINTF ("HIVE Register clock(): installed new request, instruction %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
    }
    this->ready_count = 0;
    this->status = PACKAGE_STATE_WAIT;
    this->request = request;
    return true;
}