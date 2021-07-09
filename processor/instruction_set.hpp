struct uop_info_t {
    uint32_t latency;
    uint32_t fu_id;
};

struct fu_info_t {
    uint32_t size;
    uint32_t wait_next;
};

class instruction_set_t {
public:
    std::vector<uop_info_t> uops;
    std::vector<fu_info_t> functional_units;

    std::vector<std::vector<uint32_t>> uops_per_instruction;

    std::unordered_map<std::string, uint32_t> fu_id;
    std::unordered_map<std::string, uint32_t> uops_id;
    std::unordered_map<std::string, uint32_t> instructions_id;


    instruction_set_t();
    ~instruction_set_t();
    void allocate();
};
