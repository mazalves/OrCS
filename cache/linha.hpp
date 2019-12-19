#include "../simulator.hpp"

#ifndef LINE_H
#define LINE_H

class line_t
{

private:
    uint32_t POINTER_LEVELS;

public:
    uint64_t address;
    uint64_t tag;
    uint32_t dirty;
    uint64_t lru;
    uint32_t prefetched;
    uint32_t valid;
    uint64_t ready_at;
    directory_way_t *directory_line;
    line_t ***line_ptr_caches;

    uint32_t NUMBER_OF_PROCESSORS;
    INSTANTIATE_GET_SET_ADD(uint32_t, NUMBER_OF_PROCESSORS)

    line_t()
    {
        this->clean_line();
        // TODO deveria ser feito no allocate?!
        libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
        set_NUMBER_OF_PROCESSORS(cfg_root["PROCESSOR"].getLength());
    }

    ~line_t()
    {
        for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
        {
            for (uint32_t j = 0; j < POINTER_LEVELS; j++)
            {
                //free (line_ptr_caches[i][j]);
            }
            delete[] line_ptr_caches[i];
        }
        delete[] line_ptr_caches;
        //delete[] directory_line;
    }

    void allocate(uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS)
    {
        set_POINTER_LEVELS((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
        this->line_ptr_caches = new line_t **[NUMBER_OF_PROCESSORS];
        for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++)
        {
            this->line_ptr_caches[i] = new line_t *[POINTER_LEVELS];
            for (uint32_t j = 0; j < POINTER_LEVELS; j++)
            {
                this->line_ptr_caches[i][j] = NULL;
            }
        }
        this->directory_line = new directory_way_t;
        this->directory_line->allocate(NUMBER_OF_PROCESSORS, INSTRUCTION_LEVELS, DATA_LEVELS);
    }

    void clean_line()
    {
        this->address = 0;
        this->tag = 0;
        this->dirty = 0;
        this->lru = 0;
        this->prefetched = 0;
        this->valid = 0;
        this->ready_at = 0;
    }

    INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};

#endif // LINE_H
