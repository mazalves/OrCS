class directory_t {
    
    public:
        uint32_t n_sets;
        directory_set_t *sets;

        directory_t();
        ~directory_t();
        void allocate(cache_t llc, uint32_t POINTER_LEVELS);
        void setPointers(line_t ***cache_lines, uint32_t n_processors, uint32_t cache_levels, uint32_t idx, int32_t line, memory_operation_t mem_op, uint64_t *cache_tags);

};