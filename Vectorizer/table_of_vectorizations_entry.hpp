class table_of_vectorizations_entry_t {
    public:
        table_of_stores_entry_t *ts_entry; /* Entrada correspondente na TS */

        // *********
        // Validação
        // *********
        int32_t next_validation; /* Próxima prosição a ser validada */
        uint8_t remaining_registers; /* Registradores a sobrescrever antes de validação */

        int8_t state; /* 0 -> Esperando confirmação do Rename;
                         1 -> Endereço da primeira leitura adquirido, começando a ignorar as originais;
                         2 -> Endereço da segunda leitura adquirido;
                         3 -> Endereço do store adquirido; 
                         4 -> Enviado, processando VIMA;
                         5 -> Todas ignoradas, esperando substituição nos registradores*/

        /* Endereços pré-vetorização */
        uint64_t addr_mem_load[2];
        uint64_t addr_mem_store;

        /* Endereços finais da operação */
        // Usados para invalidar no caso de loads e stores conflitantes
        uint64_t after_last_byte_mem_load[2];
        uint64_t after_last_byte_mem_store;



        bool ready; /* Instrução VIMA executada */
        int32_t num_elements; /* Tamanho da vetorização */

        int64_t load_stride[2];
        int64_t store_stride;
        uint32_t need_confirmation;

        bool ready_for_commit;
        int32_t committed_elements; // Número de instruções que já comitaram
                                    // Quando todas comitarem, pode descartar a entrada

        bool discard_results; // Caso tenha sido invalidada, marca para descartar suas ignoradas no commit
        int32_t inst_inside_ROB; /* Número de instruções ignoradas no ROB */
                                 /* Usado para caso os resultados sejam descartados, saber quando essa entrada pode ser liberada */

        uint8_t next_inside_ROB;           // Contador indicando as últimas instruções adicionadas ao ROB
                                         // Resetada quando o store é adicionado
                                         // 0 -> Load 1
                                         // 1 -> Load 2
                                         // 2 -> Op
                                         // 3 -> store
                                         // Também é utilizada para verificar se foram adicionadas na ordem certa

        /* Cópias das instruções utilizadas para a vetorização */
        uop_package_t uops[4];

        uint16_t others_inside_vectorization; // Para que o commit comece, todas as demais instruções que
                                              // estão no ROB entre as em waiting devem ser completadas sem
                                              // exceções. Assim, cada instrução que não entra em waiting
                                              // enquanto coletamos as instruções em waiting incrementa este contador
                                              // Conforme são completadas, as instruções decrementam esse valor.
                                              // Caso esta entrada na tv seja invalidada, uma verificação do valor de
                                              // vectorization_id indica que o decremento não se refere mais àquela vetorização.
        uint64_t time_last_other_completed;     // Instante de readAt no qual a última outra instrução ficou pronta para 
                                                // o commit
        
        uint8_t vectorization_id;   // Identifica unicamente uma vetorização e é incrementado a cada vetorização realizada.
                                    // Devido ao tamanho do ROB, é impossível que duas vetorizações obtenham o mesmo id.

        uint64_t lru;
        bool free;

        /* Vetorização da próxima parte, caso tenha terminado */
        table_of_vectorizations_entry_t *next;

        /* Vetorização anterior, caso tenha substituido alguma */
        table_of_vectorizations_entry_t *prev;

        // Vectorizer
        table_of_vectorizations_t *tv;

        table_of_vectorizations_entry_t () {
            this->ts_entry = NULL;
            this->next_validation = 0;
            this->remaining_registers = 0;

            this->state = 0;
            this->addr_mem_load[0] = 0x0;
            this->addr_mem_load[1] = 0x0;
            this->addr_mem_store   = 0x0;

            this->uops[0].package_clean(); // Load 1
            this->uops[1].package_clean(); // Load 2
            this->uops[2].package_clean(); // Op
            this->uops[3].package_clean(); // Store


            this->ready = false;
            this->num_elements = 0;

            this->load_stride[0] = 0;
            this->load_stride[1] = 0;
            this->store_stride = 0;
            this->need_confirmation = 0;

            this->ready_for_commit = false;
            this->committed_elements = 0;

            this->discard_results = false;
            this->inst_inside_ROB = 0;
            this->next_inside_ROB = 0;

            this->time_last_other_completed = 0;
            this->others_inside_vectorization = 0;
            this->vectorization_id = 0;
            this->lru = 0;
            this->free = true;

            this->tv = NULL;
        }

        inline void update_lru(uint64_t lru);


        inline void allocate(table_of_vectorizations_t *tv);

        inline void clean();

        inline void fill_entry(table_of_stores_entry_t *ts_entry,
                              uint32_t next_validation,
                              bool ready,
                              uint32_t num_elements,
                              int64_t load_stride_1,
                              int64_t load_stride_2,
                              int64_t store_stride,
                              uint64_t lru,
                              uint8_t id);

        // Basicamente confere se todos os registradores das iterações foram substituídos sem problemas
        // (Verificar se são utilizados por alguém antes de sobrescritos :p)
        inline void register_replace();

        inline void register_add (uint8_t num_registers);

        inline void increment_validation ();

        inline bool has_ended ();

        inline bool is_able_to_commit();

        inline bool is_ready_for_commit();

        inline void why_ready_for_commit();

        inline void set_state (int8_t state);

        inline int8_t get_state();

        inline void new_commit (uop_package_t *uop);

        inline void new_waiting (uop_package_t *uop, uint8_t structural_id);

        inline bool verify_stride(uop_package_t *uop);

        inline void set_ready (); // Quando vima executar e retornar

        inline void print ();

        inline void new_other_inside_renamed (uop_package_t *uop);
        inline void new_other_inside_completed (uop_package_t *uop);

        inline void verify_completion();

};


