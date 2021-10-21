#include "./../simulator.hpp"
// #############################################################################################################################

void vectorizer_t::allocate(functional_unit_t *mem_op_fu, libconfig::Setting &cfg_root) {


    // Processor defaults
	libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    // Dynamic vectorizer defaults
    libconfig::Setting &cfg_dyn_vec = cfg_root["VECTORIZER"];

    if (!cfg_root.exists("VIMA_CONTROLLER"))
	{
        printf("A VIMA deve existir para que as vetorizações sejam feitas!\n");
        exit(1);

    }

    this->enabled = ((int32_t)cfg_dyn_vec["VECTORIZATION_ENABLED"]) ? true : false;


    this->tl = new table_of_loads_t;
    this->to = new table_of_operations_t;
    this->ts = new  table_of_stores_t;
    this->tv = new table_of_vectorizations_t;
    this->rt = new registers_tracker_t;
    this->ti = new table_of_ignored_t;
    this->tpv = new table_of_pre_vectorization_t;


    this->tl->allocate(cfg_dyn_vec, this->to, this->ts, this->tv, this);

    this->to->allocate(cfg_dyn_vec, this->tl, this->ts, this->tv, this);

    this->ts->allocate(cfg_dyn_vec, this->tl, this->to, this->tv, this);

    this->tv->allocate(cfg_dyn_vec, this->tl, this->to, this->ts, this->ti, this->tpv, this, 
                       cfg_processor["LATENCY_MEM_VIMA"],
                       cfg_processor["WAIT_NEXT_MEM_VIMA"], mem_op_fu);

    this->rt->allocate(this->tl, this->to, this->ts, this->tv, this->ti,this->tpv, this);

    this->ti->allocate(cfg_dyn_vec,this->tl, this->to, this->ts, this);

    this->tpv->allocate(cfg_dyn_vec,this->tl, this->to, this->ts, this);

    this->vectorizations_to_execute.allocate(cfg_dyn_vec["MAX_VECTORIZATIONS_WAITING_LIST"]);

    this->instructions_to_reexecute.allocate((uint32_t)cfg_dyn_vec["MAX_VECTORIZATIONS_WAITING_LIST"] * 4); // Cada vetorização pode gerar até 4 instruções ao ser invalidada

    // *****************
    // Vetorização atual
    // *****************
    this->current_vectorization = NULL;

    // **********
    // Statistics
    // **********
    this->statistics_counters = new uint64_t[VECTORIZER_STATISTIC_LAST];
    for (uint32_t i=0; i < VECTORIZER_STATISTIC_LAST; ++i) {
        this->statistics_counters[i] = 0;
    }

    this->invalidation_point = new uint64_t[(uint32_t)cfg_dyn_vec["VEC_SIZE"] + 1];
    this->invalidation_point_size = (uint32_t)cfg_dyn_vec["VEC_SIZE"] + 1;

    for (uint32_t i=0; i < this->invalidation_point_size; ++i) {
        this->invalidation_point[i] = 0;
    }

    
    this->ready_point = new uint64_t[(uint32_t)cfg_dyn_vec["VEC_SIZE"] + 1];

    for (uint32_t i=0; i < this->invalidation_point_size; ++i) {
        this->ready_point[i] = 0;
    }
    


    // **********
    // Debug info
    // **********
    printf("Vectorizer allocated");
    if (this->enabled) {
        printf("\n");
    } else {
        printf(" but not enabled!\n");
    }

}
// #############################################################################################################################

void vectorizer_t::new_renamed_uop (uop_package_t *uop) {

    if (!this->enabled) return;

    if (uop->reexecution) return;

    switch(uop->uop_operation) {
        case INSTRUCTION_OPERATION_MEM_LOAD:
            this->rt->renamed_ld(uop);
            break;
        case INSTRUCTION_OPERATION_MEM_STORE:
            this->rt->renamed_st(uop);
            break;
        
        case INSTRUCTION_OPERATION_INT_ALU:
        case INSTRUCTION_OPERATION_INT_MUL:
        case INSTRUCTION_OPERATION_INT_DIV:
        case INSTRUCTION_OPERATION_FP_ALU:
        case INSTRUCTION_OPERATION_FP_MUL:
        case INSTRUCTION_OPERATION_FP_DIV:
        case INSTRUCTION_OPERATION_OTHER:
            this->rt->renamed_op(uop);
            break;
        default:
            // Just a common instruction
            this->rt->renamed_op(uop);
            break;
    }

   

}
// #############################################################################################################################

