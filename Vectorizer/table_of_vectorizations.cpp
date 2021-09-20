#include "./../simulator.hpp"

        // Deve ser chamado antes de qualquer execução
void table_of_vectorizations_t::allocate (libconfig::Setting &vectorizer_configs,
                                          table_of_loads_t *tl, table_of_operations_t *to, table_of_stores_t *ts,
                                          table_of_ignored_t *ti, table_of_pre_vectorization_t *tpv, vectorizer_t *vectorizer, 
                                          uint32_t latency_mem_operation, uint32_t latency_mem_wait, functional_unit_t *mem_op_fu)  { 
        uint32_t size = vectorizer_configs["TV_SIZE"];

        assert (size > 0);

        this->entries = new table_of_vectorizations_entry_t[size];
        for (uint32_t i=0; i<size; ++i) {
            this->entries[i].allocate();
        }

        this->num_entries = 0;
        this->max_entries = size;

        this->vectorization_size = vectorizer_configs["VEC_SIZE"];

        // *************************
        // Link para demais tabelas
        // *************************
        this->tl = tl;
        this->to = to;
        this->ts = ts;
        this->ti = ti;
        this->tpv = tpv;
        this->vectorizer = vectorizer;


        // *******************************************
        // Tempos para criação da instrução em memória
        // *******************************************
        this->mem_operation_latency = latency_mem_operation;
        this->mem_operation_wait_next = latency_mem_wait;
        this->mem_operation_fu = mem_op_fu;

        // **********
        // Statistics
        // **********
        this->vectorizations = 0;
        this->invalidations  = 0;
       
}
// #############################################################################################################################

table_of_vectorizations_entry_t *table_of_vectorizations_t::allocate_entry() {
    // **********************************************************
    // Busca entrada livre ou LRU
    // Apenas seleciona aquelas livres ou em treinamento
    // **********************************************************
    table_of_vectorizations_entry_t *choice = NULL;
    for (uint32_t i=0; i < this->max_entries; ++i) {
        /* No melhor dos casos, pega uma entrada livre */
        if (this->entries[i].free) {
            choice = &this->entries[i];
            break;
        }

        // Tenta pegar qualquer um em treinamento (qualquer possível)
        if (!choice) {
            if (this->entries[i].state < 4) {
                choice = &this->entries[i];
            }
        } else { /* Se já tiver uma escolha, avalia o LRU */
            /* Só avalia o LRU se puder (a outra estiver treinando) */
            if (this->entries[i].state < 4) {
                if (this->entries[i].lru < choice->lru) {
                    choice = &this->entries[i];
                }
            }
        }
    }

    return choice;
}


table_of_vectorizations_entry_t* table_of_vectorizations_t::get_id(uint32_t id) {
            return &this->entries[id];
}

// TODO
instruction_operation_t table_of_vectorizations_t::define_vima_operation(table_of_stores_entry_t *ts_entry) {
    switch(ts_entry->is_mov) {
        case true:
            break;
        case false:
            break;
    }
    return INSTRUCTION_OPERATION_VIMA_FP_ALU;
}
// #############################################################################################################################

