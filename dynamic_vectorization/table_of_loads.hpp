class table_of_loads_t {
    public:
        table_of_loads_entry_t *entries;
        int32_t entries_size;
        int32_t next_replacement;

        table_of_loads_entry_t *add_pc (uint64_t pc, uint64_t addr);
        table_of_loads_entry_t *find_pc (uint64_t pc);
        void update_stride (table_of_loads_entry_t *tl_entry, uint64_t addr);
        table_of_loads_t  (int32_t num_entries);
        ~table_of_loads_t ();

        void list_contents();
};