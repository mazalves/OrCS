#include "../simulator.hpp"

class line_t;

class level_way_t {

public:
    uint32_t shared;
    line_t *cache_way;
    uint32_t cache_status;

    level_way_t() {
        this->clean_way();
    }

    ~level_way_t() = default;

    void clean_way() {
        this->shared = 0;
        this->cache_way = NULL;
        this->cache_status = UNCACHED;
    }

    // void allocate() {
    //     cache_way = new line_t;
    // }
};