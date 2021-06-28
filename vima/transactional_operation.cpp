
#include "./../simulator.hpp"

// Retorna true se invalidou
// Retorna false se não invalidou
bool transactional_operation_t::possible_invalidation(memory_package_t* request) {
    bool invalidou = false;
    uint32_t ultimaValidacao = 0;
    // TODO
    // LOAD
    if (request->memory_operation == MEMORY_OPERATION_READ ||
        request->memory_operation == MEMORY_OPERATION_INST) {
        // Se carregar dados de um operando
        // OK, não gera problema

        // Se carregar dados do destino
        //// Precisa para não invalidar em escrita de xTemp em xValid (poderia acontecer logo)
        //// E lidar com barreiras
        if (this->intersect(request->memory_address, request->memory_size,
                            this->destiny_addr, this->operation_size))
        {
            ultimaValidacao = orcs_engine.processor->vectorizer->request_invalidation_from_TM(this);
            
            // Registra aproveitamento
            total_validated = ultimaValidacao + 1;
            total_discarded = operation_size - ultimaValidacao - 1;
            
            // Faz writeback, se necessário -> aparentemente é instantâneo, algum write buffer :P
            //...
            invalidou = true;
        }
    }

    // STORE
    else if (request->memory_operation == MEMORY_OPERATION_WRITE) {
        // Salva dados em um operando
        //// Primeiro operando
        //// Segundo operando
         // Salva dados no destino
        if (this->intersect(request->memory_address, request->memory_size,
                            this->source_addr[0], this->operation_size)  ||
            this->intersect(request->memory_address, request->memory_size,
                            this->source_addr[1], this->operation_size)  ||
            this->intersect(request->memory_address, request->memory_size,
                            this->destiny_addr, this->operation_size))
        {
            ultimaValidacao = orcs_engine.processor->vectorizer->request_invalidation_from_TM(this);
            
            // Registra aproveitamento
            total_validated = ultimaValidacao + 1;
            total_discarded = operation_size - ultimaValidacao - 1;
            
            // Faz writeback, se necessário -> aparentemente é instantâneo, algum write buffer :P
            //...
            invalidou = true;
        }
    }

    return invalidou;
}