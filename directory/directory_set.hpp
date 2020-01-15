class directory_set_t
{

public:
    uint32_t n_ways;
    directory_way_t *ways;

    directory_set_t()
    {
        this->n_ways = 0;
        this->ways = NULL;
    }

    ~directory_set_t()
    {
        // delete[] this->ways;
    }

    void allocate(uint32_t associativity, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS, uint32_t *ICACHE_AMOUNT, uint32_t *DCACHE_AMOUNT)
    {
        this->n_ways = associativity;
        this->ways = new directory_way_t[this->n_ways];
        for (uint32_t i = 0; i < this->n_ways; i++)
        {
            this->ways[i].allocate(INSTRUCTION_LEVELS, DATA_LEVELS, ICACHE_AMOUNT, DCACHE_AMOUNT);
        }
    }
};