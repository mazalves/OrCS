class directory_way_t {

    private:
        uint32_t POINTER_LEVELS;

    public:
        // uint32_t level;
        uint32_t shared;
        way_t *cache_way;
        uint32_t cache_status;

        directory_way_t() {
            this->clean_way();
        }

        ~directory_way_t()=default;

        void clean_way() {
            // this->level = 0;
            this->shared = 0;
            this->cache_way = NULL;
            this->cache_status = UNCACHED;
        }

        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};