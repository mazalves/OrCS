#include "../simulator.hpp"

#ifdef CACHE_DEBUG
	#define CACHE_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
	#define CACHE_DEBUG_PRINTF(...)
#endif

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
void cache_t::allocate(uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS) {
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
			this->sets[i].lines[j].directory_line = new directory_line_t;
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
inline void cache_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag, uint32_t n_sets, uint32_t offset) {
	uint32_t get_bits = (n_sets) - 1;
	*tag = (address >> offset);
	*idx = *tag & get_bits;
	*tag >>= utils_t::get_power_of_two(n_sets);
}

// Reads a cache, updates cycles and return HIT or MISS status
uint32_t cache_t::read(uint64_t address,uint32_t &ttc){
    uint32_t idx;
    uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
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

// Writebacks an address from a specific cache to its next lower leveL
inline void cache_t::writeBack(directory_t directory, uint32_t idx, uint32_t line) {
	CACHE_DEBUG_PRINTF("Writeback in address %lu in cache level %u. DIRTY = %u\n", this->sets[idx].lines[line].address, this->level, this->sets[idx].lines[line].dirty);
	uint32_t idx_next_level, line_next_level = POSITION_FAIL;
	uint64_t tag;
	CACHE_DEBUG_PRINTF("Searching for address in directory... ");
	this->tagIdxSetCalculation(this->sets[idx].lines[line].address, &idx_next_level, &tag, 4096, this->offset);
	for (size_t i = 0; i < directory.sets[idx_next_level].n_lines; i++) {
		if (directory.sets[idx_next_level].lines[i][2].tag == tag) {
			line_next_level = i;
			break;
		}
	}
	CACHE_DEBUG_PRINTF("Found in set %u and line %u. TAG = %lu\n", idx_next_level, line_next_level, directory.sets[idx_next_level].lines[line_next_level][2].tag);
	if (this->sets[idx].lines[line].directory_line == NULL) {
		CACHE_DEBUG_PRINTF("Cache pointer to directory is NULL. Cache TAG: %lu\n", this->sets[idx].lines[line].tag);
	}
	if (this->level == 0) {
		CACHE_DEBUG_PRINTF("In cache L1:  ");
		if (this->sets[idx].lines[line].dirty == 1) {
			CACHE_DEBUG_PRINTF("Dirty, writeback\n");
			for (uint32_t i = 1; i < POINTER_LEVELS; i++) {
				if (directory.sets[idx_next_level].lines[line_next_level][i].cache_line == NULL) {
					CACHE_DEBUG_PRINTF("Directory pointer to cache level %u is NULL. Directory TAG: %lu\n", i, directory.sets[idx_next_level].lines[line_next_level][i].tag);
				}
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->dirty = this->sets[idx].lines[line].dirty;
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->ready_at = this->sets[idx].lines[line].ready_at;
			}
			this->add_cache_writeback();
		}
		CACHE_DEBUG_PRINTF("Cleaning directory line and nulling pointer to cache\n");
		directory.sets[idx_next_level].lines[line_next_level][this->level].clean_line();
	} else if (this->level == POINTER_LEVELS - 1) {
		CACHE_DEBUG_PRINTF("In cache LLC:  ");
		for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
			if (directory.sets[idx_next_level].lines[line_next_level][i].cache_line != NULL) {
				CACHE_DEBUG_PRINTF("Cleaning directory and cache lines. Nulling pointers.\n");
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->clean_line();
				directory.sets[idx_next_level].lines[line_next_level][i].clean_line();
			}
		}
		this->add_cache_writeback();
	} else {
		CACHE_DEBUG_PRINTF("In cache L2:  ")
		if (directory.sets[idx_next_level].lines[line_next_level][0].cache_line != NULL) {
			if (directory.sets[idx_next_level].lines[line_next_level][0].cache_line->dirty == 1) {
				CACHE_DEBUG_PRINTF("If L1 is dirty -> copy L1 info to LLC ");
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->dirty = directory.sets[idx_next_level].lines[line_next_level][0].cache_line->dirty;
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->ready_at = directory.sets[idx_next_level].lines[line_next_level][0].cache_line->ready_at;
				this->add_cache_writeback();
			} 
			else {
				CACHE_DEBUG_PRINTF("If L1 is not dirty -> copy L2 info to LLC ");
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->dirty = directory.sets[idx_next_level].lines[line_next_level][1].cache_line->dirty;
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->ready_at = directory.sets[idx_next_level].lines[line_next_level][1].cache_line->ready_at;
			}
			CACHE_DEBUG_PRINTF("and clean directory and cache lines refered to cache L1 ");
			directory.sets[idx_next_level].lines[line_next_level][0].cache_line->clean_line();
			directory.sets[idx_next_level].lines[line_next_level][0].clean_line();
		} else {
			if (this->sets[idx].lines[line].dirty == 1) {
				CACHE_DEBUG_PRINTF("If just L2 is dirty, copy L2 info to LLC ");
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->dirty = this->sets[idx].lines[line].dirty;
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->ready_at = this->sets[idx].lines[line].ready_at;
				this->add_cache_writeback();
			}
		}
		CACHE_DEBUG_PRINTF("Then, clean diretory pointer to L2\n");
		directory.sets[idx_next_level].lines[line_next_level][this->level].clean_line();
	}
	CACHE_DEBUG_PRINTF("Clean directory pointer in current cache line\n");
	if (this->sets[idx].lines[line].directory_line != NULL) {
		this->sets[idx].lines[line].directory_line->clean_line();
	}
}

