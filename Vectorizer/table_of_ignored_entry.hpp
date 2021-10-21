class table_of_ignored_entry_t {
    private:
        uint64_t pc;
        uint8_t uop_id;
        bool free;
        table_of_vectorizations_entry_t *tv_pointer;
        uint8_t structural_id; // 0 -> ld 1; 1 -> ld 2; 2 -> op; 3 -> st
                               // Indentificação para máscara na tl_entry (caso invalide sabe o que reexecutar)

        bool training; // Substituir por TPV //TODO
    public:
        inline bool is_equal (uint64_t addr, uint8_t uop_id);
        inline bool is_free();
        inline void set (uint64_t addr, uint8_t uop_id, table_of_vectorizations_entry_t *tv_entry, uint8_t structural_id);
        inline void clean ();
        inline void print ();
        inline table_of_vectorizations_entry_t* get_tv_entry (uint8_t *structural_id);


        table_of_ignored_entry_t () {
            this->pc = 0;
            this->uop_id = 0;
            this->free = true;
            this->tv_pointer = NULL;
            this->structural_id = 4;
        }

};


    inline bool table_of_ignored_entry_t::is_equal (uint64_t addr, uint8_t uop_id) {
        if (this->free) return false;
        return ((addr == this->pc) && (uop_id == this->uop_id));
    }

    inline bool table_of_ignored_entry_t::is_free() {
        return this->free;
    }

    inline void table_of_ignored_entry_t::set (uint64_t addr, uint8_t uop_id, table_of_vectorizations_entry_t *tv_entry, uint8_t structural_id) {
        this->pc        = addr;
        this->uop_id    = uop_id;
        this->free      = false;
        this->tv_pointer = tv_entry;
        this->structural_id = structural_id;
    }

    inline void table_of_ignored_entry_t::clean () {
        this->pc = 0;
        this->uop_id = 0;
        this->free = true;
        this->tv_pointer = NULL;
        this->structural_id = 4;
    }

    inline table_of_vectorizations_entry_t *table_of_ignored_entry_t::get_tv_entry (uint8_t *structural_id) {
        *structural_id = this->structural_id;
        return this->tv_pointer;
    }


    inline void table_of_ignored_entry_t::print () {
        printf("PC: %lu  uop_id: %u  structural_id: %u  tv_entry: %p  free: %s\n",
                this->pc, this->uop_id, this->structural_id, (void *)this->tv_pointer, this->free ? "true" : "false");
    }