inline void table_of_vectorizations_entry_t::update_lru(uint64_t lru) {
    this->lru = lru;
}


inline void table_of_vectorizations_entry_t::allocate(table_of_vectorizations_t *tv) {
    this->ts_entry = NULL;
    this->next_validation = 0;
    this->remaining_registers = 0;

    this->state = 0;
    this->addr_mem_load[0] = 0x0;
    this->addr_mem_load[1] = 0x0;
    this->addr_mem_store   = 0x0;

    this->after_last_byte_mem_load[0] = 0x0;
    this->after_last_byte_mem_load[1] = 0x0;
    this->after_last_byte_mem_store   = 0x0;

    this->ready = false;
    this->num_elements = 0;

    this->load_stride[0] = 0;
    this->load_stride[1] = 0;
    this->store_stride = 0;
    this->need_confirmation = 0;

    this->ready_for_commit = false;
    this->committed_elements = 0;

    this->discard_results = false;
    this->inst_inside_ROB = 0;
    this->next_inside_ROB = 0;

    this->time_last_other_completed = 0;
    this->others_inside_vectorization = 0;
    this->vectorization_id = 0;
    this->lru = 0;
    this->free = true;

    this->next = NULL;
    this->prev = NULL;

    this->tv = tv;
}

inline void table_of_vectorizations_entry_t::clean() {
    this->ts_entry = NULL;
    this->next_validation = 0;
    this->remaining_registers = 0;

    this->state = 0;
    this->addr_mem_load[0] = 0x0;
    this->addr_mem_load[1] = 0x0;
    this->addr_mem_store   = 0x0;
    
    this->after_last_byte_mem_load[0] = 0x0;
    this->after_last_byte_mem_load[1] = 0x0;
    this->after_last_byte_mem_store   = 0x0;

    this->uops[0].package_clean(); // Load 1
    this->uops[1].package_clean(); // Load 2
    this->uops[2].package_clean(); // Op
    this->uops[3].package_clean(); // Store

    this->ready = false;
    this->num_elements = 0;

    this->load_stride[0] = 0;
    this->load_stride[1] = 0;
    this->store_stride = 0;
    this->need_confirmation = 0;

    this->ready_for_commit = false;
    this->committed_elements = 0;

    this->discard_results = false;
    this->inst_inside_ROB = 0;
    this->next_inside_ROB = 0;

    this->time_last_other_completed = 0;
    this->others_inside_vectorization = 0;
    this->vectorization_id = 0;
    this->lru = 0;
    this->free = true;
    //printf("Cleaning TV %p\n", (void *)this);

    /* Unlink from next */
    if (this->next) {
        this->next->prev = NULL;
    }

    if (this->prev) {
        this->prev->next = NULL;
    }

    this->next = NULL;
    this->prev = NULL;
}

