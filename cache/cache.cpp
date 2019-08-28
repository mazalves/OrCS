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
    this->cache_read = 0;
    this->cache_write = 0;
    this->cache_writeback = 0;
    this->change_line = 0;
}

// Desctructor
// cache_t::~cache_t() {
// 	if(this->sets!=NULL) delete[] &sets;
// }
cache_t::~cache_t(){
	delete[] sets;
}

// Allocate each cache type (removed EMC case)
void cache_t::allocate(cacheId_t cache_type, uint32_t cache_level, uint32_t cache_size, uint32_t cache_associativity, uint32_t cache_latency) {
    // printf("-> Allocating %u cache %u\n", cache_type, cache_level);
	libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
	set_LINE_SIZE (cfg_root[0]["LINE_SIZE"]);

	set_L1_DATA_ASSOCIATIVITY (cfg_root[0]["L1_DATA_ASSOCIATIVITY"]);
    set_L1_DATA_LATENCY (cfg_root[0]["L1_DATA_LATENCY"]);
	L1_DATA_SETS = (L1_DATA_SIZE/LINE_SIZE)/L1_DATA_ASSOCIATIVITY;
        
    set_L1_INST_ASSOCIATIVITY (cfg_root[0]["L1_INST_ASSOCIATIVITY"]);
    set_L1_INST_LATENCY (cfg_root[0]["L1_INST_LATENCY"]);
	L1_INST_SETS = (L1_INST_SIZE/LINE_SIZE)/L1_INST_ASSOCIATIVITY;
        
    set_L2_ASSOCIATIVITY (cfg_root[0]["L2_ASSOCIATIVITY"]);
    set_L2_LATENCY (cfg_root[0]["L2_LATENCY"]);
	L2_SETS = (L2_SIZE/LINE_SIZE)/L2_ASSOCIATIVITY;
    // ==================== LEVEL 2 =====================
    // ==================== LLC     =====================
    set_LLC_ASSOCIATIVITY (cfg_root[0]["LLC_ASSOCIATIVITY"]);
    set_LLC_LATENCY (cfg_root[0]["LLC_LATENCY"]);
	LLC_SETS = (LLC_SIZE/LINE_SIZE)/LLC_ASSOCIATIVITY;
        
    set_PREFETCHER_ACTIVE (cfg_root[0]["PREFETCHER_ACTIVE"]);

	set_INSTRUCTION_LEVELS (cfg_root[0]["INSTRUCTION_LEVELS"]);
	set_DATA_LEVELS (cfg_root[0]["DATA_LEVELS"]);
	POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
    set_CACHE_MANAGER_DEBUG (cfg_root[0]["CACHE_MANAGER_DEBUG"]);
    set_WAIT_CYCLE (cfg_root[0]["WAIT_CYCLE"]);
	
    this->id = cache_type;
    this->level = cache_level;
    this->size = cache_size;
    this->n_sets = (cache_size / LINE_SIZE) / cache_associativity;
    this->latency = cache_latency;
    this->associativity = cache_associativity;
    this->offset = utils_t::get_power_of_two(LINE_SIZE);
    this->sets = new cacheSet_t[this->n_sets];
    for (size_t i = 0; i < this->n_sets; i++) {
        this->sets[i].lines = new line_t[this->associativity];
        this->sets[i].n_lines = this->associativity;
        for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
            this->sets[i].lines[j].allocate();
            for (uint32_t k = 0; k < NUMBER_OF_PROCESSORS; k++) {
                for (uint32_t l = 0; l < POINTER_LEVELS; l++) {
                    this->sets[i].lines[j].line_ptr_caches[k][l] = NULL;
                }
            }
            this->sets[i].lines[j].clean_line();
        }
    }
    this->set_cache_access(0);
    this->set_cache_hit(0);
    this->set_cache_miss(0);
    this->set_cache_read(0);
    this->set_cache_write(0);
    this->set_cache_writeback(0);
}

// Return address index in cache
inline void cache_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag) {
    // printf("%s\n", "-> tagIdxSetCalculation in cache.cpp");
	uint32_t get_bits = (this->n_sets) - 1;
	*tag = (address >> this->offset);
	*idx = *tag & get_bits;
}

