class conversion_status_t {
    public:
        uint64_t unique_conversion_id;
        bool conversion_started; // Conversion enabled, pattern found and first iteration to alignment calculated
        uint64_t conversion_beginning; // First iteration to convert
        uint64_t conversion_ending;    // Last iteration to convert

        uint32_t infos_remaining; // Addresses needed from AGU to generate VIMA instruction
        int64_t mem_addr_confirmations_remaining;
        int32_t wait_reg_deps_number;
        uint64_t deps_readyAt; // Quando a última dependência vai ficar pronta


        uint64_t base_addr[4];
        uint32_t base_uop_id[4];
        uint64_t base_mem_addr[4]; // 0 -> Ld1; 1 -> Ld2; 2 -> Op [NULL]; 3 -> St
        uint32_t mem_size; // AVX-256 (32) or AVX-512 (64)


        bool vima_sent;
        bool CPU_requirements_meet;
        bool VIMA_requirements_meet;
        uint64_t VIMA_requirements_meet_readyAt;

        bool entry_can_be_removed; // Indicative that the operations related with this conversion are completed

        bool is_mov; // (is_mov) ? (ld1 -> st) : (Ld1 + Ld 2 -> Op -> St)

        bool first_conversion; // Indicates that this is the first try from a loop
                               // if it fails, the loop will enter in the black list

        trace_checkpoint_t checkpoint; /// Checkpoint para caso ocorra um flush
        uint64_t checkpoint_uop_number; /// Uop de checkpoint para o flush

    
        void package_clean() {
            this->unique_conversion_id = 0;
            this->conversion_started = false;
            this->conversion_beginning = 0;
            this->conversion_ending = 0;

            this->infos_remaining = 0;
            this->mem_addr_confirmations_remaining = 0;
            this->wait_reg_deps_number = 0;
            this->deps_readyAt = 0;

            for (uint32_t i=0; i<4; ++i) {
                this->base_addr[i] = 0x0;
                this->base_uop_id[i] = 0x0;
                this->base_mem_addr[i] = 0x0;
            }

            this->mem_size = 0;

            this->vima_sent = false;
            this->CPU_requirements_meet = false;
            this->VIMA_requirements_meet = false;
            this->VIMA_requirements_meet_readyAt = 0;

            this->entry_can_be_removed = false;

            this->is_mov = false;

            this->first_conversion = false;

            this->checkpoint.package_clean();
            this->checkpoint_uop_number = 0;

        }

        bool IsEqual(conversion_status_t *other) {
            if (this->unique_conversion_id == other->unique_conversion_id &&
                this->mem_size == other->mem_size &&
                this->is_mov == other->is_mov &&
                this->base_mem_addr[0] == other->base_mem_addr[0] &&
                this->base_mem_addr[1] == other->base_mem_addr[1] &&
                this->base_mem_addr[2] == other->base_mem_addr[2] &&
                this->base_mem_addr[3] == other->base_mem_addr[3]) 
                {
                    return true;
                } 
                printf("isEqual:\n");
                printf("ID: %lu vs ID %lu\n", this->unique_conversion_id, other->unique_conversion_id);
                printf("SIZE: %u vs SIZE %u\n", this->mem_size, other->mem_size);
                printf("Is_mov: %s vs Is_mov %s\n", this->is_mov ? "true" : "false", other->is_mov ? "true" : "false");
                printf("base_mem_addr[0]: %lu vs base_mem_addr[0] %lu\n", this->base_mem_addr[0], other->base_mem_addr[0]);
                printf("base_mem_addr[1]: %lu vs base_mem_addr[1] %lu\n", this->base_mem_addr[1], other->base_mem_addr[1]);
                printf("base_mem_addr[2]: %lu vs base_mem_addr[2] %lu\n", this->base_mem_addr[2], other->base_mem_addr[2]);
                printf("base_mem_addr[3]: %lu vs base_mem_addr[3] %lu\n", this->base_mem_addr[3], other->base_mem_addr[3]);
                exit(1);
                return false;
        }

};