inline void table_of_vectorizations_entry_t::fill_entry(table_of_stores_entry_t *ts_entry,
                        uint32_t next_validation,
                        bool ready,
                        uint32_t num_elements,
                        int64_t load_stride_1,
                        int64_t load_stride_2,
                        int64_t store_stride,
                        uint64_t lru,
                        uint8_t id) {
    this->ts_entry = ts_entry;
    this->next_validation = next_validation;
    this->remaining_registers = 0;
    this->ready = ready;
    this->num_elements = num_elements;

    this->load_stride[0] = load_stride_1;
    this->load_stride[1] = load_stride_2;
    this->store_stride   = store_stride;
    this->need_confirmation = 0;

    this->ready_for_commit = false;
    this->committed_elements = 0;
    this->discard_results = false;
    this->inst_inside_ROB = 0;
    this->next_inside_ROB = 0;


    this->time_last_other_completed = 0;
    this->others_inside_vectorization = 0;
    this->vectorization_id = id;
    this->lru = lru;
    this->free = false;

}

// Basicamente confere se todos os registradores das iterações foram substituídos sem problemas
// (Verificar se são utilizados por alguém antes de sobrescritos :p)
inline void table_of_vectorizations_entry_t::register_replace() {
    this->update_lru(orcs_engine.get_global_cycle());
        //printf("Remaining registers: %u -> %u\n", this->remaining_registers, this->remaining_registers - 1);
        
        assert (this->remaining_registers > 0); 
        --this->remaining_registers;
        //printf(" * Restam %u substituições!\n", this->remaining_registers);

        if (this->remaining_registers == 0 && 
            this->next_validation >= this->num_elements && 
            this->need_confirmation == 0 && 
            this->discard_results == false && 
            this->ready && 
            this->others_inside_vectorization == 0 &&
            this->time_last_other_completed <= orcs_engine.get_global_cycle()) {
            this->ready_for_commit = true;
            //printf("register_replace => READY FOR COMMIT %p!!!\n", (void *)this);
            /*for (uint32_t i=0; i < MAX_REGISTERS; ++i) {
                registers_tracker_entry_t *entry = orcs_engine.processor->get_tv_register_id(i);
                if (entry->tv_pointer == this) {
                    printf("ERRO! SOBROU UMA ENTRADA!\n");
                    exit(1);
                }
            }*/
        }
        else if (this->discard_results == true && this->inst_inside_ROB == 0 && this->remaining_registers == 0 && this->ready) {
            // Pode apagar
            //printf("register_replace => Todas as instruções esperando foram descartadas e os registradores substituídos! Apagando a entrada na TV (%p)\n",(void *)this);
            orcs_engine.processor->vectorizer->increment_counter(VECTORIZER_SUCCESSFULLY_DISCARDED_VECTORIZATION, 1);            
            this->clean();
        }
}

inline void table_of_vectorizations_entry_t::register_add (uint8_t num_registers) {
    this->update_lru(orcs_engine.get_global_cycle());
    
    //printf("Incrementing remaining registers: %u -> %u\n", this->remaining_registers, this->remaining_registers + num_registers);
    this->remaining_registers += num_registers;
}

inline void table_of_vectorizations_entry_t::increment_validation () {
    this->update_lru(orcs_engine.get_global_cycle());
    if (this->next_validation < this->num_elements) {
        this->next_validation += 1;
    }
    //printf("Validação incrementada para %d\n", this->next_validation);
}

inline bool table_of_vectorizations_entry_t::has_ended () {
    if (this->next_validation == this->num_elements) {
        return true;
    }
    return false;
}

inline bool table_of_vectorizations_entry_t::is_able_to_commit() {
    if (!(ready_for_commit || this->discard_results)) {
        this->verify_completion();
    }
    return (ready_for_commit || this->discard_results);
}

inline bool table_of_vectorizations_entry_t::is_ready_for_commit() {
    return ready_for_commit;
}

inline void table_of_vectorizations_entry_t::why_ready_for_commit() {
    printf("[%p] Validação: %d/%d, Remaining regs: %d Confirmations remaining: %u Ready: %s Others incompleted: %u\n", (void *) this, this->next_validation, this->num_elements, this->remaining_registers, this->need_confirmation, this->ready ? "true" : "false", this->others_inside_vectorization);

    assert (this->num_elements > 0);
}

inline void table_of_vectorizations_entry_t::set_state (int8_t state) {
    this->update_lru(orcs_engine.get_global_cycle());
    this->state = state;
}

inline int8_t table_of_vectorizations_entry_t::get_state() {
    return this->state;
}