table_of_vectorizations_entry_t *table_of_vectorizations_t::new_vectorization (table_of_stores_entry_t *ts_entry) {
        printf(" >> New vectorization -- Addr lista: %p\n", (void *) &this->tl->tv->vectorizer->vectorizations_to_execute);


        // ****************************
        // Aloca uma nova entrada na TV
        // ****************************
        table_of_vectorizations_entry_t *entry = this->allocate_entry();
        if (!entry)
            return NULL;

        if (!entry->free)
            this->start_invalidation(entry);

        // **********
        // Statistics
        // **********
        this->vectorizations++;

        // ****************
        // Preenche entrada
        // ****************
        table_of_loads_entry_t *tl_entries[2];
        table_of_operations_entry_t *to_entry;

        if (ts_entry->is_mov) {
            tl_entries[0] = this->tl->get_id(ts_entry->tl_to_entry);
            tl_entries[1] = NULL;
            to_entry      = NULL;
            assert(tl_entries[0]);
        } else {
            to_entry      = this->to->get_id(ts_entry->tl_to_entry);
            tl_entries[0] = to_entry->tl_entries[0];
            tl_entries[1] = to_entry->tl_entries[1];
            printf("[ TV ] Usando entrada %u da TO\n", this->to->entry_to_id(to_entry));
            assert(to_entry->free == false);
            assert(tl_entries[0]);
            assert(tl_entries[1]);
        }
        assert (tl_entries[0]);
        printf("[1] stride: %d\n", tl_entries[1]->stride);
        printf("[ts] stride: %d\n", ts_entry->stride);
        printf("Ciclo global: %lu\n", orcs_engine.get_global_cycle());
        entry->fill_entry(ts_entry, 0, false, this->vectorization_size, 
                            tl_entries[0]->stride, 
                            (tl_entries[1]) ? tl_entries[1]->stride : 0, 
                            ts_entry->stride, 
                            orcs_engine.get_global_cycle());

        // **********************************
        // Vincula entrada com outras tabelas
        // **********************************
        if (tl_entries[0]) {
            tl_entries[0]->tv_entry = entry;
        }
        if (tl_entries[1]) {
            tl_entries[1]->tv_entry = entry;
        }
        if (to_entry) {
            to_entry->tv_entry = entry;
        }

        ts_entry->tv_entry = entry;

        return entry;
}
// #############################################################################################################################

bool table_of_vectorizations_t::new_pre_vectorization (table_of_stores_entry_t *ts_entry) {
    // **********************************************************    
    // Aloca entrada na TV
    // **********************************************************
    table_of_vectorizations_entry_t *tv_entry = this->new_vectorization(ts_entry);
    if (!tv_entry) {
        return false;
        printf("Faltam entradas para pré-vetorizar\n");
    } else {
        printf("Pré-vetorizei\n");
    }


    // **********************************************************
    // Insere instruções de load e store para ajuste de endereços
    // **********************************************************
    table_of_loads_entry_t *tl_entries[2];

    if (ts_entry->is_mov) {
        tl_entries[0] = this->tl->get_id(ts_entry->tl_to_entry);
        tl_entries[1] = NULL;
    } else {
        table_of_operations_entry_t *to_entry = this->to->get_id(ts_entry->tl_to_entry);
        tl_entries[0] = to_entry->tl_entries[0];
        tl_entries[1] = to_entry->tl_entries[1];
    }
    //// Load 1
    if (tl_entries[0]) {
        this->tpv->insert(tl_entries[0]->get_pc(), tl_entries[0]->get_uop_id(), tv_entry);
    }
    //// Load 2
    if (tl_entries[1]) {
        this->tpv->insert(tl_entries[1]->get_pc(), tl_entries[1]->get_uop_id(), tv_entry);
    }
    //// Store
    this->tpv->insert(ts_entry->get_pc(), ts_entry->get_uop_id(), tv_entry);

    // *************************
    // Ajusta máquina de estados
    // *************************
    tv_entry->state = 0;

    return true;
}
// #############################################################################################################################

