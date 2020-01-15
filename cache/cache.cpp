#include "../simulator.hpp"

#ifdef CACHE_DEBUG
#define CACHE_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
#define CACHE_DEBUG_PRINTF(...)
#endif

// Constructor
cache_t::cache_t() {
	this->id = NAC;
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
}

cache_t::~cache_t() {
	if (orcs_engine.get_global_cycle() == 0)
		return;
	for (size_t i = 0; i < this->n_sets; i++) {
		for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
			//delete this->sets[i].lines[j].directory_line;
		}
		delete[] this->sets[i].lines;
	}
	delete[] sets;
	//ORCS_PRINTF ("cycle: %lu\n", orcs_engine.get_global_cycle())
}

// Allocate each cache type
void cache_t::allocate(uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS, uint32_t *ICACHE_AMOUNT, uint32_t *DCACHE_AMOUNT) {
	// Access configure file
	libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();

	// Get prefetcher info
	libconfig::Setting &prefetcher_defs = cfg_root["PREFETCHER"];
	set_PREFETCHER_ACTIVE(prefetcher_defs["PREFETCHER_ACTIVE"]);

	// Get general cache info
	libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"]["CONFIG"];
	set_WAIT_CYCLE(cfg_cache_defs["WAIT_CYCLE"]);

	set_INSTRUCTION_LEVELS(INSTRUCTION_LEVELS);
	set_DATA_LEVELS(DATA_LEVELS);
	POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);

	uint32_t line_number = this->size/this->LINE_SIZE;
	uint32_t total_sets = line_number/associativity;

	this->offset_bits_shift = 0;
    this->index_bits_shift = utils_t::get_power_of_two(this->get_LINE_SIZE());
    this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(total_sets);
	
	uint64_t i;
    /// OFFSET MASK
    for (i = 0; i < utils_t::get_power_of_two(this->get_LINE_SIZE()); i++) {
        this->offset_bits_mask |= 1 << i;
    }
    
    /// INDEX MASK
    for (i = 0; i < utils_t::get_power_of_two(total_sets); i++) {
        this->index_bits_mask |= 1 << (i + index_bits_shift);
    }

    /// TAG MASK
    for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->tag_bits_mask |= 1 << i;
    }
	
	this->sets = new cacheSet_t[this->n_sets];
	for (size_t i = 0; i < this->n_sets; i++) {
		this->sets[i].allocate(this->associativity, INSTRUCTION_LEVELS, DATA_LEVELS, ICACHE_AMOUNT, DCACHE_AMOUNT);
	}
    this->set_cache_access(0);
    this->set_cache_hit(0);
    this->set_cache_miss(0);
	this->set_cache_eviction(0);
	this->set_cache_read(0);
	this->set_cache_write(0);
	this->set_cache_writeback(0);
}

inline uint32_t cache_t::is_LLC() {
	if (this->level == POINTER_LEVELS - 1) {
		return 1;
	}
	return 0;
}

// Return address index in cache
inline void cache_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag) {
	uint32_t get_bits = (this->n_sets) - 1;
	*tag = (address >> this->offset); //TODO tirar o 6
	*idx = *tag & get_bits;
	*tag >>= utils_t::get_power_of_two(this->n_sets);
	//printf("tag: %lu idx: %lu\n", get_tag(address), get_index(address));
}

void cache_t::printTagIdx (uint64_t address){
	uint32_t get_bits = (this->n_sets) - 1;
	uint64_t tag = (address >> this->offset);
	uint32_t idx = tag & get_bits;
	tag >>= utils_t::get_power_of_two(this->n_sets);
	printf("tag: %lu idx: %u\n", tag, idx);
}

inline int32_t cache_t::getInvalidLine(uint32_t idx) {
	int32_t line = POSITION_FAIL;
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		if (this->sets[idx].lines[i].valid == 0) {
			line = i;
			break;
		}
	}
	return line;
}

inline int32_t cache_t::getCacheLine(uint32_t idx, uint64_t tag) {
	int32_t line = POSITION_FAIL;
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		if (this->sets[idx].lines[i].tag == tag) {
			line = i;
			break;
		}
	}
	return line;
}

