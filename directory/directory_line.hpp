class directory_line_t {

    private:
        uint32_t POINTER_LEVELS;

    public:
        uint32_t id;
        uint64_t tag; 
        uint32_t level;
        uint32_t shared;
        line_t *cache_line;
        uint32_t cache_status;

        directory_line_t() {
            this->clean_line();
        }

        ~directory_line_t(){
            free (cache_line);
        }

        void clean_line() {
            this->id = 0;
            this->tag = 0;
            this->level = 0;
            this->shared = 0;
            this->cache_line = NULL;
            this->cache_status = UNCACHED;
        }

        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};