// Searches for a cache line to write data
line_t* cache_t::installLine(uint64_t address, uint32_t latency, directory_t directory, uint32_t &idx, uint32_t &line, uint64_t &tag) {
	// printf("install address %lu in cache level %u\n", address, this->level);
	line = POSITION_FAIL;
	this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		if (this->sets[idx].lines[i].valid == 0) {
			line = i;
            break;
		}
	}
	CACHE_DEBUG_PRINTF("address %lu can be installed in index %u and line %u. TAG: %lu\n", address, idx, line, tag);
	if ((int)line == POSITION_FAIL) {
		line = this->searchLru(&this->sets[idx]);
		CACHE_DEBUG_PRINTF("No line found, best LRU in line %u\n", line);
		this->add_change_line();
		this->writeBack(directory, idx, line);
	}

	this->sets[idx].lines[line].address = address;
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
void cache_t::returnLine(uint64_t address, cache_t *cache, directory_t directory, cacheId_t cache_type) {
	CACHE_DEBUG_PRINTF("Return address %lu from cache %u to cache %u\n", address, this->level, cache->level);	
	uint32_t idx, idx_padding, line_padding;
	uint64_t tag, tag_padding;
	this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	int32_t line = POSITION_FAIL;
	for (size_t i = 0; i < this->sets->n_lines; i++) {
		if (this->sets[idx].lines[i].tag == tag) {
			this->sets[idx].lines[i].lru = orcs_engine.get_global_cycle();
			line = i;
			break;
		}
	}
	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line não encontrada para retorno") 
	line_t *line_return = NULL;
	line_return = cache->installLine(address, this->latency, directory, idx_padding, line_padding, tag_padding);

	if (line_return != NULL) {
		CACHE_DEBUG_PRINTF("Installed address in cache\n");
	}
	uint32_t aux_idx, aux_line = POSITION_FAIL;
	uint64_t aux_tag;
	CACHE_DEBUG_PRINTF("Searching address %lu in directory... ", address);
	this->tagIdxSetCalculation(address, &aux_idx, &aux_tag, 4096, this->offset);
	for (uint32_t i = 0; i < directory.sets[aux_idx].n_lines; i++) {
		if (directory.sets[aux_idx].lines[i][2].tag == aux_tag) {
			aux_line = i;
			break;
		}
	}
	CACHE_DEBUG_PRINTF("Found in set %u and line %u\n", aux_idx, aux_line);
	CACHE_DEBUG_PRINTF("Updating cache level %u with cache level %u info and setting pointers between cache and directory.\n", cache->level, this->level);
	line_return->dirty = this->sets[idx].lines[line].dirty;
	line_return->lru = this->sets[idx].lines[line].lru;
	line_return->prefetched = this->sets[idx].lines[line].prefetched;
	line_return->ready_at = orcs_engine.get_global_cycle();

	directory.sets[aux_idx].lines[aux_line][this->level - 1].cache_line = line_return;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].shared = 1;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].cache_status = CACHED;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].id = cache_type;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].tag = line_return->tag;
	line_return->directory_line = &directory.sets[aux_idx].lines[aux_line][this->level - 1];
}

// write address
uint32_t cache_t::write(uint64_t address, directory_t directory) {
	// printf("write address %lu in level %u\n", address, this->level);
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
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
		this->writeBack(directory, idx, line);
    }
	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line did not found to be written.")
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