class back_list_entry_t {
    public:
        uint64_t pc;
        uint32_t uop_id;

        back_list_entry_t () {
            pc = 0x0;
            uop_id = 0;
        }

        void package_clean() {
            pc = 0x0;
            uop_id = 0;
        }
};

class vima_converter_t {
    public:
        uint32_t iteration;
        // State machine
        // 0 -> Waiting first avx 256 load
        // 1 -> Waiting second avx 256 load
        // 2 -> Waiting operation
        // 3 -> Waiting avx 256 store
        uint32_t state_machine;


        // Instructions info
        // [Iter 0] Load 1 -> 0
        // [Iter 0] Load 2 -> 1
        // [Iter 0] Op     -> 2
        // [Iter 0] Store  -> 3
        // --------------------
        // [Iter 1] Load 1 -> 4
        // [Iter 1] Load 2 -> 5
        // [Iter 1] Op     -> 6
        // [Iter 1] Store  -> 7

        uint64_t addr[8];
        uint32_t uop_id[8];
        uint64_t mem_addr[8];
        uint64_t next_unique_conversion_id;
        conversion_status_t *current_conversion;

        bool regs_list[259];


        circular_buffer_t<conversion_status_t> current_conversions;

        // ***********
        // Black list
        // ***********
        circular_buffer_t<back_list_entry_t> conversions_blacklist;



        // ***********************
        // Launch vima instruction
        // ***********************
        circular_buffer_t<uop_package_t> vima_instructions_buffer;

        // *******************
        // VIMA configurations
        // *******************
        uint64_t VIMA_SIZE;
        uint32_t mem_operation_latency;
        uint32_t mem_operation_wait_next;
        functional_unit_t *mem_operation_fu;

        // **********
        // Prefetcher
        // **********
        uint64_t contiguous_conversions;
        vima_prefetcher_t *prefetcher;
        uint64_t CONTIGUOUS_CONVERSIONS_TO_PREFETCH;


        // **********
        // Statistics
        // **********
        uint64_t vima_instructions_launched;
        uint64_t placeholder_instructions_launched;

        uint64_t conversion_failed;
        uint64_t conversion_successful;
        uint64_t prefetched_vima_used;
        uint64_t prefetch_failed;
        std::unordered_map<std::string, uint64_t> invalidation_reason;
        uint64_t instructions_intercepted;
        uint64_t instructions_intercepted_until_commit;
        uint64_t instructions_reexecuted;
        uint64_t original_program_instructions;
        

        uint64_t conversion_entry_allocated;
        uint64_t not_enough_conversion_entries;


        uint64_t AGU_result_from_wrong_conversion;
        uint64_t AGU_result_from_current_conversion;


        std::unordered_map<uint32_t, uint64_t> CPU_to_VIMA_conversions;


        uint64_t conversions_blacklisted;
        uint64_t avoided_by_the_blacklist;

        uint64_t time_waiting_for_infos;
            uint64_t time_waiting_for_infos_start;
            uint64_t time_waiting_for_infos_stop;

        uint64_t VIMA_before_CPU;
        uint64_t CPU_before_VIMA;

        uint64_t blocked_until_conversion_complete;
        uint64_t unblocked_after_conversion_complete;

        uint64_t flushed_instructions; /// Múmero de instruções que sofreram flush por conta de uma invalidação
        

        vima_converter_t();

        void initialize(uint32_t mem_operation_latency, uint32_t mem_operation_wait_next, functional_unit_t *mem_operation_fu, uint32_t VIMA_SIZE,
                        uint32_t PREFETCH_BUFFER_SIZE, uint32_t CONTIGUOUS_CONVERSIONS_TO_PREFETCH, uint32_t PREFETCH_SIZE);

        uop_package_t create_placeholder_for_conversion(conversion_status_t *conversion_data);
    
