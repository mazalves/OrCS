#include "../simulator.hpp"

#ifdef CACHE_DEBUG
	#define CACHE_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
	#define CACHE_DEBUG_PRINTF(...)
#endif

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
    this->change_way = 0;
}

cache_t::~cache_t(){
	if (orcs_engine.get_global_cycle() == 0) return;
	for (size_t i = 0; i < this->n_sets; i++) {
		// for (uint32_t j = 0; j < this->sets[i].n_ways; j++) {
        //     //delete this->sets[i].ways[j].directory_way;
		// }
		delete[] this->sets[i].ways;
    }
	delete[] sets;
	//ORCS_PRINTF ("cycle: %lu\n", orcs_engine.get_global_cycle())
}

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
		this->sets[i].ways = new way_t[this->associativity];
		this->sets[i].n_ways = this->associativity;
        for (uint32_t j = 0; j < this->sets[i].n_ways; j++) {
			this->sets[i].ways[j].directory_way = new directory_way_t;
	        this->sets[i].ways[j].clean_way();
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

inline uint32_t cache_t::is_LLC() {
	if (this->level == POINTER_LEVELS - 1) {
		return 1;
	}
	return 0;
}

inline void cache_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag) {
	uint32_t get_bits = this->n_sets - 1;
	*tag = address >> this->offset;
	*idx = *tag & get_bits;
}

inline int32_t cache_t::getInvalidLine(uint32_t idx) {
	int32_t way = POSITION_FAIL;
	for (size_t i = 0; i < this->sets[idx].n_ways; i++) {
		if (this->sets[idx].ways[i].valid == 0) {
			way = i;
			break;
		}
	}
	return way;
}

inline int32_t cache_t::getCacheLine(uint32_t idx, uint64_t tag) {
	int32_t way = POSITION_FAIL;
	for (size_t i = 0; i < this->sets[idx].n_ways; i++) {
		if (this->sets[idx].ways[i].tag == tag) {
			way = i;
			break;
		}
	}
	return way;
}

uint32_t cache_t::read(uint64_t address,uint32_t &ttc){
    uint32_t idx;
    uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	int32_t way = this->getCacheLine(idx, tag);
	CACHE_DEBUG_PRINTF("Reading address %lu (TAG: %lu - index %u) in %s %s: ", address, tag, idx, get_enum_cache_id_char(this->id), get_cache_level_char(this->level));

	if (way != POSITION_FAIL) {
		if (this->sets[idx].ways[way].ready_at <= orcs_engine.get_global_cycle()) {
			if (PREFETCHER_ACTIVE) {
				if (this->sets[idx].ways[way].prefetched == 1) {
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					this->sets[idx].ways[way].prefetched = 0;
				}
			}
			this->sets[idx].ways[way].lru = orcs_engine.get_global_cycle();
			ttc += this->latency;
		} else {
			if (PREFETCHER_ACTIVE) {
				if (this->sets[idx].ways[way].prefetched == 1) {
					orcs_engine.cacheManager->prefetcher->add_latePrefetches();
					orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
					uint32_t latePrefetcher = orcs_engine.cacheManager->prefetcher->get_totalCycleLate() + this->sets[idx].ways[way].ready_at - orcs_engine.get_global_cycle();
					orcs_engine.cacheManager->prefetcher->set_totalCycleLate(latePrefetcher);
					this->sets[idx].ways[way].prefetched = 0;
				}
			}
			ttc+=(this->sets[idx].ways[way].ready_at - orcs_engine.get_global_cycle());
			this->sets[idx].ways[way].lru = ttc;
		}
		CACHE_DEBUG_PRINTF("HIT!\n");
		return HIT;
	}
	ttc += this->latency;
	CACHE_DEBUG_PRINTF("MISS!\n");
	return MISS;
}

