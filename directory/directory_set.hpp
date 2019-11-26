class directory_set_t {

    public:
        uint32_t n_ways;
        directory_way_t **ways;

        directory_set_t () {
            this->n_ways = 0;
            this->ways = NULL;
        }

        ~directory_set_t () {
            delete[] this->ways;
        }
};