void table_of_vectorizations_t::start_invalidation (table_of_vectorizations_entry_t *tv_entry) {
    printf("INVALIDATION ENTRY %u\n", this->entry_to_id(tv_entry));
    // Executa escalarmente e libera as instruções em espera
    this->invalidate(tv_entry);

    // Para de ignorar as instruções vinculadas
    this->ti->remove_vectorization(tv_entry);

    // Para de treinar a pré-vetorização, se estiver treinando
    this->tpv->remove_vectorization(tv_entry);

    // Remove dados das tabelas de treinamento
    if (tv_entry->ts_entry) {
        if (tv_entry->ts_entry->linked_tl_to) {
            if (tv_entry->ts_entry->is_mov) {
                // Invalida TL
                this->tl->invalidate(this->tl->get_id(tv_entry->ts_entry->tl_to_entry));
            } else {
                table_of_operations_entry_t *to_pointer = this->to->get_id(tv_entry->ts_entry->tl_to_entry);
                // Invalida TL
                this->tl->invalidate(to_pointer->tl_entries[0]);
                this->tl->invalidate(to_pointer->tl_entries[1]);
    
                // Invalida TO
                this->to->invalidate(to_pointer);
            }
    
            // Invalida TS
            this->ts->invalidate(tv_entry->ts_entry);
        }
    }

    // Limpa a própria entrada
    /* É limpa quando todos comitarem */


}
// #############################################################################################################################

// Deve invalidar a vetorização da própria tabela e reexecutar operações
void table_of_vectorizations_t::invalidate(table_of_vectorizations_entry_t *tv_entry) {

    // ****************
    // Já está liberado
    // ****************
    if (tv_entry->discard_results) {
        return;
    }

    // **********
    // Statistics
    // **********
    this->invalidations++;
    
    // *****************************************
    // Libera instruções para descarte no commit
    // *****************************************
    tv_entry->discard_results = true;
        
    // *****************************
    // Insere instruções no pipeline
    // *****************************
    int32_t size = tv_entry->next_validation;
    if (tv_entry->ts_entry->is_mov) {
        for (int32_t i = 0; i < size; ++i) {
            // Ajusta endereços
            tv_entry->uops[0].memory_address[0] = tv_entry->addr_mem_load[0] + (tv_entry->load_stride[0] * i);
            tv_entry->uops[3].memory_address[0] = tv_entry->addr_mem_store + (tv_entry->store_stride * i);

            // Envia load e store
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[0]);
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[3]);

        }

        //// Resto
        if (tv_entry->inst_inside_ROB_bitmask[0]) {
            // Ajusta endereço
            tv_entry->uops[0].memory_address[0] = tv_entry->addr_mem_load[0] + (tv_entry->load_stride[0] * size);

            // Envia apenas o load restante
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[0]);
        }

    } else { // Loads + op + store
        for (int32_t i = 0; i < size; ++i) {
            // Ajusta endereços
            tv_entry->uops[0].memory_address[0] = tv_entry->addr_mem_load[0] + (tv_entry->load_stride[0] * i);
            tv_entry->uops[1].memory_address[0] = tv_entry->addr_mem_load[1] + (tv_entry->load_stride[1] * i);
            tv_entry->uops[3].memory_address[0] = tv_entry->addr_mem_store   + (tv_entry->store_stride   * i);

            // Envia as operações
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[0]);
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[1]);
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[2]);
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[3]);

        }

        //// Resto
        if (tv_entry->inst_inside_ROB_bitmask[0]) { // Load 1
            tv_entry->uops[0].memory_address[0] = tv_entry->addr_mem_load[0] + (tv_entry->load_stride[0] * (size));
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[0]);
        }
        if (tv_entry->inst_inside_ROB_bitmask[1]) { // Load 2
            tv_entry->uops[1].memory_address[0] = tv_entry->addr_mem_load[1] + (tv_entry->load_stride[1] * (size));
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[1]);
        }
        if (tv_entry->inst_inside_ROB_bitmask[2]) { // Op
            this->vectorizer->instructions_to_reexecute.push_back(tv_entry->uops[2]);
        }

    }

}
// #############################################################################################################################

// Return id from entry
uint32_t table_of_vectorizations_t::entry_to_id(table_of_vectorizations_entry_t *entry) {
    uint32_t id = ((uint64_t)entry - (uint64_t)this->entries) / sizeof(table_of_vectorizations_entry_t);
    assert(&this->entries[id] == entry);
    return id;
}
// #############################################################################################################################

