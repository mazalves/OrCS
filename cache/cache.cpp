#include "../simulator.hpp"

// Constructor
cache_t::cache_t() {
    this->id = 0;
    this->level = 0;
    this->size = 0;
    this->latency = 0;
    this->associativity = 0;
    this->n_sets = 0;
    this->sets = NULL;
    this->offset = 0;
	this->cache_hit = 0;
    this->cache_miss = 0;
    this->cache_access = 0;
	this->cache_eviction = 0;
    this->cache_read = 0;
    this->cache_write = 0;
    this->cache_writeback = 0;
    this->change_line = 0;
}

cache_t::~cache_t(){
	// delete[] sets;
}

// Allocate each cache type
void cache_t::allocate(uint32_t NUMBER_OF_PROCESSORS, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS) {
    // Access configure file
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();

	// Get prefetcher info
	libconfig::Setting &prefetcher_defs = cfg_root["PREFETCHER"];
	set_PREFETCHER_ACTIVE(prefetcher_defs["PREFETCHER_ACTIVE"]);

	// Get general cache info
	libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"]["CONFIG"];
	set_WAIT_CYCLE(cfg_cache_defs["WAIT_CYCLE"]);

	set_INSTRUCTION_LEVELS (INSTRUCTION_LEVELS);
	set_DATA_LEVELS (DATA_LEVELS);
	POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
	
	this->sets = new cacheSet_t[this->n_sets];
    for (size_t i = 0; i < this->n_sets; i++) {
		this->sets[i].lines = new line_t[this->associativity];
		this->sets[i].n_lines = this->associativity;
        for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
            this->sets[i].lines[j].allocate(POINTER_LEVELS);
            for (uint32_t k = 0; k < NUMBER_OF_PROCESSORS; k++) {
                for (uint32_t l = 0; l < POINTER_LEVELS; l++) {
                    this->sets[i].lines[j].line_ptr_caches[k][l] = NULL;
                }
				this->sets[i].lines[j].directory_line = new directory_line_t;
			}
            this->sets[i].lines[j].clean_line();
        }
    }
    this->set_cache_access(0);
    this->set_cache_hit(0);
    this->set_cache_miss(0);
	this->set_cache_eviction(0);
    this->set_cache_read(0);
    this->set_cache_write(0);
    this->set_cache_writeback(0);
}

// Return address index in cache
inline void cache_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag) {
	uint32_t get_bits = (this->n_sets) - 1;
	*tag = (address >> this->offset);
	*idx = *tag & get_bits;
	*tag >>= utils_t::get_power_of_two(this->n_sets);
}

// Reads a cache, updates cycles and return HIT or MISS status
uint32_t cache_t::read(uint64_t address,uint32_t &ttc){
    uint32_t idx;
    uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	for (size_t i = 0; i < this->sets->n_lines; i++) {
		if(this->sets[idx].lines[i].tag == tag) {
			// Se ready Cycle for menor que o ciclo atual, a latencia é apenas da leitura, sendo um hit.
			if (this->sets[idx].lines[i].ready_at <= orcs_engine.get_global_cycle()){
				if (PREFETCHER_ACTIVE){
					if (this->sets[idx].lines[i].prefetched == 1){
						orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
						this->sets[idx].lines[i].prefetched =0;
					}
				}
				this->sets[idx].lines[i].lru = orcs_engine.get_global_cycle();
				ttc += this->latency;
				// if (this->id == DATA) {
				// 	if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				// 		//ORCS_PRINTF("     Cache level %u Ready At %lu\n", this->level, this->sets[idx].lines[i].ready_at)
				// 	}
				// }
				return HIT;
			}
			// Se ready Cycle for maior que o atual, a latencia é dada pela demora a chegar
			else{
				if (PREFETCHER_ACTIVE){
					if (this->sets[idx].lines[i].prefetched == 1){
						orcs_engine.cacheManager->prefetcher->add_latePrefetches();
						orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
						uint32_t latePrefetcher = orcs_engine.cacheManager->prefetcher->get_totalCycleLate()+
						(this->sets[idx].lines[i].ready_at - orcs_engine.get_global_cycle());
						orcs_engine.cacheManager->prefetcher->set_totalCycleLate(latePrefetcher);
						this->sets[idx].lines[i].prefetched =0;
					}
				}
				ttc+=(this->sets[idx].lines[i].ready_at - orcs_engine.get_global_cycle());
				this->sets[idx].lines[i].lru = ttc;
				return HIT;
			}
		}
	}
	ttc += this->latency;
	return MISS;
}