// Returns the minor LRU line
inline int32_t cache_t::searchLru(uint32_t idx) {
	int32_t index = 0;
	for (uint32_t i = 1; i < this->sets[idx].n_lines; i++) {
		index = (this->sets[idx].lines[index].lru <= this->sets[idx].lines[i].lru) ? index : i;
	}
	return index;
}

// TODO: fazer com que o read retorne o endereço da linha de cache que vai ser modificada
// TODO: diminuir número de cálculos de indice e tag nas funções da cache
// TODO: no futuro, quando o diretório estiver pronto, passar a fazer a busca do endereço direto nele e contabilizar as latências de cache para otimizar a busca
uint32_t cache_t::read(uint64_t address, uint32_t &ttc) {
	this->add_cache_access();
	uint32_t idx;
	uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	int32_t line = this->getCacheLine(idx, tag);
	CACHE_DEBUG_PRINTF("Reading address %lu (TAG: %lu - index %u) in %s %s: ", address, tag, idx, get_enum_cache_id_char(this->id), get_cache_level_char(this->level));
	if (line != POSITION_FAIL) {
		if (this->sets[idx].lines[line].ready_at <= orcs_engine.get_global_cycle()) {
			if (PREFETCHER_ACTIVE) {
				if (this->sets[idx].lines[line].prefetched == 1) {
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					this->sets[idx].lines[line].prefetched = 0;
				}
			}
			this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();
			ttc += this->latency;
		}
		else {
			if (PREFETCHER_ACTIVE) {
				if (this->sets[idx].lines[line].prefetched == 1) {
					orcs_engine.cacheManager->prefetcher->add_latePrefetches();
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					uint32_t latePrefetcher = orcs_engine.cacheManager->prefetcher->get_totalCycleLate() + this->sets[idx].lines[line].ready_at - orcs_engine.get_global_cycle();
					orcs_engine.cacheManager->prefetcher->set_totalCycleLate(latePrefetcher);
					this->sets[idx].lines[line].prefetched = 0;
				}
			}
			ttc += (this->sets[idx].lines[line].ready_at - orcs_engine.get_global_cycle());
			this->sets[idx].lines[line].lru = ttc;
		}
		CACHE_DEBUG_PRINTF("HIT!\n");
		this->add_cache_hit();
		return HIT;
	}
	ttc += this->latency;
	CACHE_DEBUG_PRINTF("MISS!\n");
	this->add_cache_miss();
	return MISS;
}

