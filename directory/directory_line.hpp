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
            this->clean_line();
        }

        ~directory_line_t() {
            delete[] cache_lines;
        }

        void clean_line() {
            this->cache_lines = NULL;
            this->shared = 0;
            this->status = INVALIDO;
            this->level = 0;
            this->id = 0;
        }

        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};