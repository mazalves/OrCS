class directory_t {
    
    public:
        uint32_t n_sets;
        directory_set_t *sets;
        uint32_t associativity;
        uint32_t size;

        directory_t();
        ~directory_t();
        void allocate();
};