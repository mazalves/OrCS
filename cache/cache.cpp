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
	// *tag >>= utils_t::get_power_of_two(n_sets);
}

inline int32_t cache_t::getDirectoryLine(directory_t directory, uint32_t idx, uint64_t tag) {
	int32_t line = POSITION_FAIL;
	for (size_t i = 0; i < directory.sets[idx].n_lines; i++) {
		if (directory.sets[idx].lines[i][2].tag == tag) {
			line = i;
			break;
		}
	}
	return line;
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

inline int32_t cache_t::getCacheLine(uint32_t idx, uint64_t tag) { //TODO line as int32_t
	int32_t line = POSITION_FAIL;
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		if (this->sets[idx].lines[i].tag == tag) {
			line = i;
			break;
		}
	}
	return line;
}

// Reads a cache, updates cycles and return HIT or MISS status
uint32_t cache_t::read(uint64_t address,uint32_t &ttc){
    uint32_t idx;
    uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	uint32_t line = this->getCacheLine(idx, tag);

	if ((int32_t)line != POSITION_FAIL) {
		if (this->sets[idx].lines[line].ready_at <= orcs_engine.get_global_cycle()) {
			if (PREFETCHER_ACTIVE) {
				if (this->sets[idx].lines[line].prefetched == 1) {
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					this->sets[idx].lines[line].prefetched = 0;
				}
			}
			this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();
			ttc += this->latency;
		} else {
			if (PREFETCHER_ACTIVE) {
				if (this->sets[idx].lines[line].prefetched == 1) {
					orcs_engine.cacheManager->prefetcher->add_latePrefetches();
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					uint32_t latePrefetcher = orcs_engine.cacheManager->prefetcher->get_totalCycleLate() + this->sets[idx].lines[line].ready_at - orcs_engine.get_global_cycle();
					orcs_engine.cacheManager->prefetcher->set_totalCycleLate(latePrefetcher);
					this->sets[idx].lines[line].prefetched = 0;
				}
			}
			ttc+=(this->sets[idx].lines[line].ready_at - orcs_engine.get_global_cycle());
			this->sets[idx].lines[line].lru = ttc;
		}
		return HIT;
	}
	ttc += this->latency;
	return MISS;
}

// Returns the minor LRU line
inline uint32_t cache_t::searchLru(uint32_t idx) {
	uint32_t index = 0;
	for (uint32_t i = 1; i < this->sets[idx].n_lines; i++) {
		index = (this->sets[idx].lines[index].lru <= this->sets[idx].lines[i].lru) ? index : i;
	}
	return index;
}

//TODO criar uma função apenas para invalidar e lidar com o diretorio!

// Writebacks an address from a specific cache to its next lower leveL
inline void cache_t::eviction(directory_t directory, uint32_t idx, uint32_t line) {
	CACHE_DEBUG_PRINTF("Writeback in address %lu (Tag: %lu) in cache level %u. DIRTY = %u\n", this->sets[idx].lines[line].address, this->sets[idx].lines[line].tag, this->level, this->sets[idx].lines[line].dirty);
	uint32_t idx_next_level, wb_status = 0;
	uint64_t tag;
	CACHE_DEBUG_PRINTF("Searching for address in directory... ");
	this->tagIdxSetCalculation(this->sets[idx].lines[line].address, &idx_next_level, &tag, 4096, this->offset); //TODO remober o 4096 feio!!!
	uint32_t line_next_level = this->getDirectoryLine(directory, idx_next_level, tag); //TODO renomear para VIA do diretorio e não linha

	CACHE_DEBUG_PRINTF("Found in set %u and way %u.\n", idx_next_level, line_next_level);
	if (this->sets[idx].lines[line].directory_line == NULL) {
		CACHE_DEBUG_PRINTF("Cache pointer to directory is NULL. Cache TAG: %lu\n", this->sets[idx].lines[line].tag);
	}

	if (this->level == POINTER_LEVELS - 1) {
		// TODO: writeback to memory
		CACHE_DEBUG_PRINTF("In cache LLC:  ");
		for (uint32_t i = 0; i < POINTER_LEVELS - 1; i++) { //TODO BEM GRANDE: FAZER A ESCRITA NA DRAM!!!!!!!!
			if (directory.sets[idx_next_level].lines[line_next_level][i].cache_line != NULL) {
				CACHE_DEBUG_PRINTF("Cleaning directory and cache lines. Nulling pointers.\n");
				// directory.sets[idx_next_level].lines[line_next_level][i].cache_line->clean_line();
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->directory_line = NULL;
				// directory.sets[idx_next_level].lines[line_next_level][i].clean_line();
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line = NULL;
			}
			this->add_cache_writeback();
		}
	} else {
		if (this->sets[idx].lines[line].dirty == 1) {
			for (uint32_t i = 0; i < this->level; i++) {
				if (directory.sets[idx_next_level].lines[line_next_level][i].cache_line != NULL) {
					if (directory.sets[idx_next_level].lines[line_next_level][i].cache_line->dirty == 1) {
						if (wb_status == 0) {
							directory.sets[idx_next_level].lines[line_next_level][this->level+1].cache_line->dirty = directory.sets[idx_next_level].lines[line_next_level][i].cache_line->dirty;
							directory.sets[idx_next_level].lines[line_next_level][this->level+1].cache_line->lru = orcs_engine.get_global_cycle();
							directory.sets[idx_next_level].lines[line_next_level][this->level+1].cache_line->ready_at = directory.sets[idx_next_level].lines[line_next_level][i].cache_line->ready_at;
							wb_status = 1;
						}
					}
					directory.sets[idx_next_level].lines[line_next_level][i].cache_line->directory_line = NULL;
					directory.sets[idx_next_level].lines[line_next_level][i].cache_line = NULL;
				}
			}
			if (wb_status == 0) {
				for (uint32_t i = this->level + 1; i < POINTER_LEVELS; i++) {
					directory.sets[idx_next_level].lines[line_next_level][i].cache_line->dirty = this->sets[idx].lines[line].dirty;
					directory.sets[idx_next_level].lines[line_next_level][i].cache_line->lru = orcs_engine.get_global_cycle();
					directory.sets[idx_next_level].lines[line_next_level][i].cache_line->ready_at = this->sets[idx].lines[line].ready_at;
				}
			}
			this->add_cache_writeback();
		}
		this->sets[idx].lines[line].directory_line = NULL;
		directory.sets[idx_next_level].lines[line_next_level][this->level].cache_line = NULL;
	}
}

