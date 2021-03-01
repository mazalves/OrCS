class table_of_loads_t {
    public:
        set_associative_t<table_of_loads_entry_t> entries;

        table_of_loads_entry_t *add_pc (uint64_t pc, uint64_t addr);
        table_of_loads_entry_t *find_pc (uint64_t pc);
        void update_stride (table_of_loads_entry_t *tl_entry, uint64_t addr);
        table_of_loads_t (uint32_t num_entries, uint32_t associativity);
        ~table_of_loads_t ();

        void list_contents();
};