inline void table_of_vectorizations_entry_t::new_commit (uop_package_t *uop) {
    if (this->discard_results) {
        assert (uop->waiting);
        --this->inst_inside_ROB;
        if (this->inst_inside_ROB == 0 && this->remaining_registers == 0 && this->ready) {
            // Pode apagar
            //printf("new_commit => Todas as instruções esperando foram descartadas e os registradores substituídos! Apagando a entrada na TV (%p)\n",(void *)this);
            orcs_engine.processor->vectorizer->increment_counter(VECTORIZER_SUCCESSFULLY_DISCARDED_VECTORIZATION, 1);
            this->clean();
        }
    } else {
        /* Pra liberar o commit já deve ter verificado os registradores e estar pronta, então só commita */
        if (uop->uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
            ++(this->committed_elements);
            //printf("%p -> %d committed elements\n", (void *)this, this->committed_elements);
            assert (this->committed_elements <= this->num_elements);
            if (this->committed_elements == this->num_elements) {
                // Pode apagar
                orcs_engine.processor->vectorizer->increment_counter(VECTORIZER_SUCCESSFULLY_COMPLETED_VECTORIZATION, 1);
                this->clean();

                //printf("Último commit, apagando a entrada na TV (%p)\n",(void *)this);
            }
        }
    }
}

inline void table_of_vectorizations_entry_t::new_waiting (uop_package_t *uop, uint8_t structural_id) {
    ++this->inst_inside_ROB;
    assert (structural_id < 4);

    // Se não era o que esperava, invalida (isso acaba impedindo saltos para instruções internas :p)
    if (structural_id != this->next_inside_ROB) {
        this->tv->start_invalidation(this);
        return;
    }

    // Próximo
    // // Se for mov faz pular para o 3
    if (structural_id == 0 && this->ts_entry->is_mov) {
        this->next_inside_ROB += 2;
    }

    this->next_inside_ROB++;

    // // Se tiver sido um store vai para o 0
    if (this->next_inside_ROB > 3) {
        this->next_inside_ROB = 0;
    }


    // Faz esperar confirmação do stride (0, 1 e 3)
    if (structural_id != 2) {
        //printf("Confirmações necessárias incrementadas: %u -> %u\n", this->need_confirmation, this->need_confirmation + 1);
        this->need_confirmation++;
    }

    // Salva para caso precise reexecutar
    if (this->uops[structural_id].opcode_operation == INSTRUCTION_OPERATION_NOP) {
        this->uops[structural_id] = *uop;
        this->uops[structural_id].reexecution = true;
        this->uops[structural_id].waiting = false;
        
    }
    
}

inline bool table_of_vectorizations_entry_t::verify_stride(uop_package_t *uop) {
    uint64_t mem_addr = uop->memory_address[0];
    int64_t stride = 0;
    switch (uop->structural_id)
    {
    case 0:
        stride = mem_addr - (this->addr_mem_load[0]);
        //printf ("Endereço acessado pelo primeiro load: %lu (load base: %lu)\n", mem_addr, (this->addr_mem_load[0]));
        if (stride != this->load_stride[0]*uop->validation_number) {
            //printf("Stride do primeiro load (Addr %lu) incompatível para a validação %u! %lu != %lu [%ld * %u]\n", uop->opcode_address, uop->validation_number, stride, this->load_stride[0]*uop->validation_number, this->load_stride[0],uop->validation_number);
            return false;
        }
        break;
    case 1:
        stride = mem_addr - (this->addr_mem_load[1]);
        //printf ("Endereço acessado pelo segundo load: %lu (load base: %lu)\n", mem_addr, (this->addr_mem_load[1]));
        if (stride != this->load_stride[1]*uop->validation_number) {
            //printf("Stride do segundo load (Addr %lu) incompatível para a validação %u! %lu != %lu [%ld * %u]\n", uop->opcode_address, uop->validation_number, stride, this->load_stride[1]*uop->validation_number, this->load_stride[1],uop->validation_number);
            return false;
        }
        break;


    case 3:
        stride = mem_addr - (this->addr_mem_store);
        //printf ("Endereço acessado pelo store: %lu (store base: %lu)\n", mem_addr, (this->addr_mem_store));
        if (stride != this->store_stride*uop->validation_number) {
            //printf("Stride do store (Addr %lu) incompatível para a validação %u! %lu != %lu [%ld * %u]\n", uop->opcode_address, uop->validation_number, stride, this->store_stride*uop->validation_number, this->store_stride,uop->validation_number);
            return false;
        }
        break;


    default:
        printf("table_of_vectorizations_entry_t::verify_stride => Uop (%lu [%u]) de structural_id inesperado %d recebida!\n", uop->opcode_address, uop->uop_id, uop->uop_operation);
        exit(1);
        break;
    }
    this->need_confirmation--;

    if (this->remaining_registers == 0 && 
        this->next_validation >= this->num_elements && 
        this->need_confirmation == 0 &&
        this->ready &&
        this->others_inside_vectorization == 0 &&
        this->time_last_other_completed <= orcs_engine.get_global_cycle()) {
        
        this->ready_for_commit = true;
        //printf("verify_stride => READY FOR COMMIT %p!!!\n", (void *)this);
        /*for (uint32_t i=0; i < MAX_REGISTERS; ++i) {
                registers_tracker_entry_t *entry = orcs_engine.processor->get_tv_register_id(i);
                if (entry->tv_pointer == this) {
                    printf("ERRO! SOBROU UMA ENTRADA!\n");
                    exit(1);
                }
            }*/
    }

    //printf("Stride do %s confirmado! (%u => %u)\n", (uop->structural_id == 0) ? "primeiro load" : (uop->structural_id == 1) ? "segundo load" : (uop->structural_id == 3) ? "store" : "outro", this->need_confirmation+1, this->need_confirmation);
    return true;
}

