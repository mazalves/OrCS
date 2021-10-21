class table_of_pre_vectorization_entry_t {
    private:
        uint64_t pc;
        uint8_t uop_id;
        bool free;
        // Data
        table_of_vectorizations_entry_t *tv_entry;

    public:
        inline bool is_equal (uint64_t addr, uint8_t uop_id);
        inline bool is_free();
        inline void set (uint64_t addr, uint8_t uop_id, table_of_vectorizations_entry_t *tv_entry);
        inline void clean ();
        inline void print ();
        inline table_of_vectorizations_entry_t* get_tv();

        table_of_pre_vectorization_entry_t () {
            this->pc = 0;
            this->uop_id = 0;
            this->free = true;
            this->tv_entry = NULL;
        }

};


    inline bool table_of_pre_vectorization_entry_t::is_equal (uint64_t addr, uint8_t uop_id) {
        if (this->free) return false;
        return ((addr == this->pc) && (uop_id == this->uop_id));
    }

    inline bool table_of_pre_vectorization_entry_t::is_free() {
        return this->free;
    }

    inline void table_of_pre_vectorization_entry_t::set (uint64_t addr, uint8_t uop_id, table_of_vectorizations_entry_t *tv_entry) {
        this->pc        = addr;
        this->uop_id    = uop_id;
        this->free      = false;
        this->tv_entry  = tv_entry;
    }

    inline void table_of_pre_vectorization_entry_t::clean () {
        this->pc = 0;
        this->uop_id = 0;
        this->free = true;
        this->tv_entry = NULL;
    }

    inline table_of_vectorizations_entry_t* table_of_pre_vectorization_entry_t::get_tv () {
        return this->tv_entry;
    }


    inline void table_of_pre_vectorization_entry_t::print () {
        printf("PC: %lu  uop_id: %u  tv_entry: %p  free: %s\n",
                this->pc, this->uop_id, (void *)this->tv_entry, this->free ? "true" : "false");
    }