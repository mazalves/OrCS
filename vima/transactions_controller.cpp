#include "./../simulator.hpp"


 transactions_controller_t::~transactions_controller_t() {
    if (this->entries != NULL) {
        delete[] this->entries;
    }
    num_entries = 0;
}


void transactions_controller_t::allocate() {
    printf("ALLOCATE transactions controller\n");
    // Get configurations
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_dyn_vec = cfg_root["DYNAMIC_VECTORIZATION"];
    printf("GET PARAMETERS transactions controller\n");
    // Set configurations
    this->num_entries = cfg_dyn_vec["MAX_TRANSACTIONS"];
    printf("SET num_entries transactions controller\n");
    this->vectorizer_request_latency = cfg_dyn_vec["VECTORIZER_REQUEST_LATENCY"];
    printf("SET vectorizer_request_latency transactions controller\n");
    // Allocate entries
    this->entries = new transactional_operation_t[this->num_entries]();
    printf("END ALLOCATE transactions controller\n");

}

/*********************************************
Retorna uma entra livre na tabela de operações
Ou NULL caso nenhuma esteja disponível.
********************************************/
transactional_operation_t * transactions_controller_t::get_entry() {
    for (uint32_t i = 0; i < num_entries; ++i) {
        if (this->entries[i].valid == false) {
            return &this->entries[i];
        }
    }
    return NULL;
}


/**********************************************************************
Chamado pelo memory_controller a cada request recebido,
inclusive vindo da VIMA.
Filtra acessos de outros processos e instruções que não
a própria instrução da VIMA que fez a requisição para seus cálculos.
**********************************************************************/
void transactions_controller_t::new_memory_access(memory_package_t* request) {
    bool collision = false;
    for (uint32_t i = 0; i < this->num_entries; ++i) {
        // Ignora entradas já invalidadas
        if (this->entries[i].valid == false) continue;

        // Request da própria instrução:
        if (this->entries[i].request_owner(request)) continue;

        // Entrada diferente, compara para invalidação
        collision = collision || this->entries[i].possible_invalidation(request);
    }
    if (collision) {
        request->updatePackageWaitTM(vectorizer_request_latency + vectorizer_request_latency);
    }
}