inline void table_of_vectorizations_entry_t::set_ready () {
    this->ready = true;


    // ***********************
    // Tenta liberar a entrada
    // ************************
    if (this->remaining_registers == 0 && 
        this->next_validation >= this->num_elements && 
        this->need_confirmation == 0 && 
        this->discard_results == false && 
        this->ready &&
        this->others_inside_vectorization == 0 &&
        this->time_last_other_completed <= orcs_engine.get_global_cycle()) {

        this->ready_for_commit = true;
    }
    else if (this->discard_results == true && this->inst_inside_ROB == 0 && this->remaining_registers == 0 && this->ready) {
        // Pode apagar
        //printf("set_ready => Todas as instruções esperando foram descartadas e os registradores substituídos! Apagando a entrada na TV (%p)\n",(void *)this);
        orcs_engine.processor->vectorizer->increment_counter(VECTORIZER_SUCCESSFULLY_DISCARDED_VECTORIZATION, 1);            
        this->clean();
    }
}

inline void table_of_vectorizations_entry_t::print () {
    printf("  ts_entry: %p  next_validation: %d  remaining_registers: %u  state: %d  ready: %s  need_confirmation: %u  ready_for_commit: %s discard_results: %s\n",
            (void *)this->ts_entry,  this->next_validation,  this->remaining_registers, this->state,  this->ready ? "true" : "false",  this->need_confirmation,  this->ready_for_commit ? "true" : "false", this->discard_results ? "true" : "false");
}

inline void table_of_vectorizations_entry_t::new_other_inside_renamed (uop_package_t *uop) {
    this->others_inside_vectorization++;
    uop->vectorization_linked_id = this->vectorization_id;
    uop->vectorization_linked = this;
}

inline void table_of_vectorizations_entry_t::new_other_inside_completed (uop_package_t *uop) {
    if (this->vectorization_id == uop->vectorization_linked_id) {
        assert (this->others_inside_vectorization);
        this->others_inside_vectorization--;
        if (this->time_last_other_completed < uop->readyAt) {
            this->time_last_other_completed = uop->readyAt;
        }

        if (this->remaining_registers == 0 && 
            this->next_validation >= this->num_elements && 
            this->need_confirmation == 0 && 
            this->discard_results == false && 
            this->ready &&
            this->others_inside_vectorization == 0 &&
            this->time_last_other_completed <= orcs_engine.get_global_cycle()) {

            this->ready_for_commit = true;
        }
    }
}

// #####################################################################
// O commit precisa verificar se conseguiu os requisitos para completar,
// já que pode precisar esperar o tempo de outras terminarem
// #####################################################################
inline void table_of_vectorizations_entry_t::verify_completion() {
    if (this->remaining_registers == 0 && 
            this->next_validation >= this->num_elements && 
            this->need_confirmation == 0 && 
            this->discard_results == false && 
            this->ready &&
            this->others_inside_vectorization == 0 &&
            this->time_last_other_completed <= orcs_engine.get_global_cycle()) {

            this->ready_for_commit = true;
        }
}