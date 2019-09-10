class desambiguation_t {
    public:
        virtual ~desambiguation_t() {};
        virtual void allocate(){};
        virtual void make_memory_dependences(memory_order_buffer_line_t *mob_line){
            std::ignore = mob_line;
        };
        virtual void solve_memory_dependences(memory_order_buffer_line_t *mob_line){
            std::ignore = mob_line;
        };
        virtual void statistics(){};
    
};

