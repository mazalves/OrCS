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
	printf("writeback address %lu; dirty: %u\n", this->sets[idx].lines[line].address, this->sets[idx].lines[line].dirty);
	uint32_t idx_next_level, line_next_level = POSITION_FAIL;
	uint64_t tag;
	this->tagIdxSetCalculation(this->sets[idx].lines[line].address, &idx_next_level, &tag, 4096, this->offset);
	for (size_t i = 0; i < directory.sets[idx_next_level].n_lines; i++) {
		if (directory.sets[idx_next_level].lines[i][2].tag == tag) {
			line_next_level = i;
			break;
		}
	}
	// this->sets[idx].lines[line].directory_line->cache_line->dirty;
	// if (this->sets[idx].lines[line].address == 0) {
	// 	printf("entrou no if do endereço 0\n");
	// 	printf("aponta para diretório: %lu %lu\n", this->sets[idx].lines[line].directory_line->tag, this->sets[idx].lines[line].directory_line->cache_line->tag);
	// }
	printf("tag cache: %lu\n", this->sets[idx].lines[line].tag);
	if (this->sets[idx].lines[line].directory_line == NULL) 
		printf("DIRETÓRIO NULO!\n");
	printf("tag diretorio: %lu\n", this->sets[idx].lines[line].directory_line->cache_line->tag);
	if (this->level == 0) {
		if (this->sets[idx].lines[line].dirty == 1) {
			// printf("%lu dirty no nível %u: %u\n", this->sets[idx].lines[line].address, this->level, this->sets[idx].lines[line].dirty);
			for (uint32_t i = 1; i < POINTER_LEVELS; i++) {
				// printf("endereço das outras caches %u: %lu\n", i, directory.sets[idx_next_level].lines[line_next_level][i].cache_line->address);
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->dirty = this->sets[idx].lines[line].dirty;
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->ready_at = this->sets[idx].lines[line].ready_at;
				// printf("dirty nas outras caches %u: %u\n", i, directory.sets[idx_next_level].lines[line_next_level][i].cache_line->dirty);
			}
			// directory.sets[idx_next_level].lines[line_next_level][this->level].clean_line();
		}
		directory.sets[idx_next_level].lines[line_next_level][this->level].cache_line = NULL;
	} else if (this->level == POINTER_LEVELS - 1) {
		for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
			if (directory.sets[idx_next_level].lines[line_next_level][i].cache_line != NULL) {
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line->directory_line = NULL;
				// directory.sets[idx].lines[line][i].cache_line->clean_line();
				directory.sets[idx_next_level].lines[line_next_level][i].cache_line = NULL;
				// directory.sets[idx].lines[line][i].clean_line();
			}
		}
	} else {
		if (directory.sets[idx_next_level].lines[line_next_level][0].cache_line != NULL) {
			if (directory.sets[idx_next_level].lines[line_next_level][0].cache_line->dirty == 1) {
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->dirty = directory.sets[idx_next_level].lines[line_next_level][0].cache_line->dirty;
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->ready_at = directory.sets[idx_next_level].lines[line_next_level][0].cache_line->ready_at;
			} 
			else {
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->dirty = directory.sets[idx_next_level].lines[line_next_level][1].cache_line->dirty;
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->ready_at = directory.sets[idx_next_level].lines[line_next_level][1].cache_line->ready_at;
			}
			directory.sets[idx_next_level].lines[line_next_level][0].cache_line->directory_line = NULL;
			// directory.sets[idx_next_level].lines[line_next_level][0].cache_line->clean_line();
			directory.sets[idx_next_level].lines[line_next_level][0].cache_line = NULL;
			// directory.sets[idx_next_level].lines[line_next_level][0].clean_line();
		} else {
			if (this->sets[idx].lines[line].dirty == 1) {
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->dirty = this->sets[idx].lines[line].dirty;
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->lru = orcs_engine.get_global_cycle();
				directory.sets[idx_next_level].lines[line_next_level][2].cache_line->ready_at = this->sets[idx].lines[line].ready_at;
			}
		}
		directory.sets[idx_next_level].lines[line_next_level][this->level].cache_line = NULL;
		// directory.sets[idx_next_level].lines[line_next_level][this->level].clean_line();
	}
	if (this->sets[idx].lines[line].directory_line != NULL) {
		this->sets[idx].lines[line].directory_line = NULL;
		this->sets[idx].lines[line].clean_line();
	}
	// directory.sets[idx_next_level].lines[line_next_level][this->level].cache_line = NULL;
	this->add_cache_writeback();
}

