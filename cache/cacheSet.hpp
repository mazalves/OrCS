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
};

#endif // CACHESET_H