        void generate_placeholder_instruction(conversion_status_t *conversion_data);
        void generate_VIMA_instruction(conversion_status_t *conversion_data);
        instruction_operation_t define_vima_operation(conversion_status_t *conversion_data);

        inline void start_new_conversion();
        void continue_conversion(conversion_status_t *prev_conversion); // After a successfull conversion tries to convert the following loop iterations

        // Clock -> Check for finished conversions
        void clock();

        // Conversion went wrong
        /* Para a conversão antes de ter lançado a instrução VIMA */
        void stop_conversion(conversion_status_t *conversion_data);

        /* Limpa a conversão depois de lançar a instrução VIMA (limpa o ROB também)*/
        void invalidate_conversion(conversion_status_t *invalidated_conversion);

        // Check if conversion finished
        void check_conversions();

        // Conversion address
        bool AGU_result(uop_package_t *); // True -> OK; False -> Invalidate
        bool get_index_for_alignment(conversion_status_t * current_conversion, uint32_t access_size, uint64_t base_store_address);
        void confirm_mem_addr(uop_package_t *); /* Versão simplificada do AGU result */

        // Completed execution
        void vima_execution_completed(memory_package_t *vima_package, uint64_t readyAt);

        // Black list
        bool is_blacklisted(uop_package_t *uop);

        // Statistics
        inline void statistics(FILE *);

        // Conversion stats for debug
        inline void conversionStats(uint64_t conversion_id);

        conversion_status_t *get_conversion(uint64_t conversion_unique_id);
};

inline void vima_converter_t::start_new_conversion() {
#if VIMA_CONVERSION_DEBUG == 1
    printf("***************************************************\n");
    printf("Resetting vima converter to start new conversion...\n");
    printf("***************************************************\n");

    printf("Trying to allocate a new converter entry...\n");
#endif
    conversion_status_t new_conversion;
    new_conversion.package_clean();
    new_conversion.first_conversion = true;
    int32_t entry = this->current_conversions.push_back(new_conversion);
    if (entry != -1) {
#if VIMA_CONVERSION_DEBUG == 1
        printf("Entry allocated!\n");
#endif
        this->conversion_entry_allocated++;
        this->current_conversion = &this->current_conversions[entry];

        this->iteration = 0;
        this->state_machine = 0;

        // Write register control
        for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;


        // Conversion data
        for (uint32_t i=0; i < 8; ++i) {
            this->addr[i] = 0;
            this->uop_id[i] = 0;
            this->mem_addr[i] = 0;
        }

#if VIMA_CONVERSION_DEBUG == 1
    printf("New conversion with conversion id: %lu\n", next_unique_conversion_id);
#endif
        this->current_conversion->unique_conversion_id = next_unique_conversion_id++;

        this->current_conversion->conversion_started = false;
        this->current_conversion->conversion_beginning = 0;
        this->current_conversion->conversion_ending = 0;

        this->current_conversion->infos_remaining = 0;
        this->current_conversion->mem_addr_confirmations_remaining = 0;
        this->current_conversion->wait_reg_deps_number = 0;
        this->current_conversion->deps_readyAt = 0;


        this->current_conversion->vima_sent = false;
        this->current_conversion->CPU_requirements_meet = false;
        this->current_conversion->VIMA_requirements_meet = false;
        this->current_conversion->VIMA_requirements_meet_readyAt = 0;

    }
    // *******************************
    // An entry could not be allocated
    // *******************************
    else {
#if VIMA_CONVERSION_DEBUG == 1
        printf("Entry could not be allocated!\n");
#endif
        if (this->current_conversion != 0x0) {
            ++this->not_enough_conversion_entries;
        }
        this->current_conversion = 0x0;

        this->iteration = UINT32_MAX;
        this->state_machine = UINT8_MAX;

        // Write register control
        for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;


        // Conversion data
        for (uint32_t i=0; i < 8; ++i) {
            this->addr[i] = 0;
            this->uop_id[i] = 0;
            this->mem_addr[i] = 0;
        }

    }

}




