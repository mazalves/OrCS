class desambiguation_t
{
public:
    desambiguation_t();
    ~desambiguation_t();
    void allocate();
    void make_memory_dependences(memory_order_buffer_line_t *mob_line);
    void solve_memory_dependences(memory_order_buffer_line_t *mob_line);
    void statistics();
private:

    #if PERFECT
        disambiguation_perfect_t *disambiguator;
    #endif
    //#if HASHED
        disambiguation_hashed_t *disambiguator;
    //#endif

    
};

