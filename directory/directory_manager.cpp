// #include "../simulator.hpp"

// directory_manager_t::directory_manager_t() {}

// directory_manager_t::~directory_manager_t() {}

// // TODO: descobrir quantas LLC existem na arquitetura e alocar esse numero de diretórios
// //       fazer uma lista de LLCs para guardar as associatividades e tamanhos ou usar do parametro
// void directory_manager_t::allocate(uint32_t NUMBER_OF_PROCESSORS) {
//     printf("%s\n", "directory_manager_t allocate");
//     set_LLC_CACHES(NUMBER_OF_PROCESSORS);
//     uint32_t ASSOCIATIVITY[1] = {20};
//     uint32_t SIZE[1] = {4194304};
//     uint32_t LINE_SIZE = 64;

//     this->directory = new directory_t[LLC_CACHES];
//     for (uint32_t i = 0; i < LLC_CACHES; i++) {
//         this->directory[i].associativity = ASSOCIATIVITY[i];
//         this->directory[i].size = SIZE[i];
//         this->directory[i].n_sets = ((SIZE[i]/LINE_SIZE)/ASSOCIATIVITY[i]);
//         this->directory[i].allocate();
//     }
// }