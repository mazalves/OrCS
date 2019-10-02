class directory_line_t {

    private:
        uint32_t POINTER_LEVELS;

    public:
        line_t *cache_lines;
        uint32_t shared;
        uint32_t status;
        uint32_t level;
        uint32_t id;

        directory_line_t() {
            this->cache_lines = NULL;
            this->shared = 0;
            this->status = INVALIDO;
            this->level = 0;
            this->id = 0;
        }

        ~directory_line_t() {
            delete[] cache_lines;
        }

        // void allocate(uint32_t n_caches) {
        //     this->cache_lines = new line_t[n_caches];
        // }

        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};