void table_of_vectorizations_t::register_overwritten (table_of_vectorizations_entry_t *entry) {
    // Não faz sentido liberar algo já sobrescrito :p
    assert (entry->is_ready_for_commit() == false);
    
    entry->register_replace();

    if (entry->is_ready_for_commit()) {

        table_of_loads_entry_t *tl_entries[2];
        table_of_operations_entry_t *to_entry;

        if (entry->ts_entry->is_mov) {
            tl_entries[0] = this->tl->get_id(entry->ts_entry->tl_to_entry);
            tl_entries[1] = NULL;
            to_entry      = NULL;
        } else {
            to_entry      = this->to->get_id(entry->ts_entry->tl_to_entry);
            tl_entries[0] = to_entry->tl_entries[0];
            tl_entries[1] = to_entry->tl_entries[1];
        }

        if (tl_entries[0]) {
            tl_entries[0]->clean();
        }
        if (tl_entries[1]) {
            tl_entries[1]->clean();
        }
        if (to_entry) {
            to_entry->clean();
        }

        entry->ts_entry->clean();
        printf("Tudo pronto para o commit! Todos os registradores sobrescritos\n");
    
    }
}
// #############################################################################################################################

void table_of_vectorizations_t::generate_VIMA_instruction(table_of_vectorizations_entry_t *entry) {
        
        // ******************************************
        // Cria uma instrução para enviar para a VIMA
        // ******************************************

        opcode_package_t base_opcode;
        uop_package_t base_uop;
        base_opcode.package_clean();
        base_uop.package_clean();

        table_of_loads_entry_t *tl_entries[2];
        table_of_operations_entry_t *to_entry;
        table_of_stores_entry_t *ts_entry;

        if (entry->ts_entry->is_mov) {
            tl_entries[0] = this->tl->get_id(entry->ts_entry->tl_to_entry);
            tl_entries[1] = NULL;
            to_entry      = NULL;
        } else {
            to_entry      = this->to->get_id(entry->ts_entry->tl_to_entry);
            tl_entries[0] = to_entry->tl_entries[0];
            tl_entries[1] = to_entry->tl_entries[1];
        }
        ts_entry = entry->ts_entry;

		base_uop.opcode_to_uop(0, define_vima_operation(ts_entry),
								this->mem_operation_latency, this->mem_operation_wait_next, this->mem_operation_fu,
								base_opcode, 0);
		base_uop.add_memory_operation(0, 1);
        base_uop.add_vectorization_reference(entry);

		
		base_uop.is_hive = false;
		base_uop.hive_read1 = -1;
		base_uop.hive_read2 = -1;
		base_uop.hive_write = -1;

		base_uop.is_vima = true;
        /* Começa a partir da que usou para identificar os addrs iniciais, por isso sem o + stride */
		base_uop.read_address = tl_entries[0]->last_address;
		base_uop.read2_address = (tl_entries[1]) ? tl_entries[1]->last_address : 0;
		base_uop.write_address = ts_entry->last_address;

		base_uop.updatePackageWait(0);
		base_uop.born_cycle = orcs_engine.get_global_cycle();

        this->vectorizer->vectorizations_to_execute.push_back(base_uop);
        printf("Instrução VIMA inserida!\n");
}

// Verifica o stride de uma instrução waiting após sua passagem pela AGU
void table_of_vectorizations_t::verify_stride(uop_package_t *uop) {
    assert (uop->tv_pointer);
    if (uop->tv_pointer->verify_stride(uop) == false) {
        printf("Stride invalidation\n");
        this->start_invalidation(uop->tv_pointer);
    }
}
// #############################################################################################################################

