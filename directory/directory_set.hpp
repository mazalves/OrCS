class directory_set_t {

    public:
        uint64_t *tag;
        cacheId_t *id;
        uint32_t n_ways;
        directory_way_t **ways;

        directory_set_t () {
            this->n_ways = 0;
            this->ways = NULL;
            for (uint32_t i = 0; i < this->n_ways; i++) {
                this->tag[i] = 0;
                this->id[i] = NAC;
            }
        }

        ~directory_set_t () {
            delete[] this->ways;
        }
};