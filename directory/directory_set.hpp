class directory_set_t {

    public:
        uint32_t n_lines;
        directory_line_t **lines;

        directory_set_t () {
            this->n_lines = 0;
            this->lines = NULL;
        }

        ~directory_set_t () {
            delete[] this->lines;
        }
};