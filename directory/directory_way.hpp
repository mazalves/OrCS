class directory_way_t {

    public:
        level_way_t *cache_level;
        uint64_t tag;
        cacheId_t id;

        directory_way_t() {
            this->cache_level = NULL;
            this->tag = 0;
            this->id = NAC;
        }

        ~directory_way_t()=default;

        void allocate(uint32_t POINTER_LEVELS) {
            this->cache_level = new level_way_t[POINTER_LEVELS];
            for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
                this->cache_level[i].clean_way();
            }
        }

};