// Returns the minor LRU line
inline uint32_t cache_t::searchLru(cacheSet_t *set) {
	uint32_t index = 0;
	for (uint32_t i = 1; i < set->n_lines; i++)	{
		index = (set->lines[index].lru <= set->lines[i].lru)? index : i;
	}
	return index;
}

// Copy data information to lower cache levels when data addresses are valid
void cache_t::copyLevels(line_t *line, uint32_t idxa, uint32_t idxb) {
	if (line->line_ptr_caches[0][idxa]->dirty == 1) {
		line->line_ptr_caches[0][idxa]->line_ptr_caches[0][idxb]->dirty = line->line_ptr_caches[0][idxa]->dirty;
		line->line_ptr_caches[0][idxa]->line_ptr_caches[0][idxb]->lru = orcs_engine.get_global_cycle();
		line->line_ptr_caches[0][idxa]->line_ptr_caches[0][idxb]->ready_at = line->line_ptr_caches[0][idxa]->ready_at;
	}
	line->line_ptr_caches[0][idxa]->clean_line();
}

// Copy data information to lower cache levels when data addresses are invalid
void cache_t::copyNextLevels(line_t *line, uint32_t idx) {
	line->line_ptr_caches[0][idx]->dirty = line->dirty;
	line->line_ptr_caches[0][idx]->lru = orcs_engine.get_global_cycle();
	line->line_ptr_caches[0][idx]->ready_at = line->ready_at;
}

// Writebacks an address from a specific cache to its next lower leveL
inline void cache_t::writeBack(line_t *line, directory_t *directory, uint32_t idx, uint32_t access_line) {
    for (uint32_t i = this->level + 1; i < DATA_LEVELS - 1; i++) {
        ERROR_ASSERT_PRINTF(line->line_ptr_caches[0][i] != NULL, "Error, no line reference in next levels.")
    }
	// L1 writeBack issues
	if (this->level == 0) {
		// printf("LVL1 WB DIR: %u\n", this->sets[idx].lines[access_line].directory_line->level);
		this->sets[idx].lines[access_line].directory_line->cache_status = UNCACHED;
		for (uint32_t i = 1; i < DATA_LEVELS; i++) {
			this->copyNextLevels(line, i);
			line->line_ptr_caches[0][i]->line_ptr_caches[0][this->level] = NULL;//Pointer to Lower Level
		}
		line->clean_line();
	// LLC writeBack issues
    } else if (this->level == DATA_LEVELS - 1) {
		// printf("LVL3 WB DIR: %u\n", this->sets[idx].lines[access_line].directory_line->level);
		this->sets[idx].lines[access_line].directory_line->cache_status = UNCACHED;
		for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
			directory->sets[idx].lines[access_line][i].clean_line();
		}
		for (uint32_t i = 0; i < DATA_LEVELS - 1; i++) {
			if (line->line_ptr_caches[0][i] != NULL) {
				line->line_ptr_caches[0][i]->clean_line();
			}
		}
	// Intermediate cache levels issues
	} else {
		// printf("LVL2 WB DIR: %u\n", this->sets[idx].lines[access_line].directory_line->level);
		this->sets[idx].lines[access_line].directory_line->cache_status = UNCACHED;
		uint32_t i = 0;
		// for (i = 0; i < this->level - 1; i++) {
        //     // printf("%s\n", "for");
		// 	if (line->line_ptr_caches[0][i] != NULL) {
        //         // printf("%s\n", "if");mshr_entry_t* add_mshr_entry(memory_order_buffer_line_t* mob_line, uint64_t latency_request);
        
		// 		copyLevels(line, i, i + 1);
		// 	}
		// }
        if (line->line_ptr_caches[0][i] != NULL) {
			copyLevels(line, i, i + 2);
		} else {
			copyNextLevels(line, i + 2);

		}
		for (uint32_t i = this->level + 1; i < DATA_LEVELS; i++) {
			for (uint32_t j = 0; j <= this->level; j++) {
				line->line_ptr_caches[0][i]->line_ptr_caches[0][j] = NULL;
			}
		}
		line->clean_line();
	}
}

