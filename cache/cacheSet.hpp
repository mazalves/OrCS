#ifndef CACHESET_H
#define CACHESET_H

class cacheSet_t {
    public:
        uint32_t n_ways;
        way_t *ways;

        // Constructor
        cacheSet_t() {
            this->n_ways = 0;
            this->ways = NULL;
        }

        // Desctructor
        ~cacheSet_t() {
            //delete[] ways;
        }
};

#endif // CACHESET_H