// Searches for a cache line to write data
line_t* cache_t::installLine(uint64_t address, uint32_t latency, directory_t directory, uint32_t &idx, uint32_t &line, uint64_t &tag) {
	printf("install address %lu in cache level %u\n", address, this->level);
	line = POSITION_FAIL;
	this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
		if (this->sets[idx].lines[i].valid == 0) {
			line = i;
            break;
		}
	}
	if ((int)line == POSITION_FAIL) {
		line = this->searchLru(&this->sets[idx]);
		this->add_change_line();
		this->writeBack(directory, idx, line);
	}
	// printf("address %lu with tag %lu in level %u\n", address, tag, this->level);
	this->sets[idx].lines[line].address = address;
	this->sets[idx].lines[line].tag = tag;
	this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle() + latency;
	this->sets[idx].lines[line].valid = 1;
	this->sets[idx].lines[line].dirty = 0;
	this->sets[idx].lines[line].prefetched = 0;
	this->sets[idx].lines[line].ready_at = orcs_engine.get_global_cycle() + latency;
	this->add_cache_eviction();
	printf("install cache line address %lu tag %lu\n", this->sets[idx].lines[line].address, this->sets[idx].lines[line].tag);

	return &this->sets[idx].lines[line];
}

// Selects a cache line to install an address and points this memory address with the other cache pointers
void cache_t::returnLine(uint64_t address, cache_t *cache, directory_t directory, cacheId_t cache_type) {
	printf("returnLine level %u address: %lu \n", this->level, address);
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

	uint32_t aux_idx, aux_line = POSITION_FAIL;
	uint64_t aux_tag;
	this->tagIdxSetCalculation(address, &aux_idx, &aux_tag, 4096, this->offset);
	for (uint32_t i = 0; i < directory.sets[aux_idx].n_lines; i++) {
		// printf("cache in directory %lu address directory %lu %lu tag original %lu address %lu\n", directory.sets[aux_idx].lines[i][2].cache_line->tag, directory.sets[aux_idx].lines[i][2].cache_line->address, directory.sets[aux_idx].lines[i][2].tag, aux_tag, address);
		if (directory.sets[aux_idx].lines[i][2].tag == aux_tag) {
			// printf("directory line %u address %lu\n", i, directory.sets[aux_idx].lines[i][2].tag);
			aux_line = i;
			break;
		}
	}
	printf("Address %lu intalled! accessing directory line: %u idx: %u\n", line_return->address, aux_line, aux_idx);
	line_return->dirty = this->sets[idx].lines[line].dirty;
	line_return->lru = this->sets[idx].lines[line].lru;
	line_return->prefetched = this->sets[idx].lines[line].prefetched;
	// line_return->address = this->sets[idx].lines[line].address;
	line_return->ready_at = orcs_engine.get_global_cycle();

	directory.sets[aux_idx].lines[aux_line][this->level - 1].cache_line = line_return;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].shared = 1;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].cache_status = CACHED;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].id = cache_type;
    directory.sets[aux_idx].lines[aux_line][this->level - 1].tag = line_return->tag;
	// directory.sets[aux_idx].lines[aux_line][this->level - 1].cache_line = line_return;
	line_return->directory_line = &directory.sets[aux_idx].lines[aux_line][this->level - 1];
	printf("dir address: %lu tag: %lu || laddress: %lu tag: %lu\n", directory.sets[aux_idx].lines[aux_line][this->level-1].cache_line->address, directory.sets[aux_idx].lines[aux_line][this->level-1].tag, line_return->directory_line->cache_line->address, line_return->directory_line->tag);
}

// write address
uint32_t cache_t::write(uint64_t address, directory_t directory) {
	printf("write address %lu in level %u\n", address, this->level);
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag, this->n_sets, this->offset);
	int32_t line = POSITION_FAIL;
	this->add_cache_write();
    for (size_t i = 0; i < this->sets->n_lines; i++) {
		printf("in set %u line %lu address %lu tag %lu\n", idx, i, this->sets[idx].lines[i].address, this->sets[idx].lines[i].tag);
		if(this->sets[idx].lines[i].tag == tag){
			line = i;
			break;
		}
	}
	if (line == POSITION_FAIL) {
        line = this->searchLru(&this->sets[idx]);
		printf("write in address %lu, %u line %u\n", this->sets[idx].lines[line].address, this->sets[idx].lines[line].dirty, line);
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
