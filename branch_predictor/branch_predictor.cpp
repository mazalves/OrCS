#include "../simulator.hpp"

branch_predictor_t::branch_predictor_t() {
    this->btb = NULL;
	this->branchPredictor = NULL;

	this->set_branches (0);
	this->set_branchNotTaken (0);
	this->set_branchNotTakenMiss (0);
	this->set_branchTaken (0);
	this->set_branchTakenMiss (0);
	
	this->set_btbHits (0);
	this->set_btbMiss (0);
	
	libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
	libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
	set_BTB_ENTRIES (cfg_processor["BTB_ENTRIES"]);
    set_BTB_WAYS (cfg_processor["BTB_WAYS"]);
    set_BTB_MISS_PENALITY (cfg_processor["BTB_MISS_PENALITY"]);
    set_MISSPREDICTION_PENALITY (cfg_processor["MISSPREDICTION_PENALITY"]);
	if (!strcmp (cfg_processor["BRANCH_PREDICTION_METHOD"], "PIECEWISE")) this->BRANCH_PREDICTION_METHOD = BRANCH_PREDICTION_METHOD_PIECEWISE;
	else if (!strcmp (cfg_processor["BRANCH_PREDICTION_METHOD"], "TWO_BIT")) this->BRANCH_PREDICTION_METHOD = BRANCH_PREDICTION_METHOD_TWO_BIT;
}

branch_predictor_t::~branch_predictor_t() {
	if (this->branchPredictor != NULL){
		delete this->branchPredictor;
	}
	delete[] this->btb;
	this->btb = NULL;
	this->branchPredictor = NULL;
}

void branch_predictor_t::allocate (uint32_t processor_id) {
	// reading processor information about branch predictor
	libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
	libconfig::Setting &cfg_branch_pred = cfg_root["PROCESSOR"][0];
	set_BTB_ENTRIES(cfg_branch_pred["BTB_ENTRIES"]);
	set_BTB_WAYS(cfg_branch_pred["BTB_WAYS"]);
	set_BTB_MISS_PENALITY(cfg_branch_pred["BTB_MISS_PENALITY"]);
	set_MISSPREDICTION_PENALITY(cfg_branch_pred["MISSPREDICTION_PENALITY"]);

	this->processor_id = processor_id;

	uint32_t size  = BTB_ENTRIES/BTB_WAYS;
    this->btb = new btb_t[size]();
	this->index = 0;
	this->way = 0;
    for (size_t i = 0; i < size; i++) {
        this->btb[i].btb_entry = new btb_line_t[BTB_WAYS]();
    	std::memset(&this->btb[i].btb_entry[0],0,(BTB_WAYS*sizeof(btb_line_t)));
    }
    switch (this->BRANCH_PREDICTION_METHOD) {
		case BRANCH_PREDICTION_METHOD_PIECEWISE: {
			this->branchPredictor = new piecewise_t();
    		this->branchPredictor->allocate (this->processor_id);
			break;
		}
		case BRANCH_PREDICTION_METHOD_TWO_BIT: {
			break;
		}
	}
}

uint32_t branch_predictor_t::searchLine(uint64_t pc) {
	uint32_t getBits = (BTB_ENTRIES/BTB_WAYS);
	uint32_t tag = (pc >> 2);
	uint32_t index = tag&(getBits-1);

	for (size_t i = 0; i < BTB_WAYS; i++) {
		if(this->btb[index].btb_entry[i].tag == pc) {
			this->btb[index].btb_entry[i].lru=orcs_engine.get_global_cycle();
			this->index = index;
			this->way = i;
			return HIT;
		}
	}
	return MISS;
}

uint32_t branch_predictor_t::installLine(opcode_package_t instruction) {
	uint32_t getBits = (BTB_ENTRIES/BTB_WAYS);
	uint32_t tag = (instruction.opcode_address >> 2);
	uint32_t index = tag&(getBits-1);

	for (size_t i = 0; i < BTB_WAYS; i++) {
		// Installs in the first invalid position
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

inline uint32_t branch_predictor_t::searchLRU(btb_t *btb) {
	uint32_t index=0;
	for (size_t i = 1; i < BTB_WAYS; i++) {
		index = (btb->btb_entry[index].lru <= btb->btb_entry[i].lru)? index : i;
	}
	return index;
}

void branch_predictor_t::statistics() {
	bool close = false;
	FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL) {
		output = fopen(orcs_engine.output_file_name,"a+");
		close=true;	
	}
	if (output != NULL) {
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

uint32_t branch_predictor_t::solveBranch(opcode_package_t branchInstrucion, opcode_package_t nextInstruction) {
	// BTB Query
    uint64_t stallCyles=0;
    uint32_t btbStatus = this->searchLine(branchInstrucion.opcode_address);
    if(btbStatus == HIT) {
        this->btbHits++;
    } else {
        this->btbMiss++;
        this->installLine(branchInstrucion);
        stallCyles+=BTB_MISS_PENALITY;
    }

    // Predict Branch
	taken_t branchStatus = this->branchPredictor->predict(branchInstrucion.opcode_address);
    if ((nextInstruction.opcode_address != this->btb[this->index].btb_entry[this->way].targetAddress)&&
        (this->btb[this->index].btb_entry[this->way].typeBranch == BRANCH_COND)){
            this->branchTaken++;
			if (branchStatus == TAKEN){
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,TAKEN);
			} else {
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,TAKEN);
				this->branchTakenMiss++;
				stallCyles+=MISSPREDICTION_PENALITY;
			}
    } else {
		this->branchNotTaken++;
			if( branchStatus == NOT_TAKEN) {
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,NOT_TAKEN);
			} else {
				this->branchPredictor->train(branchInstrucion.opcode_address,branchStatus,NOT_TAKEN);
				this->branchNotTakenMiss++;
				stallCyles+=MISSPREDICTION_PENALITY;
			}
	}
    return stallCyles;
}