// Reads a cache, updates cycles and return HIT or MISS status
// TODO considering SandyBridge latency differences (3-6-9-...)
uint32_t cache_t::read(uint64_t address,uint32_t &ttc){
    // printf("%s\n", "-> read in cache.cpp");
    uint32_t idx;
    uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
    // printf("    address: %lu | tag: %u | idx: %u | offset: %u\n", address, tag, idx, this->offset);
    //printf("    Searches tag in associativity\n");
	for (size_t i = 0; i < this->sets->n_lines; i++) {
		if(this->sets[idx].lines[i].tag == tag) {
            //printf("    Finds tag in line %lu, index %u", i, idx);
			// Se ready Cycle for menor que o ciclo atual, a latencia é apenas da leitura, sendo um hit.
			if (this->sets[idx].lines[i].ready_at <= orcs_engine.get_global_cycle()){
                //printf("%s\n", "    Ready cycle <= current cycle = HIT");
				if (PREFETCHER_ACTIVE){
					if (this->sets[idx].lines[i].prefetched == 1){
						orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
						this->sets[idx].lines[i].prefetched =0;
					}
				}
				this->sets[idx].lines[i].lru = orcs_engine.get_global_cycle();
				ttc += this->latency;
				if (this->id == DATA) {
					if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
						//ORCS_PRINTF("     Cache level %u Ready At %lu\n", this->level, this->sets[idx].lines[i].ready_at)
					}
				}
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
    //end search, se nao encontrou nada, retorna latencia do miss
    //printf("    Did not find address in cache %u = MISS\n", this->level);
	ttc += this->latency;
	return MISS;
}

// Returns the minor LRU line
inline uint32_t cache_t::searchLru(cacheSet_t *set) {
    // printf("%s\n", "-> searchLru in cache.cpp");
	uint32_t index = 0;
	for (uint32_t i = 1; i < set->n_lines; i++)	{
		index = (set->lines[index].lru <= set->lines[i].lru)? index : i;
	}
	return index;
}

// Copy data information to lower cache levels when data addresses are valid
void cache_t::copyLevels(line_t *line, uint32_t idxa, uint32_t idxb) {
    // printf("%s\n", "    copy cache information to lower cache levels when data address is valid");
	if (line->line_ptr_caches[0][idxa]->dirty == 1) {
		line->line_ptr_caches[0][idxa]->line_ptr_caches[0][idxb]->dirty = line->line_ptr_caches[0][idxa]->dirty;
		line->line_ptr_caches[0][idxa]->line_ptr_caches[0][idxb]->lru = orcs_engine.get_global_cycle();
		line->line_ptr_caches[0][idxa]->line_ptr_caches[0][idxb]->ready_at = line->line_ptr_caches[0][idxa]->ready_at;
	}
	line->line_ptr_caches[0][idxa]->clean_line();
}

// Copy data information to lower cache levels when data addresses are invalid
void cache_t::copyNextLevels(line_t *line, uint32_t idx) {
    // printf("%u\n", line->dirty);
    // printf("%s %u\n", "    copy cache information to lower cache levels when data address is invalid", idx);
	line->line_ptr_caches[0][idx]->dirty = line->dirty;
	line->line_ptr_caches[0][idx]->lru = orcs_engine.get_global_cycle();
	line->line_ptr_caches[0][idx]->ready_at = line->ready_at;
}

// Writebacks an address from a specific cache to its next lower level (removed EMC)
inline void cache_t::writeBack(line_t *line) {
    // printf("-> writeback in cache.cpp - cache level = %u\n", this->level);
    //printf("%s\n", "    searches for line references in another caches");
    for (uint32_t i = this->level + 1; i < DATA_LEVELS - 1; i++) {
        ERROR_ASSERT_PRINTF(line->line_ptr_caches[0][i] != NULL, "Error, no line reference in next levels.")
    }

	// L1 writeBack issues
	if (this->level == 0) {
        // printf("%s\n", "    in L1 level:");
		for (uint32_t i = 1; i < DATA_LEVELS; i++) {
			this->copyNextLevels(line, i);
            //printf("%s\n", "    and NULLs information from other caches to this level");
			line->line_ptr_caches[0][i]->line_ptr_caches[0][this->level] = NULL;//Pointer to Lower Level
		}
		line->clean_line();

	// LLC writeBack issues
    } else if (this->level == DATA_LEVELS - 1) {
        //printf("%s\n", "    in LLC level:");
		for (uint32_t i = 0; i < DATA_LEVELS - 1; i++) {
            //printf("%s\n", "    cleans every line from caches");
			if (line->line_ptr_caches[0][i] != NULL) {
				line->line_ptr_caches[0][i]->clean_line();
			}
		}

	// Intermediate cache levels issues
	} else {
        // printf("%s\n", "    in intermediate levels:");
		uint32_t i;
		for (i = 0; i < this->level - 1; i++) {
            // printf("%s\n", "for");
			if (line->line_ptr_caches[0][i] != NULL) {
                // printf("%s\n", "if");
				copyLevels(line, i, i + 1);
			}
		}
        // printf("%u\n", i);
        if (line->line_ptr_caches[0][i] != NULL) {
            // printf("%s\n", "if");
			copyLevels(line, i, i + 2);
		} else {
            // printf("%s\n", "else");
			// copyNextLevels(line->line_ptr_caches[0][i], i + 2);
            line->line_ptr_caches[0][i + 2]->dirty = line->dirty;
            line->line_ptr_caches[0][i + 2]->lru = orcs_engine.get_global_cycle();
            line->line_ptr_caches[0][i + 2]->ready_at = line->ready_at;
		}
        //printf("%s\n", "    NULLs lines from higher level caches");
		for (uint32_t i = this->level + 1; i < DATA_LEVELS; i++) {
			for (uint32_t j = 0; j <= this->level; j++) {
				line->line_ptr_caches[0][i]->line_ptr_caches[0][j] = NULL;
			}
		}
		line->clean_line();
	}
}

// Searches for a cache line to write data (removed EMC)
line_t* cache_t::installLine(uint64_t address, uint32_t latency) {
    // printf("-> installLine in cache.cpp - cache level = %u\n", this->level);
	int32_t line = POSITION_FAIL;
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
    //printf("%s\n", "    searches for a valid line in cache");
	for (size_t i = 0; i < this->sets->n_lines; i++) {
		if (this->sets[idx].lines[i].valid == 0) {
            //printf("    finds line %lu in set %u\n", i, idx);
			line = i;
            break;
		}
	}
    //printf("        line in installLine: %u\n", line);
	if (line == POSITION_FAIL) {
        //printf("%s\n", "There is no valid line in set");
		line = this->searchLru(&this->sets[idx]);
        //printf("    line %u in set %u was found.\n", line, idx);
        // printf("%u\n", this->sets[idx].lines[line].line_ptr_caches[0][this->level+1]->dirty);
		this->add_change_line();
		if (this->sets[idx].lines[line].dirty == 1) {
            //printf("    line %u is dirty\n", line);
			this->writeBack(&this->sets[idx].lines[line]);
			this->add_cache_writeback();
		}
	}
    //printf("%s\n", "    Install cache line");
	this->sets[idx].lines[line].tag = tag;
	this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle() + latency;
	this->sets[idx].lines[line].valid = 1;
	this->sets[idx].lines[line].dirty = 0;
	this->sets[idx].lines[line].prefetched = 0;
	this->sets[idx].lines[line].ready_at = orcs_engine.get_global_cycle() + latency;
    // printf("    Installed line: %p\n", &this->sets[idx].lines[line]);
	return &this->sets[idx].lines[line];
}

// Selects a cache line to install an address and points this memory address with the other cache pointers
void cache_t::returnLine(uint64_t address, cache_t *cache) {
    // printf("-> returnLine in cache.cpp - cache level = %u\n", this->level);
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
	int32_t line = POSITION_FAIL;

	// Selects a line in this cache
    //printf("    Selects a line in cache %u...\n", this->level);
	for (size_t i = 0; i < cache->sets->n_lines; i++) {
		if (this->sets[idx].lines[i].tag == tag) {
			this->sets[idx].lines[i].lru = orcs_engine.get_global_cycle();
			line = i;
			break;
		}
	}
    //printf("    line in returnLine = %d selected\n", line);
    ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line não encontrada para retorno")

    //printf("%s\n", "chegou no problema");
    // printf("this level: %d  cache level: %d \n", this->level, cache->level);
	if (this->level > 0) {
        //printf("cache level da cache do parametro: %u\n", cache->level);
		line_t *line_return = NULL;
		line_return = cache->installLine(address, this->latency);
        // printf("    line in returnLine = %d selected\n", line_return->dirty);
		this->sets[idx].lines[line].line_ptr_caches[0][cache->level] = line_return;

        // printf("    ptr_cache[%u] = line_return: %p - %p\n", cache->level, line_return, this->sets[idx].lines[line].line_ptr_caches[0][cache->level]);
		for (uint32_t i = this->level + 1; i < DATA_LEVELS; i++) {
			line_return->line_ptr_caches[0][i] = this->sets[idx].lines[line].line_ptr_caches[0][i];
            // printf("line_return->ptr_cache[%u] = ptr_cache[%u] = %p - %p\n", i, i, line_return->line_ptr_caches[0][i], this->sets[idx].lines[line].line_ptr_caches[0][i]);
		}
		line_return->line_ptr_caches[0][this->level] = &this->sets[idx].lines[line];
        // printf("    line_return->ptr_cache[%u] = this->sets[%u].lines[%d] = %p - %p\n", this->level, idx, line, line_return->line_ptr_caches[0][this->level], &this->sets[idx].lines[line]);

		// Copy information
		line_return->dirty = line_return->line_ptr_caches[0][this->level]->dirty;
		line_return->lru = line_return->line_ptr_caches[0][this->level]->lru;
		line_return->prefetched = line_return->line_ptr_caches[0][this->level]->prefetched;
		line_return->ready_at = orcs_engine.get_global_cycle();
	}
}


// ============================
// @address write address
// ============================
uint32_t cache_t::write(uint64_t address){
    // printf("%s\n", "-> write in cache.cpp");
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
	int32_t line = POSITION_FAIL;
	this->add_cache_write();
	// this->add_cacheAccess();
    //printf("    Selects a line in cache %u...\n", this->level);
    for (size_t i = 0; i < this->sets->n_lines; i++){
		if(this->sets[idx].lines[i].tag == tag){
			line = i;
			break;
		}
	}
    if (line == POSITION_FAIL) {
        //printf("    No correspondent line for address %lu\n", address);
        line = this->searchLru(&this->sets[idx]);
        this->add_change_line();
        if (this->sets[idx].lines[line].dirty == 1) {
            // printf("    line = %d is dirty!\n", line);
            // printf("%u\n", this->sets[idx].lines[line].line_ptr_caches[0][this->level+1]->dirty);
            this->writeBack(&this->sets[idx].lines[line]);
            this->add_cache_writeback();
        }
    }
    //printf("    line %d in indice %u selected\n", line, idx);
	ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, line nao encontrada para escrita")
    this->sets[idx].lines[line].dirty = 1;
	if(this->sets[idx].lines[line].ready_at <= orcs_engine.get_global_cycle()){
		this->sets[idx].lines[line].lru = orcs_engine.get_global_cycle();
	} else {
		this->sets[idx].lines[line].lru = this->sets[idx].lines[line].ready_at + this->latency;
	}
	return OK;
}


// ====================
// statistics of a level of cache
// ====================
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
		fprintf(output, "%d_Cache_Read: %lu\n", this->level, this->get_cache_read());
		fprintf(output, "%d_Cache_Write: %lu\n", this->level, this->get_cache_write());
		if(this->get_cache_writeback()!=0){
			fprintf(output, "%d_Cache_WriteBack: %lu\n", this->level, this->get_cache_writeback());
		}
		utils_t::largeSeparator(output);
	}
	if(close) fclose(output);
}
