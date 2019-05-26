#include "../simulator.hpp"


cache_t::cache_t()
{
		this->id = 0;
        this->nSets = 0;
        this->nLines = 0;
        this->sets = NULL;
        this->shiftData = 0;
		this->cacheHit = 0;
        this->cacheMiss = 0;
        this->cacheAccess = 0;
        this->cacheRead = 0;
        this->cacheWrite = 0;
        this->cacheWriteBack = 0;
        this->changeLine = 0;
}

cache_t::~cache_t()
{
	if(this->sets!=NULL) delete[] &sets;
}    
// ==================
// @*linha -linha to be printed
// ==================
inline void cache_t::printLine(linha_t *linha){
	ORCS_PRINTF("[TAG: %lu| DIRTY: %u| lru : %lu| PREFETCHED: %u| VALID: %u| READY AT %lu]\n",linha->tag,linha->dirty,linha->lru,linha->prefetched, linha->valid,linha->readyAt)
}
// ==================
// print cache configuration
// ==================
inline void cache_t::printCacheConfiguration(){
	ORCS_PRINTF("[Cache Level: %s|Cache ID: %u| Cache Sets: %u| Cache Lines: %u] \n",get_enum_cache_level_char(this->level),this->id,this->nSets,this->nLines)
}
void cache_t::allocate(cacheLevel_t level){	
	switch(level){
		case INST_CACHE:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->nSets = L1_INST_SETS;
			this->nLines = L1_INST_ASSOCIATIVITY;
			this->sets = new cacheSet_t[L1_INST_SETS];
			for (size_t i = 0; i < L1_INST_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[L1_INST_ASSOCIATIVITY];
				for ( uint j = 0; j < this->nLines; j++)
				{
					this->sets[i].linhas[j].clean_line();
				}
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
			break;
			}
		case L1:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->nSets = L1_DATA_SETS;
			this->nLines = L1_DATA_ASSOCIATIVITY;
			this->sets = new cacheSet_t[L1_DATA_SETS];
			for (size_t i = 0; i < L1_DATA_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[L1_DATA_ASSOCIATIVITY];
				for ( uint j = 0; j < this->nLines; j++)
				{
					this->sets[i].linhas[j].clean_line();
				}				
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
			break;
			}
		case L2:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->nSets = L2_SETS;
			this->nLines = L2_ASSOCIATIVITY;
			this->sets = new cacheSet_t[L2_SETS];
			for (size_t i = 0; i < L2_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[L2_ASSOCIATIVITY];
				for ( uint j = 0; j < this->nLines; j++)
				{
					this->sets[i].linhas[j].clean_line();
				}
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
			break;
		}
		case LLC:{
			this->shiftData = utils_t::get_power_of_two(LINE_SIZE);
			this->level = level;
			this->nSets = LLC_SETS;
			this->nLines = LLC_ASSOCIATIVITY;
			this->sets = new cacheSet_t[LLC_SETS];
			for (size_t i = 0; i < LLC_SETS; i++)
			{
				this->sets[i].linhas = new linha_t[LLC_ASSOCIATIVITY];
				for ( uint j = 0; j < this->nLines; j++)
				{
					this->sets[i].linhas[j].clean_line();
				}
			}
			this->set_cacheAccess(0);
			this->set_cacheHit(0);
			this->set_cacheMiss(0);
			this->set_cacheRead(0);
			this->set_cacheWrite(0);
			this->set_cacheWriteBack(0);
			break;
		}
	}
}
// ==================
// @address -address to get index
// @return tag to data in cache
// ==================
inline uint64_t cache_t::tagSetCalculation(uint64_t address){
	uint64_t tag = (address >> this->shiftData);
	return tag;

}
// ==================
// @address -address to get index
// @return index of data in cache
// ==================
inline uint32_t cache_t::idxSetCalculation(uint64_t address){
	uint32_t getBits = (this->nSets)-1;
	uint64_t tag = this->tagSetCalculation(address);
	uint32_t index = tag&getBits;
	return index;

}
// ==================
// @address -address to make a read
// @ttc latency to complete
// @return HIT or MISS
// ==================
uint32_t cache_t::read(uint64_t address,uint64_t &ttc){
	uint32_t idx = this->idxSetCalculation(address);
	uint64_t tag = this->tagSetCalculation(address);
	// this->add_cacheRead();
	for (size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx].linhas[i].tag == tag){
			if (CACHE_MANAGER_DEBUG){
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("Cache Line %s\n",this->sets[idx].linhas[i].content_to_string().c_str())
				}
			}
			// =====================================================
			// Se ready Cycle for menor que o atual, a latencia é
			// apenas da leitura, sendo um hit.
			// =====================================================
			if(this->sets[idx].linhas[i].readyAt<=orcs_engine.get_global_cycle()){
				if (PREFETCHER_ACTIVE){
					if (this->sets[idx].linhas[i].prefetched == 1){
						orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
						this->sets[idx].linhas[i].prefetched =0;
					}
				}
				//add cache hit
				if(this->level == INST_CACHE){
					ttc+=L1_INST_LATENCY;
				}else if(this->level == L1){
					ttc+=L1_DATA_LATENCY;
					if (CACHE_MANAGER_DEBUG){
						if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
							ORCS_PRINTF("L1 Ready At %lu\n",this->sets[idx].linhas[i].readyAt)
						}
					}
				}else if(this->level == L2){
					ttc+=L2_LATENCY;
					if (CACHE_MANAGER_DEBUG){
						if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
							ORCS_PRINTF("L2 Ready At %lu\n",this->sets[idx].linhas[i].readyAt)
						}
					}
				}
				else if(this->level == LLC){
					ttc+=LLC_LATENCY;
					if (CACHE_MANAGER_DEBUG){
						if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
							ORCS_PRINTF("LLC Ready At %lu\n",this->sets[idx].linhas[i].readyAt)
						}
					}
				}
				return HIT;
			}
			// =====================================================
			// Se ready Cycle for maior que o atual, a latencia é
			// dada pela demora a chegar
			// =====================================================
			else{
				if (PREFETCHER_ACTIVE){
					if (this->sets[idx].linhas[i].prefetched == 1){
						orcs_engine.cacheManager->prefetcher->add_latePrefetches();
						orcs_engine.cacheManager->prefetcher->add_usefulPrefetches();
						uint32_t latePrefetcher = orcs_engine.cacheManager->prefetcher->get_totalCycleLate()+
						(this->sets[idx].linhas[i].readyAt - orcs_engine.get_global_cycle());
						orcs_engine.cacheManager->prefetcher->set_totalCycleLate(latePrefetcher);
						this->sets[idx].linhas[i].prefetched =0;
					}
				}
				ttc+=(this->sets[idx].linhas[i].readyAt - orcs_engine.get_global_cycle());
				this->sets[idx].linhas[i].lru = ttc;
				return HIT;
			}				
		}
	}//end search, se nao encontrou nada, retorna latencia do miss
		if(this->level == INST_CACHE){
				ttc+=L1_INST_LATENCY;
			}else if(this->level == L1){
				ttc+=L1_DATA_LATENCY;
			}else if(this->level == L2){
				ttc+=L2_LATENCY;
			}else{
				ttc+=LLC_LATENCY;
			}
	return MISS;
}
// ============================
// @address write address
// ============================
uint32_t cache_t::write(uint64_t address){
	uint64_t tag = this->tagSetCalculation(address);
	uint32_t idx = this->idxSetCalculation(address);
	int32_t line = POSITION_FAIL;
	this->add_cacheWrite();
	// this->add_cacheAccess();
			for (size_t i = 0; i < this->nLines; i++){
				if(this->sets[idx].linhas[i].tag == tag){
					line = i;
					break;
				}
			}
		//acertar lru.
		ERROR_ASSERT_PRINTF(line != POSITION_FAIL, "Error, Linha nao encontrada para escrita")
		if(this->sets[idx].linhas[line].readyAt<=orcs_engine.get_global_cycle()){
			this->sets[idx].linhas[line].dirty=1;
			this->sets[idx].linhas[line].lru = orcs_engine.get_global_cycle();
		}else{
			this->sets[idx].linhas[line].dirty=1;
			this->sets[idx].linhas[line].lru = this->sets[idx].linhas[line].readyAt+L1_DATA_LATENCY;
		}
	
	return OK;	
}
// ==================
// @address - address to install a line
// @return - pointer to line  
// ==================
linha_t* cache_t::installLine(uint64_t address,uint64_t latency){
	uint32_t idx = this->idxSetCalculation(address);
	uint64_t tag = this->tagSetCalculation(address);
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].valid==0){
			this->sets[idx].linhas[i].tag = tag;
			this->sets[idx].linhas[i].lru = orcs_engine.get_global_cycle()+latency;
			this->sets[idx].linhas[i].valid = 1;
			this->sets[idx].linhas[i].dirty = 0;
			this->sets[idx].linhas[i].prefetched = 0;
			this->sets[idx].linhas[i].readyAt = orcs_engine.get_global_cycle()+latency;
			return &this->sets[idx].linhas[i];
		}
	}
	uint32_t line = this->searchLru(&this->sets[idx]);
	this->add_changeLine();
	if(this->sets[idx].linhas[line].dirty==1){
		this->writeBack(&this->sets[idx].linhas[line]);
		this->add_cacheWriteBack();
		}
	this->sets[idx].linhas[line].tag = tag;
	this->sets[idx].linhas[line].lru = orcs_engine.get_global_cycle()+latency;
	this->sets[idx].linhas[line].valid = 1;	
	this->sets[idx].linhas[line].dirty = 0;	
	this->sets[idx].linhas[line].prefetched = 0;	
	this->sets[idx].linhas[line].readyAt = orcs_engine.get_global_cycle()+latency;
	return &this->sets[idx].linhas[line];
}
// ===================
// @set - cache set to locate lru
// @return index of line lru 
// ===================
inline uint32_t cache_t::searchLru(cacheSet_t *set){
	uint32_t index=0;
	uint32_t i=0;
	for (i = 1; i < this->nLines; i++)
	{
		index = (set->linhas[index].lru <= set->linhas[i].lru)? index : i ;
	}
	return index;
}