inline int32_t cache_t::searchLru(uint32_t idx) {
	int32_t index = 0;
	for (uint32_t i = 1; i < this->sets[idx].n_ways; i++) {
		index = (this->sets[idx].ways[index].lru <= this->sets[idx].ways[i].lru) ? index : i;
	}
	return index;
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

uint32_t cache_t::checkUpperLevels(uint64_t address, directory_t directory) {
	uint32_t wb_status = 0;
	for (uint32_t i = 0; i < this->level; i++) {
		if (directory.validCacheLine(address, i)) {
			if (directory.dirtyCacheLine(address, i)) {
				if (wb_status == 0) {
					directory.copyCacheInfo(address, this->level + 1, i);
					wb_status = 1;
				}
			}
			directory.nullCachePointer(address, i);
		}
	}
	return wb_status;
}

inline void cache_t::writeback(way_t cache_way, directory_t directory, memory_operation_t mem_op) {
	CACHE_DEBUG_PRINTF("Writeback address %lu (TAG %lu) in %s %s\n", cache_way.address, cache_way.tag, get_enum_cache_id_char(this->id), get_cache_level_char(this->level));

	if (this->is_LLC()) {
		CACHE_DEBUG_PRINTF("Dirty in LLC: ...")
		this->sendMemoryRequest(cache_way.address, mem_op);
		directory.nullingCaches(cache_way.address, POINTER_LEVELS);
	} else {
		CACHE_DEBUG_PRINTF("Dirty in level %s ...", get_cache_level_char(this->level));
		uint32_t wb_status = this->checkUpperLevels(cache_way.address, directory);
		if (wb_status == 0) {
			for (uint32_t i = this->level + 1; i < POINTER_LEVELS; i++) {
				directory.copyCacheInfo(cache_way.address, i, this->level);
			}
		}
	}
	this->add_cache_writeback();
}

inline void cache_t::eviction(directory_t directory, uint32_t idx, int32_t way, memory_operation_t mem_op) {
	CACHE_DEBUG_PRINTF("Evicting address %lu (TAG %lu - index %u - way %d) in %s %s\n", this->sets[idx].ways[way].address, this->sets[idx].ways[way].tag, idx, way, get_enum_cache_id_char(this->id), get_cache_level_char(this->level));

	if (this->sets[idx].ways[way].directory_way == NULL) {
		CACHE_DEBUG_PRINTF("Cache pointer to directory is NULL. Cache TAG: %lu\n", this->sets[idx].ways[way].tag);
	}

	if (this->sets[idx].ways[way].dirty == 1) {
		this->writeback(this->sets[idx].ways[way], directory, mem_op);
	} else {
		if (this->is_LLC()) {
			CACHE_DEBUG_PRINTF("Not dirty in LLC ...\n");
			directory.nullingCaches(this->sets[idx].ways[way].address, POINTER_LEVELS);
		} else {
			CACHE_DEBUG_PRINTF("Not dirty in %s ...\n", get_cache_level_char(this->level));
			[[maybe_unused]] static uint32_t wb_status = this->checkUpperLevels(this->sets[idx].ways[way].address, directory);
			directory.nullCachePointer(this->sets[idx].ways[way].address, this->level);
		}
	}
}

way_t *cache_t::installLine(uint64_t address, uint32_t latency, directory_t directory, uint32_t &idx, int32_t &way, memory_operation_t mem_op) {
	uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	way = this->getInvalidLine(idx);

	CACHE_DEBUG_PRINTF("Installing address %lu (TAG %lu - index %u) in %s %s\n", address, tag, idx, get_enum_cache_id_char(this->id), get_cache_level_char(this->level));

	if (way == POSITION_FAIL) {
		way = this->searchLru(idx);
		this->add_change_way();
		this->eviction(directory, idx, way, mem_op);
	}

	this->sets[idx].ways[way].address = address;
	this->sets[idx].ways[way].tag = tag;
	this->sets[idx].ways[way].lru = orcs_engine.get_global_cycle() + latency;
	this->sets[idx].ways[way].valid = 1;
	this->sets[idx].ways[way].dirty = 0;
	this->sets[idx].ways[way].prefetched = 0;
	this->sets[idx].ways[way].ready_at = orcs_engine.get_global_cycle() + latency;
	this->add_cache_eviction();

	CACHE_DEBUG_PRINTF("Address %lu (TAG %lu - index %u) installed in way %d in %s cache\n", address, tag, idx, way, get_enum_cache_id_char(this->id));

	if (this->level == 0 && mem_op == MEMORY_OPERATION_WRITE) {
		this->write(idx, way);
	}

	return &this->sets[idx].ways[way];
}

void cache_t::returnLine(uint64_t address, cache_t *cache, directory_t directory, memory_operation_t mem_op) {
	uint32_t idx, idx_padding;
	int32_t way, way_padding;
	uint64_t tag;
	this->tagIdxSetCalculation(address, &idx, &tag);
	way = this->getCacheLine(idx, tag);
	this->sets[idx].ways[way].lru = orcs_engine.get_global_cycle();

	CACHE_DEBUG_PRINTF("Return address %lu (TAG %lu - index %u - way %d) from %s cache %s to %s cache %s\n", address, tag, idx, way, get_enum_cache_id_char(this->id), get_cache_level_char(this->level), get_enum_cache_id_char(cache->id), get_cache_level_char(cache->level));
	ERROR_ASSERT_PRINTF(way != POSITION_FAIL, "Error, way not found!")

	way_t *return_way = NULL;
	return_way = cache->installLine(address, this->latency, directory, idx_padding, way_padding, mem_op);

	if (this->sets[idx].ways[way].directory_way == NULL) {
		CACHE_DEBUG_PRINTF("POINTER TO DIRECTORY IS NULL!\n");
	}

	return_way->dirty = this->sets[idx].ways[way].dirty;
	return_way->lru = this->sets[idx].ways[way].lru;
	return_way->prefetched = this->sets[idx].ways[way].prefetched;
	return_way->ready_at = orcs_engine.get_global_cycle();

	directory.setCachePointers(return_way, cache->level, mem_op);

	if (mem_op == MEMORY_OPERATION_WRITE) {
		this->add_cache_write();
	}
}

void cache_t::write(uint32_t idx, int32_t way) {
	CACHE_DEBUG_PRINTF("Writing address %lu in %s cache\n", this->sets[idx].ways[way].address, get_enum_cache_id_char(this->id));
	this->add_cache_write();
    this->sets[idx].ways[way].dirty = 1;
	if(this->sets[idx].ways[way].ready_at <= orcs_engine.get_global_cycle()){
		this->sets[idx].ways[way].lru = orcs_engine.get_global_cycle();
	} else {
		this->sets[idx].ways[way].lru = this->sets[idx].ways[way].ready_at + this->latency;
	}
}

void cache_t::statistics() {
	FILE *output = stdout;
	bool close = false;
	if(orcs_engine.output_file_name != NULL){
		close=true;
		output = fopen(orcs_engine.output_file_name,"a+");
	}
	if (output != NULL){
		utils_t::largeSeparator(output);
		fprintf(output, "Cache_Level: %s - Cache_Type: %s\n", get_cache_level_char(this->level), get_enum_cache_id_char(this->id));
		fprintf(output, "%s_Cache_Access: %lu\n", get_cache_level_char(this->level), this->get_cache_access());
		fprintf(output, "%s_Cache_Hits: %lu\n", get_cache_level_char(this->level), this->get_cache_hit());
		fprintf(output, "%s_Cache_Miss: %lu\n", get_cache_level_char(this->level), this->get_cache_miss());
		fprintf(output, "%s_Cache_Eviction: %lu\n", get_cache_level_char(this->level), this->get_cache_eviction());
		fprintf(output, "%s_Cache_Read: %lu\n", get_cache_level_char(this->level), this->get_cache_read());
		fprintf(output, "%s_Cache_Write: %lu\n", get_cache_level_char(this->level), this->get_cache_write());
		if(this->get_cache_writeback()!=0){
			fprintf(output, "%s_Cache_WriteBack: %lu\n", get_cache_level_char(this->level), this->get_cache_writeback());
		}
		utils_t::largeSeparator(output);
	}
	if(close) fclose(output);
}
