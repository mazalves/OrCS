class directory_t {
    
    public:
        uint32_t n_sets;
        directory_set_t *sets;

        directory_t();
        ~directory_t();
        void allocate(cache_t llc);
};