// Searches for a cache line to write data
line_t* cache_t::installLine(uint64_t address, uint32_t latency, directory_t *directory, uint32_t &idx, uint32_t &line) {
	line = POSITION_FAIL;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		if (this->sets[idx].lines[i].valid == 0) {
			line = i;
            break;
		}
	}
	if ((int)line == POSITION_FAIL) {
		line = this->searchLru(&this->sets[idx]);
		this->add_change_line();
		if (this->sets[idx].lines[line].dirty == 1) {
			this->writeBack(&this->sets[idx].lines[line], directory, idx, line);
			this->add_cache_writeback();
		}
	}
	this->sets[idx].lines[line].tag = tag;
	this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle() + latency;
	this->sets[idx].lines[line].valid = 1;
	this->sets[idx].lines[line].dirty = 0;
	this->sets[idx].lines[line].prefetched = 0;
	this->sets[idx].lines[line].ready_at = orcs_engine.get_global_cycle() + latency;
	this->add_cache_eviction();
	return &this->sets[idx].lines[line];
}

// Selects a cache line to install an address and points this memory address with the other cache pointers
void cache_t::returnLine(uint64_t address, cache_t *cache, directory_t *directory) {
	uint32_t idx, idx_padding, line_padding;
	uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
	int32_t line = POSITION_FAIL;
	// Selects a line in this cache
	for (size_t i = 0; i < this->sets->n_lines; i++) {
		if (this->sets[idx].lines[i].tag == tag) {
			this->sets[idx].lines[i].lru = orcs_engine.get_global_cycle();
			line = i;
			break;
		}
	}
    ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line não encontrada para retorno")
	if (this->level > 0) {
		line_t *line_return = NULL;
		line_return = cache->installLine(address, this->latency, directory, idx_padding, line_padding);
		line_return->directory_line->cache_status = CACHED;

		this->sets[idx].lines[line].line_ptr_caches[0][cache->level] = line_return;
		for (uint32_t i = this->level + 1; i < POINTER_LEVELS; i++) {
			line_return->line_ptr_caches[0][i] = this->sets[idx].lines[line].line_ptr_caches[0][i];
		}
		line_return->line_ptr_caches[0][this->level] = &this->sets[idx].lines[line];
		// Copy information
		line_return->dirty = line_return->line_ptr_caches[0][this->level]->dirty;
		line_return->lru = line_return->line_ptr_caches[0][this->level]->lru;
		line_return->prefetched = line_return->line_ptr_caches[0][this->level]->prefetched;
		line_return->ready_at = orcs_engine.get_global_cycle();
	}
}


// write address
uint32_t cache_t::write(uint64_t address, directory_t *directory){
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
	int32_t line = POSITION_FAIL;
	this->add_cache_write();
    for (size_t i = 0; i < this->sets->n_lines; i++) {
		if(this->sets[idx].lines[i].tag == tag){
			line = i;
			break;
		}
	}
    if (line == POSITION_FAIL) {
        line = this->searchLru(&this->sets[idx]);
        this->add_change_line();
        if (this->sets[idx].lines[line].dirty == 1) {
            this->writeBack(&this->sets[idx].lines[line], directory, idx, line);
            this->add_cache_writeback();
        }
    }
	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line didn't find to be written.")
    this->sets[idx].lines[line].dirty = 1;
	if(this->sets[idx].lines[line].ready_at <= orcs_engine.get_global_cycle()){
		this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();
	} else {
		this->sets[idx].lines[line].lru = this->sets[idx].lines[line].ready_at + this->latency;
	}
	return OK;
}

// statistics of a level of cache
void cache_t::statistics() {
	FILE *output = stdout;
	bool close = false;
	if(orcs_engine.output_file_name != NULL){
		close=true;
		output = fopen(orcs_engine.output_file_name,"a+");
	}
	if (output != NULL){
		utils_t::largeSeparator(output);
		fprintf(output, "Cache_Level: %d - Cache_Type: %u\n", this->level, this->id);
		fprintf(output, "%d_Cache_Access: %lu\n", this->level, this->get_cache_access());
		fprintf(output, "%d_Cache_Hits: %lu\n", this->level, this->get_cache_hit());
		fprintf(output, "%d_Cache_Miss: %lu\n", this->level, this->get_cache_miss());
		fprintf(output, "%d_Cache_Eviction: %lu\n", this->level, this->get_cache_eviction());
		fprintf(output, "%d_Cache_Read: %lu\n", this->level, this->get_cache_read());
		fprintf(output, "%d_Cache_Write: %lu\n", this->level, this->get_cache_write());
		if(this->get_cache_writeback()!=0){
			fprintf(output, "%d_Cache_WriteBack: %lu\n", this->level, this->get_cache_writeback());
		}
		utils_t::largeSeparator(output);
	}
	if(close) fclose(output);
}