//====================
//write back
// @1 address - endereco do dado
// @2 linha a ser feito WB
//====================
inline void cache_t::writeBack(linha_t *linha){
	if(this->level == L1){
		ERROR_ASSERT_PRINTF(linha->linha_ptr_l2!=NULL,"Erro, Linha sem referencia a nivel L2 ")
		ERROR_ASSERT_PRINTF(linha->linha_ptr_llc!=NULL,"Erro, Linha sem referencia a LLC ")
		//Copy Status to L2
		linha->linha_ptr_l2->dirty = linha->dirty;//DIRTY
		linha->linha_ptr_l2->lru = orcs_engine.get_global_cycle();//LRU
		linha->linha_ptr_l2->readyAt = linha->readyAt;//READY_AT
		//Copy Status to LLC
		linha->linha_ptr_llc->dirty = linha->dirty;//DIRTY
		linha->linha_ptr_llc->lru = orcs_engine.get_global_cycle();//LRU
		linha->linha_ptr_llc->readyAt = linha->readyAt;//READY_AT

		// Nulling Pointers
		linha->linha_ptr_l2->linha_ptr_l1 = NULL;//Pointer to Lower Level
		linha->linha_ptr_llc->linha_ptr_l1 = NULL;//Pointer to Lower Level
		// invalidando a linha recem feita WB. 
		linha->clean_line();
	}else if(this->level==L2){
		ERROR_ASSERT_PRINTF(linha->linha_ptr_llc!=NULL,"Erro, Linha sem referencia a LLC ")
		if(linha->linha_ptr_l1 != NULL){
			if(linha->linha_ptr_l1->dirty==1){
				linha->linha_ptr_l1->linha_ptr_llc->dirty = linha->linha_ptr_l1->dirty;//DIRTY
				linha->linha_ptr_l1->linha_ptr_llc->lru = orcs_engine.get_global_cycle();//LRU
				linha->linha_ptr_l1->linha_ptr_llc->readyAt = linha->linha_ptr_l1->readyAt;//READY_AT
			}
			linha->linha_ptr_l1->clean_line();
		}else{
			//Copy Status from L2 to LLC
			linha->linha_ptr_llc->dirty = linha->dirty;//DIRTY
			linha->linha_ptr_llc->lru = orcs_engine.get_global_cycle();//LRU
			linha->linha_ptr_llc->readyAt = linha->readyAt;//READY_AT
		}
		// ========================================
		// Nulling pointers
		linha->linha_ptr_llc->linha_ptr_l1=NULL;
		linha->linha_ptr_llc->linha_ptr_l2=NULL;
		// Clean line
		linha->clean_line();
	}
	else if(this->level == LLC){
		// verify if has L1 reference
		if(linha->linha_ptr_l1 != NULL){
			// Invalidate L1 line
			linha->linha_ptr_l1->clean_line();
		}
		// verify if has L2 reference
		if(linha->linha_ptr_l2 != NULL){
			// Invalidate L2 line
			linha->linha_ptr_l2->clean_line();
		}
	}
}
//====================
// @1 address - endereco do dado
// @2 nivel de cache alvo da mudanca
// @3 *retorno 
//====================
void cache_t::returnLine(uint64_t address,cache_t *cache){
	uint32_t idx = this->idxSetCalculation(address);
	uint64_t tag = this->tagSetCalculation(address);
	int32_t line=POSITION_FAIL;
	// pega a linha desta cache
	for (size_t i = 0; i < this->nLines; i++)
	{
		if(this->sets[idx].linhas[i].tag==tag){
			this->sets[idx].linhas[i].lru=orcs_engine.get_global_cycle();
			line = i;
			break;
		}
	}
	ERROR_ASSERT_PRINTF(line!=POSITION_FAIL,"Error, linha não encontrada para retorno")
	linha_t *linha_return = NULL;
	if(this->level == LLC){
		ERROR_ASSERT_PRINTF(cache->level == L2,"Error, returning from LLC to Non L2 Level")
		linha_return = cache->installLine(address,LLC_LATENCY);
		this->sets[idx].linhas[line].linha_ptr_l2 = linha_return;
		linha_return->linha_ptr_llc = &this->sets[idx].linhas[line];

		// Copying infos
		linha_return->dirty = linha_return->linha_ptr_llc->dirty;
		linha_return->lru = linha_return->linha_ptr_llc->lru;
		linha_return->prefetched = linha_return->linha_ptr_llc->prefetched;
		linha_return->readyAt = orcs_engine.get_global_cycle();

	}
	if(this->level == L2){
		ERROR_ASSERT_PRINTF((cache->level == L1)||(cache->level == INST_CACHE),"Error, returning from L2 to Non L1 Level")
		linha_return = cache->installLine(address,L2_LATENCY);
		this->sets[idx].linhas[line].linha_ptr_l1 = linha_return;
		linha_return->linha_ptr_llc = this->sets[idx].linhas[line].linha_ptr_llc;
		linha_return->linha_ptr_l2 = &this->sets[idx].linhas[line];

		// Copying infos
		linha_return->dirty = linha_return->linha_ptr_l2->dirty;
		linha_return->lru = linha_return->linha_ptr_l2->lru;
		linha_return->prefetched = linha_return->linha_ptr_l2->prefetched;
		linha_return->readyAt = orcs_engine.get_global_cycle();
	}
}
// ====================
// statistics of a level of cache
// ====================
void cache_t::statistics(){
	FILE *output = stdout;
	bool close = false;
	if(orcs_engine.output_file_name != NULL){
		close=true;
		output = fopen(orcs_engine.output_file_name,"a+");
	}
	if (output != NULL){
		utils_t::largeSeparator(output);
		fprintf(output,"Cache_Level: %s\n",get_enum_cache_level_char(this->level));
		fprintf(output,"%s_Cache_Access: %lu\n",get_enum_cache_level_char(this->level),this->get_cacheAccess());
		fprintf(output,"%s_Cache_Hits: %lu\n",get_enum_cache_level_char(this->level),this->get_cacheHit());
		fprintf(output,"%s_Cache_Miss: %lu\n",get_enum_cache_level_char(this->level),this->get_cacheMiss());
		fprintf(output,"%s_Cache_Read: %lu\n",get_enum_cache_level_char(this->level),this->get_cacheRead());
		fprintf(output,"%s_Cache_Write: %lu\n",get_enum_cache_level_char(this->level),this->get_cacheWrite());
		if(this->get_cacheWriteBack()!=0){
			fprintf(output,"%s_Cache_WriteBack: %lu\n",get_enum_cache_level_char(this->level),this->get_cacheWriteBack());
		}
		utils_t::largeSeparator(output);
	}
	if(close) fclose(output);
}
// =====================================================================================================================================================================
// ==================
// @address -address to make a read
// @return HIT or MISS
// ==================
uint32_t cache_t::read_oracle(uint64_t address){
	uint32_t idx = this->idxSetCalculation(address);
	uint64_t tag = this->tagSetCalculation(address);
	// this->add_cacheRead();
	for (size_t i = 0; i < this->nLines; i++){
		if(this->sets[idx].linhas[i].tag == tag){		
				return HIT;
			}				
	}//end search, se nao encontrou nada, retorna latencia do miss
	return MISS;
}