void vectorizer_t::new_committed_uop (uop_package_t *uop) {

    if (!this->enabled) return;

    if (uop->reexecution) return;

    switch(uop->uop_operation) {
        case INSTRUCTION_OPERATION_MEM_LOAD:
            this->rt->committed_ld(uop);
            break;
        case INSTRUCTION_OPERATION_MEM_STORE:
            this->rt->committed_st(uop);
            break;
        
        case INSTRUCTION_OPERATION_INT_ALU:
        case INSTRUCTION_OPERATION_INT_MUL:
        case INSTRUCTION_OPERATION_INT_DIV:
        case INSTRUCTION_OPERATION_FP_ALU:
        case INSTRUCTION_OPERATION_FP_MUL:
        case INSTRUCTION_OPERATION_FP_DIV:
        case INSTRUCTION_OPERATION_OTHER:
            this->rt->committed_op(uop);
            break;
        default:
            // Just a common instruction
            this->rt->committed_op(uop);
            break;
    }

   

}
// #############################################################################################################################


//TODO
uint32_t vectorizer_t::request_invalidation_from_TM(transactional_operation_t *operation) { (void) operation; return 0; }
// #############################################################################################################################

// Reexecuta instruções da vetorização de maneira escalar
// Coloca instruções correspondentes às ignoradas no 'instructions_to_reexecute'
// Marca a entrada na tv para descartar as instruções ignoradas durante o commit
// Remove entradas vinculadas em outras tabelas e a entrada na tv é descartada após o descarte
// de todas as instruções em espera no ROB.
void vectorizer_t::flush_vectorization(table_of_vectorizations_entry_t *tv_entry) {

    // Invalida vetorização, retirando das tabelas e liberando instruções do ROB para descarte
    // Também adiciona instruções para reexecução
    //printf("Flush vectorization\n");
    this->tv->start_invalidation(tv_entry);

}
// #############################################################################################################################

// Toda uop de load/store que é calculada no estágio de execute chama essa
        // função para invalidar conflitos com vetorizações
void vectorizer_t::new_AGU_calculation(uop_package_t *uop) {
    this->tv->new_AGU_calculation(uop);
}

void vectorizer_t::statistics(FILE *output) {
		utils_t::largestSeparator(output);
		fprintf(output, "Vectorizer\n");
        utils_t::largestSeparator(output);
        for (uint32_t i=0; i < VECTORIZER_STATISTIC_LAST; ++i) {
            fprintf(output, "  %s: %lu\n", get_enum_vectorizer_statistic_char((vectorizer_statistic_t)i), this->statistics_counters[i]);
        }

        utils_t::smallSeparator(output);

        fprintf(output, "Invalidation point (next validation when invalidated):\n");
        for (uint32_t i=0; i<this->invalidation_point_size; ++i) {
            fprintf(output, "%u: %lu; ", i, this->invalidation_point[i]);
        }
        fprintf(output, "\n");

        utils_t::smallSeparator(output);

        fprintf(output, "Ready point (next validation when VIMA inst completed):\n");
        for (uint32_t i=0; i<this->invalidation_point_size; ++i) {
            fprintf(output, "%u: %lu; ", i, this->ready_point[i]);
        }
        fprintf(output, "\n");

        utils_t::largeSeparator(output);

        this->tv->statistics(output);

		utils_t::largestSeparator(output);

    
}

void vectorizer_t::increment_counter(vectorizer_statistic_t counter, uint64_t value) {
    this->statistics_counters[counter] += value;
}


/* Caso uma vetorização esteja em andamento e uma instrução externa seja executada,
devemos garantir que ela termine antes da vetorização comitar, dessa forma
evitando uma exceção durante o commit. 
Assim, essa função vincula a instrução externa à vetorização em validação,
para que a vetorização a espere para o commit. */
void vectorizer_t::others_inside (reorder_buffer_line_t *rob_line) {
    if (this->current_vectorization) {
        rob_line->uop.vectorization_linked = this->current_vectorization;
        rob_line->uop.vectorization_linked_id = this->current_vectorization->vectorization_id;
        this->current_vectorization->new_other_inside_renamed(&rob_line->uop);
    }
}

/* Caso uma vetorização esteja em andamento e uma instrução externa seja executada,
devemos garantir que ela termine antes da vetorização comitar, dessa forma
evitando uma exceção durante o commit. 
Assim, essa função marca para a vetorização vinculada que a instrução está pronta */
void vectorizer_t::others_inside_completed (reorder_buffer_line_t *rob_line) {
    if (rob_line->uop.vectorization_linked) {
        rob_line->uop.vectorization_linked->new_other_inside_completed(&rob_line->uop);
    }
}