// Searches for a cache line to write data
line_t* cache_t::installLine(uint64_t address, uint32_t latency, directory_t directory, uint32_t &idx, int32_t &line, uint64_t &tag) {
	this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	line = this->getInvalidLine(idx);

	if (line == POSITION_FAIL) {
		line = this->searchLru(idx);
		CACHE_DEBUG_PRINTF("No line found, best LRU in line %u\n", line);
		this->add_change_line();
		this->eviction(directory, idx, line);
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
	uint32_t idx, idx_padding, aux_idx;
	int32_t line, line_padding, aux_line;
	uint64_t tag, tag_padding, aux_tag;
	this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	line = this->getCacheLine(idx, tag);
	this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();

	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line não encontrada para retorno") 
	line_t *line_return = NULL;
	line_return = cache->installLine(address, this->latency, directory, idx_padding, line_padding, tag_padding);

	CACHE_DEBUG_PRINTF("Searching address %lu in directory... ", address);
	this->tagIdxSetCalculation(address, &aux_idx, &aux_tag, 4096, this->offset);
	aux_line = this->getDirectoryLine(directory, aux_idx, aux_tag);
	
	CACHE_DEBUG_PRINTF("Found in set %u and line %u\n", aux_idx, aux_line);
	if (this->sets[idx].lines[line].directory_line == NULL) {
		printf("DIRETÓRIO NULO\n");
	}
	// CACHE_DEBUG_PRINTF("Updating cache level %u with cache level %u info and setting pointers between cache and directory.\n", cache->level, this->level);
	line_return->directory_line = &directory.sets[aux_idx].lines[aux_line][this->level - 1];
	line_return->directory_line->cache_line = line_return;

	line_return->dirty = this->sets[idx].lines[line].dirty;
	line_return->lru = this->sets[idx].lines[line].lru;
	line_return->prefetched = this->sets[idx].lines[line].prefetched;
	line_return->ready_at = orcs_engine.get_global_cycle();

	line_return->directory_line->cache_line = line_return;
	line_return->directory_line->shared = 1;
	line_return->directory_line->cache_status = CACHED;
	line_return->directory_line->id = cache_type;
	line_return->directory_line->tag = line_return->tag;

	directory.sets[aux_idx].lines[aux_line][this->level - 1].cache_line = line_return;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].shared = 1;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].cache_status = CACHED;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].id = cache_type;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].tag = line_return->tag;
}

// write address
uint32_t cache_t::write(uint64_t address, directory_t directory) { //TODO fazer um merge do write com install??? Como ???
	// printf("write address %lu in level %u\n", address, this->level);
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	uint32_t line = this->getCacheLine(idx, tag);
	this->add_cache_write();

	if ((int32_t)line == POSITION_FAIL) {
        line = this->searchLru(idx);
        this->add_change_line();
		this->eviction(directory, idx, line);
    }
	ERROR_ASSERT_PRINTF((int32_t)line != POSITION_FAIL, "Error, line did not found to be written.")
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
