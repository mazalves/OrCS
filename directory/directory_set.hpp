class directory_set_t {

    public:
        uint32_t n_lines;
        directory_line_t **lines;

        directory_set_t () {
            this->n_lines = 0;
            this->lines = NULL;
        }

        ~directory_set_t () {
            for (uint32_t i = 0; i < this->n_lines; i++) {
                delete[] this->lines[i];
            }
            delete[] this->lines;
            // }
        }
};