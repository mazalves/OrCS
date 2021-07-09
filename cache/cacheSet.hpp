#ifndef CACHESET_H
#define CACHESET_H

class cacheSet_t {
    public:
        uint32_t n_lines;
        line_t *lines;

        // Constructor
        cacheSet_t() {
            this->n_lines = 0;
            this->lines = NULL;
        }

        // Desctructor
        ~cacheSet_t() {
            delete[] lines;
        }

        void print_lines() {
            for (uint32_t i = 0; i < n_lines; ++i) {
                lines[i].print_line();
            }
        }
};

#endif // CACHESET_H