void cache_t::sendMemoryRequest(uint64_t address, memory_operation_t mem_op) {
	CACHE_DEBUG_PRINTF("Returns address %lu to DRAM\n", address);
	memory_package_t *wb_mob_line = new memory_package_t;
	wb_mob_line->memory_address = address;
	wb_mob_line->memory_operation = mem_op;
	wb_mob_line->status = PACKAGE_STATE_UNTREATED;
	wb_mob_line->is_hive = false;
	wb_mob_line->readyAt = orcs_engine.get_global_cycle();
	orcs_engine.memory_controller->requestDRAM(wb_mob_line, wb_mob_line->memory_address);
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
void cache_t::writeBack(line_t *line) {
	for (uint32_t i = this->level + 1; i < DATA_LEVELS - 1; i++) {
		ERROR_ASSERT_PRINTF(line->line_ptr_caches[0][i] != NULL, "Error, no line reference in next levels.")
	}
	// L1 writeBack issues
	if (this->level == 0) {
		// printf("LVL1 WB DIR: %u\n", this->sets[idx].lines[access_line].directory_line->level);
		// this->sets[idx].lines[access_line].directory_line->cache_status = UNCACHED;
		for (uint32_t i = 1; i < DATA_LEVELS; i++) {
			this->copyNextLevels(line, i);
			line->line_ptr_caches[0][i]->line_ptr_caches[0][this->level] = NULL; //Pointer to Lower Level
		}
		line->clean_line();
		// LLC writeBack issues
	} else if (this->level == DATA_LEVELS - 1) {
		// printf("LVL3 WB DIR: %u\n", this->sets[idx].lines[access_line].directory_line->level);
		// this->sets[idx].lines[access_line].directory_line->cache_status = UNCACHED;
		// for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
		// 	directory->sets[idx].lines[access_line][i].clean_line();
		// }
		for (uint32_t i = 0; i < DATA_LEVELS - 1; i++) {
			if (line->line_ptr_caches[0][i] != NULL) {
				line->line_ptr_caches[0][i]->clean_line();
			}
		}
		// this->sendMemoryRequest(address, mem_op);
		// Intermediate cache levels issues
	} else {
		// printf("LVL2 WB DIR: %u\n", this->sets[idx].lines[access_line].directory_line->level);
		// this->sets[idx].lines[access_line].directory_line->cache_status = UNCACHED;
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

// uint32_t cache_t::checkUpperLevels(line_t *cache_line) {
// 	uint32_t wb_status = 0;
// 	for (uint32_t i = 0; i < this->level; i++) {
// 		if (cache_line->directory_line->cache_level[i].cache_way->directory_line != NULL) {
// 			if (cache_line->directory_line->cache_level[i].cache_way->dirty == 1) {
// 				if (wb_status == 0) {
// 					cache_line->directory_line->cache_level[this->level + 1].cache_way->dirty = cache_line->directory_line->cache_level[i].cache_way->dirty;
// 					cache_line->directory_line->cache_level[this->level + 1].cache_way->lru = orcs_engine.get_global_cycle();
// 					cache_line->directory_line->cache_level[this->level + 1].cache_way->ready_at = cache_line->directory_line->cache_level[i].cache_way->ready_at;
// 					wb_status = 1;
// 				}
// 			}
// 			cache_line->directory_line->cache_level[i].cache_status = UNCACHED;
// 			cache_line->directory_line->cache_level[i].cache_way->directory_line = NULL;
// 		}
// 	}
// 	return wb_status;
// }

// void cache_t::writeback(line_t *cache_line, memory_operation_t mem_op) {
// 	CACHE_DEBUG_PRINTF("Writeback address %lu (TAG %lu) in %s %s\n", cache_line->address, cache_line->tag, get_enum_cache_id_char(this->id), get_cache_level_char(this->level));

// 	if (this->is_LLC()) {
// 		CACHE_DEBUG_PRINTF("Dirty in LLC: ...")
// 		this->sendMemoryRequest(cache_line->address, mem_op);
// 		for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
// 			if (cache_line->directory_line != NULL) {
// 				cache_line->directory_line->cache_level[i].cache_way->clean_line();
// 			}
// 		}
// 	} else {
// 		CACHE_DEBUG_PRINTF("Dirty in level %s ...", get_cache_level_char(this->level));
// 		uint32_t wb_status = this->checkUpperLevels(cache_line);
// 		if (wb_status == 0) {
// 			for (uint32_t i = this->level + 1; i < POINTER_LEVELS; i++) {
// 				cache_line->directory_line->cache_level[i].cache_way->dirty = cache_line->dirty;
// 				cache_line->directory_line->cache_level[i].cache_way->lru = orcs_engine.get_global_cycle();
// 				cache_line->directory_line->cache_level[i].cache_way->ready_at = cache_line->ready_at;
// 			}
// 		}
// 		cache_line->directory_line->cache_level[this->level].cache_status = UNCACHED;
// 		cache_line->directory_line = NULL;
// 	}
// 	this->add_cache_writeback();
// }

// void cache_t::eviction(uint32_t idx, int32_t line, memory_operation_t mem_op) {
// 	CACHE_DEBUG_PRINTF("Evicting address %lu (TAG %lu - index %u - line %d) in %s %s\n", this->sets[idx].lines[line].address, this->sets[idx].lines[line].tag, idx, line, get_enum_cache_id_char(this->id), get_cache_level_char(this->level));

// 	if (this->sets[idx].lines[line].directory_line == NULL) {
// 		CACHE_DEBUG_PRINTF("Cache pointer to directory is NULL. Cache TAG: %lu\n", this->sets[idx].lines[line].tag);
// 	}

// 	if (this->sets[idx].lines[line].dirty == 1) {
// 		this->writeback(&this->sets[idx].lines[line], mem_op);
// 	} else {
// 		if (this->is_LLC()) {
// 			CACHE_DEBUG_PRINTF("Not dirty in LLC ...\n");
// 			for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
// 				if (this->sets[idx].lines[line].directory_line != NULL) {
// 					this->sets[idx].lines[line].directory_line = NULL;
// 				}
// 			}
// 		} else {
// 			CACHE_DEBUG_PRINTF("Not dirty in %s ...\n", get_cache_level_char(this->level));
// 			[[maybe_unused]] static uint32_t wb_status = this->checkUpperLevels(&this->sets[idx].lines[line]);
// 			if (this->sets[idx].lines[line].directory_line != NULL) {
// 				this->sets[idx].lines[line].directory_line->cache_level[this->level].cache_status = UNCACHED;
// 				this->sets[idx].lines[line].directory_line = NULL;
// 			}
// 		}
// 	}
// }

line_t *cache_t::installLine(uint64_t address, uint32_t latency, uint32_t &idx, uint32_t &line, memory_operation_t mem_op) {
	uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	line = this->getInvalidLine(idx);
	if ((int)line == POSITION_FAIL) {
		line = this->searchLru(idx);
		// this->eviction(idx, line, mem_op);
		if (this->sets[idx].lines[line].dirty == 1) {
			this->writeBack(&this->sets[idx].lines[line]);
			this->add_cache_writeback();
		}
	}
	this->sets[idx].lines[line].address = address;
	this->sets[idx].lines[line].tag = tag;
	this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle() + latency;
	this->sets[idx].lines[line].valid = 1;
	this->sets[idx].lines[line].dirty = 0;
	this->sets[idx].lines[line].prefetched = 0;
	this->sets[idx].lines[line].ready_at = orcs_engine.get_global_cycle() + latency;
	this->add_cache_eviction();

	if (mem_op == MEMORY_OPERATION_WRITE && this->level == 0) {
		this->write(idx, line);
	}
	return &this->sets[idx].lines[line];
}

// Selects a cache line to install an address and points this memory address with the other cache pointers
// TODO: retornar a linha de cache automaticamente sem o cache manager precisar mandar
void cache_t::returnLine(memory_package_t *mob_line, cache_t *cache) {
	uint32_t idx, idx_padding, line_padding;
	uint64_t tag, address = 0;
	if (mob_line->memory_operation == MEMORY_OPERATION_INST) {
		address = mob_line->opcode_address;
	} else {
		address = mob_line->memory_address;
	}
	this->tagIdxSetCalculation(address, &idx, &tag);
	int32_t line = getCacheLine(idx, tag);
	// Selects a line in this cache
	this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();
	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line não encontrada para retorno")

	line_t *line_return = NULL;
	line_return = cache->installLine(address, this->latency, idx_padding, line_padding, mob_line->memory_operation);
	// line_return->directory_line->cache_status = CACHED;

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

// write address
void cache_t::write(uint32_t idx, int32_t line) {
	this->sets[idx].lines[line].dirty = 1;
	if (this->sets[idx].lines[line].ready_at <= orcs_engine.get_global_cycle()) {
		this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();
	} else {
		this->sets[idx].lines[line].lru = this->sets[idx].lines[line].ready_at + this->latency;
	}
}

void cache_t::statistics() {
	FILE *output = stdout;
	bool close = false;
	if (orcs_engine.output_file_name != NULL) {
		close = true;
		output = fopen(orcs_engine.output_file_name, "a+");
	}
	if (output != NULL) {
		utils_t::largeSeparator(output);
		fprintf(output, "Cache_Level: %d - Cache_Type: %u\n", this->level, this->id);
		fprintf(output, "%d_Cache_Access: %lu\n", this->level, this->get_cache_access());
		fprintf(output, "%d_Cache_Hits: %lu\n", this->level, this->get_cache_hit());
		fprintf(output, "%d_Cache_Miss: %lu\n", this->level, this->get_cache_miss());
		fprintf(output, "%d_Cache_Eviction: %lu\n", this->level, this->get_cache_eviction());
		fprintf(output, "%d_Cache_Read: %lu\n", this->level, this->get_cache_read());
		fprintf(output, "%d_Cache_Write: %lu\n", this->level, this->get_cache_write());
		if (this->get_cache_writeback() != 0) {
			fprintf(output, "%d_Cache_WriteBack: %lu\n", this->level, this->get_cache_writeback());
		}
		utils_t::largeSeparator(output);
	}
	if (close)
		fclose(output);
}
