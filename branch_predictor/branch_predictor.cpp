#include "../simulator.hpp"
#include<string.h>
branch_predictor_t::branch_predictor_t(){
    this->btb = NULL;
	this->branchPredictor = NULL;
}
branch_predictor_t::~branch_predictor_t(){
    if(this->branchPredictor != NULL){
		delete this->branchPredictor;
	}

	delete[] this->btb;
	//Setting pointers to null
	this->btb = NULL;
	this->branchPredictor = NULL;
}
void branch_predictor_t::allocate(){
	BTB_ENTRIES = orcs_engine.configuration->getSetting("BTB_ENTRIES");
    BTB_WAYS = orcs_engine.configuration->getSetting("BTB_WAYS");
    BTB_MISS_PENALITY = orcs_engine.configuration->getSetting("BTB_MISS_PENALITY");
    MISSPREDICTION_PENALITY = orcs_engine.configuration->getSetting("MISSPREDICTION_PENALITY");

    uint32_t size  = BTB_ENTRIES/BTB_WAYS;
    this->btb = new btb_t[size];
	this->index = 0;
	this->way = 0;
    for (size_t i = 0; i < size; i++)
    {
        this->btb[i].btb_entry = new btb_line_t[BTB_WAYS];
    	std::memset(&this->btb[i].btb_entry[0],0,(BTB_WAYS*sizeof(btb_line_t)));
    }
    //allocate branch predictor
	this->branchPredictor = new piecewise_t();
    this->branchPredictor->allocate();
}
uint32_t branch_predictor_t::searchLine(uint64_t pc){
	uint32_t getBits = (BTB_ENTRIES/BTB_WAYS);
	uint32_t tag = (pc >> 2);
	uint32_t index = tag&(getBits-1);
	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (size_t i = 0; i < BTB_WAYS; i++)
	{
		//std::cout<< "%u\n",this->btb[index].btb_entry[i].tag);
		if(this->btb[index].btb_entry[i].tag == pc){
			//std::cout<< "BTB_Hit");
			this->btb[index].btb_entry[i].lru=orcs_engine.get_global_cycle();
			//save locate from line
			this->index = index;
			this->way = i;
			return HIT;
		}
	}
	//std::cout<< "BTB_Miss");
	return MISS;
}
uint32_t branch_predictor_t::installLine(opcode_package_t instruction){
	uint32_t getBits = (BTB_ENTRIES/BTB_WAYS);
	uint32_t tag = (instruction.opcode_address >> 2);
	uint32_t index = tag&(getBits-1);
	// std::cout<< "bits %u, tag %u index %u\n",getBits,tag,index);
	for (size_t i = 0; i < BTB_WAYS; i++)
	{
		// instala no primeiro invalido 
		if(this->btb[index].btb_entry[i].validade == 0){
			this->btb[index].btb_entry[i].tag=instruction.opcode_address;
			this->btb[index].btb_entry[i].lru=orcs_engine.get_global_cycle();
			this->btb[index].btb_entry[i].targetAddress=instruction.opcode_address+instruction.opcode_size;
			this->btb[index].btb_entry[i].validade=1;
			this->btb[index].btb_entry[i].typeBranch=instruction.branch_type;
			this->btb[index].btb_entry[i].bht=0;
			this->index = index;
			this->way = i;
			return OK;
		}			
	}
	uint32_t way = this->searchLRU(&this->btb[index]);
	this->btb[index].btb_entry[way].tag=instruction.opcode_address;
	this->btb[index].btb_entry[way].lru=orcs_engine.get_global_cycle();
	this->btb[index].btb_entry[way].targetAddress=instruction.opcode_address+instruction.opcode_size;
	this->btb[index].btb_entry[way].validade=1;
	this->btb[index].btb_entry[way].typeBranch=instruction.branch_type;
	this->btb[index].btb_entry[way].bht=0;
	//indexes
	this->index = index;
	this->way = way;
	return OK;
}
inline uint32_t branch_predictor_t::searchLRU(btb_t *btb){
	uint32_t index=0;
	for (size_t i = 1; i < BTB_WAYS; i++)
	{
		index = (btb->btb_entry[index].lru <= btb->btb_entry[i].lru)? index : i ;
	}
	return index;
}
void branch_predictor_t::statistics(){
	bool close = false;
	FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
		close=true;	
	}
	if (output != NULL){
			utils_t::largestSeparator(output);
			fprintf(output,"BTB Hits: %u\n",this->btbHits);
			fprintf(output,"BTB Miss: %u\n",this->btbMiss);
			fprintf(output,"Total Branchs: %u\n",this->branches);
			fprintf(output,"Total Branchs Taken: %u\n",this->branchTaken);
			fprintf(output,"Total Branchs Not Taken: %u\n",this->branchNotTaken);
			fprintf(output,"Correct Branchs Taken: %u\n",(this->branchTaken-this->branchTakenMiss));
			fprintf(output,"Incorrect Branchs Taken: %u\n",this->branchTakenMiss);
			fprintf(output,"Correct Branchs Not Taken: %u\n",(this->branchNotTaken-this->branchNotTakenMiss));
			fprintf(output,"Incorrect Branchs Not Taken: %u\n",this->branchNotTakenMiss);
			utils_t::largestSeparator(output);
		}
	if(close) fclose(output);
}
uint32_t branch_predictor_t::solveBranch(opcode_package_t branchInstrucion, opcode_package_t nextInstruction){
    //==========
    // Consulta BTB
    //==========
    uint64_t stallCyles=0;
    uint32_t btbStatus = this->searchLine(branchInstrucion.opcode_address);
    if(btbStatus == HIT){
        this->btbHits++;
    }else{
        this->btbMiss++;
        this->installLine(branchInstrucion);
        stallCyles+=BTB_MISS_PENALITY;
    }
    //==========
    // Predict Branch
    //==========
    taken_t branchStatus = this->branchPredictor->predict(branchInstrucion.opcode_address);
	// printf("this->index %u, this->way %hhu\n",this->index,this->way);
	// sleep(1);
    if((nextInstruction.opcode_address != this->btb[this->index].btb_entry[this->way].targetAddress)&&
        (this->btb[this->index].btb_entry[this->way].typeBranch == BRANCH_COND)){
            this->branchTaken++;
			if(branchStatus == TAKEN){
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,TAKEN);
			}else{
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,TAKEN);
				this->branchTakenMiss++;
				stallCyles+=MISSPREDICTION_PENALITY;
			}
    }else{
		this->branchNotTaken++;
			if(branchStatus == NOT_TAKEN){
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,NOT_TAKEN);
			}else{
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,NOT_TAKEN);
				this->branchNotTakenMiss++;
				stallCyles+=MISSPREDICTION_PENALITY;
			}
	}
    return stallCyles;
}