// Statistics
inline void vima_converter_t::statistics(FILE *output) {
    fprintf(output, "vima_instructions_launched: %lu\n", this->vima_instructions_launched);
    fprintf(output, "placeholder_instructions_launched: %lu\n", this->placeholder_instructions_launched);
    fprintf(output, "Conversions:\n");
    for (auto CPU_to_VIMA_conversion_type : CPU_to_VIMA_conversions) {
        fprintf(output, "%u: %lu\n", CPU_to_VIMA_conversion_type.first, CPU_to_VIMA_conversion_type.second);
    }
    fprintf(output, "conversion_failed: %lu\n", conversion_failed); 
    fprintf(output, "conversion_successful: %lu\n", conversion_successful); 
    fprintf(output, "prefetched_vima_used: %lu\n", prefetched_vima_used);     
    fprintf(output, "prefetch_failed: %lu\n", prefetch_failed);
    fprintf(output, "instructions_intercepted: %lu\n", instructions_intercepted); 
    fprintf(output, "instructions_intercepted_until_commit: %lu\n", instructions_intercepted_until_commit); 
    fprintf(output, "instructions_reexecuted: %lu\n", instructions_reexecuted); 
    fprintf(output, "original_program_instructions: %lu\n", original_program_instructions);
    fprintf(output, "conversion_entry_allocated: %lu\n", this->conversion_entry_allocated);
    fprintf(output, "not_enough_conversion_entries: %lu\n", this->not_enough_conversion_entries);
    fprintf(output, "AGU_result_from_current_conversion: %lu\n", AGU_result_from_current_conversion); 
    fprintf(output, "AGU_result_from_wrong_conversion: %lu\n", AGU_result_from_wrong_conversion);
    fprintf(output, "conversions_blacklisted: %lu\n", this->conversions_blacklisted);
    fprintf(output, "avoided_by_the_blacklist: %lu\n", this->avoided_by_the_blacklist);
    fprintf(output, "time_waiting_for_infos: %lu\n", time_waiting_for_infos);
    fprintf(output, "VIMA_before_CPU: %lu\n", VIMA_before_CPU);
    fprintf(output, "CPU_before_VIMA: %lu\n", CPU_before_VIMA);
    fprintf(output, "Blocked until conversion complete: %lu\n", this->blocked_until_conversion_complete);
    fprintf(output, "Unblocked after conversion complete: %lu\n", this->unblocked_after_conversion_complete);
    fprintf(output, "Flushed instructions: %lu\n", this->flushed_instructions);
    
    
    fprintf(output, "Invalidations reasons:\n");
    for (auto invalidation_type : invalidation_reason) {
        fprintf(output, "%s: %lu\n", invalidation_type.first.c_str(), invalidation_type.second);
    }
    fprintf(output, "Invalidations reasons ended\n");
}

inline void vima_converter_t::conversionStats(uint64_t conversion_id) {
    uint32_t index = this->current_conversions.get_size();

    for (uint32_t i=0; i < this->current_conversions.get_size(); ++i) {
        if ((this->current_conversions[i].entry_can_be_removed == false) && (this->current_conversions[i].unique_conversion_id == conversion_id)) {
            index = i;
        }
    }

    if (index == this->current_conversions.get_size()) {
        printf("Conversão com id %lu não encontrada!\n", conversion_id);
    } else {
        if (this->current_conversions.get_size() > 0) {
            printf("Conversão com id %lu:\n", conversion_id);
            printf("    CPU_requirements_meet: %s\n    infos_remaining: %u\n    mem_addr_confirmations_remaining: %ld\n    wait_reg_deps_number: %d\n    deps_readyAt: %lu/%lu\n",
                this->current_conversions[index].CPU_requirements_meet ? "true" : "false",
                this->current_conversions[index].infos_remaining,
                this->current_conversions[index].mem_addr_confirmations_remaining,
                this->current_conversions[index].wait_reg_deps_number,
                this->current_conversions[index].deps_readyAt,
                orcs_engine.get_global_cycle());
    }
    }
}

