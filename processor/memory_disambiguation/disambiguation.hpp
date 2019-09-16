class desambiguation_t {
    public:
        virtual ~desambiguation_t() {};
        virtual void allocate(){};
        virtual void make_memory_dependences(memory_order_buffer_line_t*){};
        virtual void solve_memory_dependences(memory_order_buffer_line_t*){};
        virtual void statistics(){};
    
};

