#include "../simulator.hpp"

piecewise_t::piecewise_t() {
    this->W = NULL;
    this->GA = NULL;
    this->GHR = NULL;
}

piecewise_t::~piecewise_t() {
    if(this->W) {
        for (size_t i = 0; i < N; i++) {            
            for (size_t j = 0; j < M; j++) {   
                delete [] this->W[i][j];
                this->W[i][j] = NULL;
            }
            delete[] this->W[i];
            this->W[i] = NULL;
        }
    }
    delete[] this->W;
    this->W=NULL;
    if(this->GA) delete [] GA;
    if(this->GHR) delete [] GHR;
    this->GA = NULL;
    this->GHR = NULL;
}

void piecewise_t::allocate (uint32_t processor_id) {
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_branch_pred = cfg_root["PROCESSOR"][0];
    set_N(cfg_branch_pred["N"]);
    set_M(cfg_branch_pred["M"]);
    set_H(cfg_branch_pred["H"]);

    this->processor_id = processor_id;

    THETA = ((2.14*(H)) + 20.58);
    this->W = new int8_t**[N]();
    for (size_t i = 0; i < N; i++) {
        this->W[i] = new int8_t*[M]();
        for (size_t j = 0; j < M; j++) {
            this->W[i][j] = new int8_t[H+1]();
            std::memset(&this->W[i][j][0],0,((H+1)*sizeof(int8_t)));
        }
    }
    this->GA = new uint64_t[M]();
    this->GHR = new uint8_t[H]();
    std::memset(&this->GA[0],0,(M*sizeof(uint64_t)));
    std::memset(&this->GHR[0],0,(H*sizeof(uint8_t)));
    this->saida = 0;
}

taken_t piecewise_t::predict(uint64_t address) {
    uint64_t indexA = address%N;
    uint64_t indexB = address%M;
    this->saida = this->W[indexA][indexB][0];
    for (size_t i = 0; i < H; i++) {
     (this->GHR[i]==TAKEN)?this->saida += this->W[indexA][this->GA[i]][i]
     :this->saida -= this->W[indexA][this->GA[i]][i];   
    }
    return (this->saida >=0)? TAKEN: NOT_TAKEN;
}

void piecewise_t::train(uint64_t address,taken_t predict, taken_t correct) {
    uint64_t indexA = address%N;
    uint64_t indexB = address%M;

    if((abs(this->saida)<THETA)||(predict != correct)) {
        if(correct==TAKEN) {
            this->W[indexA][indexB][0] = this->W[indexA][indexB][0] + 1;
        } else {
            this->W[indexA][indexB][0] = this->W[indexA][indexB][0] - 1;
        }
        for (size_t i = 0; i < H; i++) {
            if(this->GHR[i]==TAKEN) {
                this->W[indexA][this->GA[i]][i]+=1;
            } else {
                this->W[indexA][this->GA[i]][i]-=1;
            }
        }
    }
    for (size_t i = 0; i < (H-1); i++) {
        this->GA[i+1] = this->GA[i];
        this->GHR[i+1] = this->GHR[i];
    }
    this->GA[0]=indexB;
    this->GHR[0]=correct;
}
