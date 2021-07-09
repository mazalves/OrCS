class VR_entry_state_t {
    public:
        uint32_t F;
        uint32_t R; // Armazenam o número de uops restantes em cada caso
        uint32_t U;
        uint32_t V;

        bool sent; // Indica se uma validação já foi decodificada
        bool executed; // Indica se uma parte vetorial passou pelo decode
                        // porque só nesse estágio R é incrementado
        bool free; // Indica se um free já foi decodificado

    VR_entry_state_t () {
        F = 0;
        R = 0;
        U = 0;
        V = 0;
        
        sent = false;
        executed = false;
        free = false;
    }

};