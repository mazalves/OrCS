class table_of_stores_entry_t {
    private:
        uint64_t pc;
        uint8_t uop_id;

    public:
        uint64_t last_address;
        uint32_t access_size;
        int32_t stride;
        uint8_t confidence_counter;
        bool is_mov;
        uint8_t st_confidence;
        table_of_vectorizations_entry_t *tv_entry;

        uint32_t tl_to_entry;
        bool linked_tl_to;

        // Alocação
        uint64_t timestamp;
        uint64_t lru;
        bool free;

        // Impede vetorizações da entrada
        bool locked;


        // Indica que é vetorizável (confidence no máximo e stride com boa distância)
        // [dados contíguos e com tamanho 32, 64 bits ou 32B (vetor)]
        bool vectorizable;

        ////////////////////////
        // Methods and functions
        ////////////////////////

        table_of_stores_entry_t () {
            this->pc = 0;
            this->uop_id = 0;

            this->last_address = 0;
            this->access_size = 0;
            this->stride = 0;
            this->confidence_counter = 0;
            this->is_mov = false;
            this->st_confidence = 0;
            this->tv_entry = NULL;

            this->tl_to_entry = 0;
            this->linked_tl_to = false;

            this->timestamp = 0;
            this->lru = 0;          
            this->free = true;
            this->vectorizable = false;

            this->locked = false;
        }

        inline void clean() {
            this->pc = 0;
            this->uop_id = 0;

            this->last_address = 0;
            this->access_size = 0;
            this->stride = 0;
            this->confidence_counter = 0;
            this->is_mov = false;
            this->tv_entry = NULL;

            this->tl_to_entry = 0;
            this->linked_tl_to = false;

            this->timestamp = 0;
            this->lru = 0;
            this->free = true;   
            this->vectorizable = false;

            this->locked = false;
        }


        void allocate(libconfig::Setting &vectorizer_configs) {
            assert((int32_t)vectorizer_configs["ST_CONFIDENCE"] < 256); // uint8_t
            this->st_confidence = (int32_t)vectorizer_configs["ST_CONFIDENCE"];
            this->pc = 0;
            this->uop_id = 0;
            this->last_address = 0;
            this->stride = 0;
            this->confidence_counter = 0;
            this->is_mov = false;
            this->tv_entry = NULL;

            this->tl_to_entry = 0;
            this->linked_tl_to = false;

            this->timestamp = 0;
            this->lru = 0;
            this->free = true;

            this->vectorizable = false;

            this->locked = false;

        }

        inline void fill_entry( uint64_t pc, uint8_t uop_id,
                                uint64_t last_address,
                                uint32_t access_size,
                                int32_t stride,
                                uint8_t confidence_counter,
                                bool is_mov,
                                table_of_vectorizations_entry_t* tv_entry,
                                uint32_t tl_to_entry,
                                bool linked_tl_to,
                                uint64_t lru,
                                bool vectorizable) {
            this->pc = pc;
            this->uop_id = uop_id;
            this->last_address = last_address;
            this->access_size = access_size;
            this->stride = stride;
            this->confidence_counter = confidence_counter;
            this->is_mov = is_mov;
            this->tv_entry = tv_entry;
            this->tl_to_entry = tl_to_entry;
            this->linked_tl_to = linked_tl_to;
            this->timestamp = orcs_engine.get_global_cycle(); //id único da entrada
            this->lru = lru;
            this->free = false;
            this->vectorizable = vectorizable;

        }

        inline void inc_confidence_counter(uint64_t lru) {
            if (this->confidence_counter < this->st_confidence) {
                this->confidence_counter++;
            }
            this->lru = lru;
        }

        inline void reset_confidence_counter(uint64_t lru) {
            this->confidence_counter = 0;
            this->lru = lru;

        }

        inline void check_vectorizable(uop_package_t *uop) {
            //printf(" STORE (locked:%s) Vectorizable => Confidence %d/%d; mem_size: %u/4 || 8 || 32; 0 < stride <= mem_size: 0 < %d <= %u\n",
            //            this->locked ? "true" : "false", this->confidence_counter, this->st_confidence,
            //            uop->memory_size[0], this->stride, (int32_t)uop->memory_size[0]);
            if ((this->locked == false) &&
                (this->confidence_counter == this->st_confidence) &&
                (uop->memory_size[0] == 4 || uop->memory_size[0] == 8 || uop->memory_size[0] == 32) && //32 ou 64 bits ou vetor
                (this->stride > 0) &&
                (this->stride <= (int32_t)uop->memory_size[0]) // Contíguo
                ) {
                    this->vectorizable = true;
                } else {
                    this->vectorizable = false;
                }
        }

        inline bool is_entry(uop_package_t *uop) {
            if ((this->pc == uop->opcode_address) && 
                (this->uop_id == uop->uop_id) &&
                (this->free == false)) 
                {
                    return true;
                }
            return false;
        }

        inline uint64_t get_pc() {
            return this->pc;
        }

        inline uint64_t get_uop_id() {
            return this->uop_id;
        }

         // Impede novas vetorizações do mesmo tipo
        inline void lock() {
            this->is_mov = false;
            this->tv_entry = NULL;

            this->tl_to_entry = 0;
            this->linked_tl_to = false;

            this->vectorizable = false;

            this->locked = true;
        }

        void print () {
            printf("  %d %lu (%lu) Free:%s Stride: %d Confidence: %d/%d is_mov: %d tl_to_entry: %d (V: %d) tv_entry: %p Timestamp: %lu lru: %lu Vec: %s\n",
            this->uop_id,
            this->pc,
            this->last_address,
            (this->free) ? "true" : "false",
            this->stride,
            this->confidence_counter,
            this->st_confidence,
            this->is_mov,
            this->tl_to_entry,
            this->linked_tl_to,
            (void *)this->tv_entry,
            this->timestamp,
            this->lru,
            this->vectorizable ? "true" : "false");
        }
            

};