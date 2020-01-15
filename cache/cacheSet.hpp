#ifndef CACHESET_H
#define CACHESET_H

class cacheSet_t
{
public:
    uint32_t n_lines;
    line_t *lines;

    // Constructor
    cacheSet_t()
    {
        this->clean_way();
    }

    // Desctructor
    ~cacheSet_t()
    {
        //delete[] lines;
    }

    void allocate(uint32_t associativity, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS, uint32_t *ICACHE_AMOUNT, uint32_t *DCACHE_AMOUNT)
    {
        this->lines = new line_t[associativity];
        this->n_lines = associativity;
        for (uint32_t i = 0; i < this->n_lines; i++)
        {
            this->lines[i].allocate(INSTRUCTION_LEVELS, DATA_LEVELS, ICACHE_AMOUNT, DCACHE_AMOUNT);
            this->lines[i].clean_line();
        }
    }

    void clean_way()
    {
        this->n_lines = 0;
        this->lines = NULL;
    }
};

#endif // CACHESET_H
