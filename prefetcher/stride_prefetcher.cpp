#include "../simulator.hpp"

stride_prefetcher_t::stride_prefetcher_t(){}

stride_prefetcher_t::~stride_prefetcher_t() {
    if(this->stride_table) delete[] &this->stride_table;
}

void stride_prefetcher_t::allocate(uint32_t NUMBER_OF_PROCESSORS) {
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_prefetcher = cfg_root["PREFETCHER"];
    set_DISTANCE(cfg_prefetcher["DISTANCE"]);

    set_STRIDE_TABLE_SIZE (NUMBER_OF_PROCESSORS * 32);
    this->stride_table = new stride_table_t[STRIDE_TABLE_SIZE]();
}

inline uint32_t stride_prefetcher_t::searchLRU() {
    uint32_t idx = 0;
    for (uint32_t i = 1; i < STRIDE_TABLE_SIZE; i++){
        idx = (this->stride_table[idx].lru < this->stride_table[i].lru)? idx : i;
    }
    return idx;
}

int32_t stride_prefetcher_t::searchPattern(uint64_t pc) {
    uint64_t tag = pc;
    for (uint32_t i = 0; i < STRIDE_TABLE_SIZE; i++) {
        if((this->stride_table[i].tag == tag)&&(this->stride_table[i].status != INVALID)) {
            this->stride_table[i].lru = orcs_engine.get_global_cycle();
            return i;
        }
    }
    return MISS;
}

uint32_t stride_prefetcher_t::installStride(uint64_t pc,uint64_t address) {
    uint64_t tag = pc;
    for (size_t i = 0; i < STRIDE_TABLE_SIZE; i++) {
        if(this->stride_table[i].status == INVALID){
            this->stride_table[i].tag = tag;
            this->stride_table[i].last_address = address;
            this->stride_table[i].stride = 0;
            this->stride_table[i].status = TRAINING;
            this->stride_table[i].lru = orcs_engine.get_global_cycle();
            return OK;
        }
    }
    uint32_t idx = this->searchLRU();
    this->stride_table[idx].tag = tag;
    this->stride_table[idx].last_address = address;
    this->stride_table[idx].stride = 0;
    this->stride_table[idx].status = TRAINING;
    this->stride_table[idx].lru = orcs_engine.get_global_cycle();
    return OK;
}

uint32_t stride_prefetcher_t::updateStride(uint64_t pc, uint64_t address, status_stride_prefetcher_t status) {
    uint64_t tag = pc;
    for (size_t i = 0; i < STRIDE_TABLE_SIZE; i++) {
        if(this->stride_table[i].tag == tag){
            this->stride_table[i].stride = labs(this->stride_table[i].last_address - address);
            this->stride_table[i].last_address = address;
            this->stride_table[i].status = status;
            this->stride_table[i].lru = orcs_engine.get_global_cycle();
            return OK;
        }
    }
    return OK;
}

int64_t stride_prefetcher_t::verify(uint64_t pc,uint64_t address) {
    int64_t new_address = POSITION_FAIL;
    int32_t idx = this->searchPattern(pc); 
    if(idx == MISS) {
        this->installStride(pc,address);
    } else { 
        uint64_t anterior = this->stride_table[idx].last_address - this->stride_table[idx].stride;
        uint64_t posterior = this->stride_table[idx].last_address + this->stride_table[idx].stride;

        if((address >= anterior)||(address <=posterior)) {
            if(this->stride_table[idx].status == INVALID) {
                this->updateStride(pc,address,TRAINING);
            }
            if(this->stride_table[idx].status == ACTIVE) {
                uint32_t stride = labs(address-this->stride_table[idx].last_address);
                if(stride == this->stride_table[idx].stride) {
                //verifica se ta instalada
                new_address = (DISTANCE*stride)+this->stride_table[idx].last_address; 
                this->updateStride(pc,address,ACTIVE);
                } else {
                    this->updateStride(pc,address,INVALID);
                }
            }
            if(this->stride_table[idx].status == TRAINING) {
                    this->updateStride(pc,address,ACTIVE);
            }
        }
    }
    return new_address;
}