// Apenas remove as entradas correpondentes nas tabelas de treinamento
// Isso é útil caso já tenha convertido todas as instruções em waiting
// e queira apenas esperar seu fim, sem vetorizar à frente
// (por falta de entradas na TV :P)
void table_of_vectorizations_t::unbind (table_of_vectorizations_entry_t *tv_entry) {
    // Remove dados das tabelas de treinamento
    if (tv_entry->ts_entry->linked_tl_to) {
        if (tv_entry->ts_entry->is_mov) {
            // Invalida TL
            this->tl->invalidate(this->tl->get_id(tv_entry->ts_entry->tl_to_entry));
        } else {
            table_of_operations_entry_t *to_pointer = this->to->get_id(tv_entry->ts_entry->tl_to_entry);
            // Invalida TL
            this->tl->invalidate(to_pointer->tl_entries[0]);
            this->tl->invalidate(to_pointer->tl_entries[1]);

            // Invalida TO
            this->to->invalidate(to_pointer);
        }

        // Invalida TS
        this->ts->invalidate(tv_entry->ts_entry);
    }
    tv_entry->ts_entry = NULL;
}
// #############################################################################################################################

// Cada load executado pode invalidar uma vetorização, caso carregue dados do seu destino
// Cada store executado pode invalidar uma vetorização, caso carregue dados do sua origem
// Isso pode acontecer entre duas vetorizações (instrução em waiting) ou uma instrução comum e uma vetorização
// Isso porque não sabemos se devemos fornecer os dados novos ou antigos da linha
// TODO -> Corrigir considerando o tamanho do dado carregado
void table_of_vectorizations_t::new_AGU_calculation (uop_package_t *uop) {
    table_of_vectorizations_entry_t *entry = NULL;
    switch(uop->opcode_operation) {
        case INSTRUCTION_OPERATION_MEM_LOAD:
            for (uint32_t i=0; i < this->max_entries; ++i) {
                entry = &this->entries[i];
                if (utils_t::intersect(uop->memory_address[0], uop->memory_size[0], entry->addr_mem_store, (entry->after_last_byte_mem_store - entry->addr_mem_store))) {
                    printf("Leitura em endereço de escrita da vetorização %p (Leitura: %lu; Escritas: [%lu -- %lu)\n",
                            (void *) entry, uop->memory_address[0], entry->addr_mem_store, entry->after_last_byte_mem_store);
                    this->start_invalidation(entry);
                    }
            }
            break;
        case INSTRUCTION_OPERATION_MEM_STORE:
            for (uint32_t i=0; i < this->max_entries; ++i) {
                entry = &this->entries[i];
                if (utils_t::intersect(uop->memory_address[0], uop->memory_size[0], entry->addr_mem_load[0], (entry->after_last_byte_mem_load[0] - entry->addr_mem_load[0]))) {
                        printf("Escrita em endereço de leitura da vetorização %p (Escrita: %lu; Leituras: [%lu -- %lu)\n",
                                (void *) entry, uop->memory_address[0], entry->addr_mem_load[0], entry->after_last_byte_mem_load[0]);
                        this->start_invalidation(entry);
                    } else if (utils_t::intersect(uop->memory_address[0], uop->memory_size[0], entry->addr_mem_load[1], (entry->after_last_byte_mem_load[1] - entry->addr_mem_load[1]))) {
                        printf("Escrita em endereço de leitura da vetorização %p (Escrita: %lu; Leituras: [%lu -- %lu)\n",
                                (void *) entry, uop->memory_address[0], entry->addr_mem_load[1], entry->after_last_byte_mem_load[1]);
                        this->start_invalidation(entry);
                    }
            }
            break;

        default:
            printf("table_of_vectorizations_t::calculated_mem_access >> Instrução inesperada! (%lu[%u])\n", uop->opcode_address, uop->uop_id);
            exit(1);
    }
}

void table_of_vectorizations_t::statistics(FILE *output) {

    fprintf(output, "Table of vectorizations (TL)\n");
    fprintf(output, "  Vectorizations: %lu\n", this->vectorizations);
    fprintf(output, "  Invalidations:  %lu\n", this->invalidations);
    fprintf(output, "  Vectorization size: %u\n", this->vectorization_size);

}