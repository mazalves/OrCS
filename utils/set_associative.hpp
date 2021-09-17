#pragma once
#include "../simulator.hpp"

#ifndef _SET_ASSOCIATIVE_HPP_
#define _SET_ASSOCIATIVE_HPP_

template <class C_TYPE>
class set_associative_t {

    class set_t {
        C_TYPE   *contents;
        uint64_t *ids;
        uint64_t *lru_counters;
        bool     *valid;
        uint32_t associativity;

        set_t() {
            contents        = NULL;
            ids             = NULL;
            lru_counters    = NULL;
            valid           = NULL;
        }

        void allocate (uint32_t associativity) {
            contents        = new C_TYPE  [associativity];
            ids             = new uint64_t[associativity];
            lru_counters    = new uint64_t[associativity];
            valid           = new bool    [associativity];
            this->associativity = associativity;
        }

        inline C_TYPE* operator[](uint32_t index) {
            assert(index < associativity);
            return this->contents[index];
        }

        inline uint32_t insert (uint64_t id, C_TYPE *content) {
            // Get LRU
            uint32_t pos = this->get_lru();

            // Insert
            contents    [pos]       = *content;
            ids         [pos]       = id;
            lru_counters[pos]       = orcs_engine.get_global_cycle();
            valid       [pos]       = true;

            return pos;
        }

        inline uint32_t insert_or_replace (uint64_t id, C_TYPE *content) {
            // Find
            bool exist = false;
            uint32_t pos = this->get_position(id, &exist);

            if (exist) {
                // Replace
                contents    [pos]       = *content;
                lru_counters[pos]       = orcs_engine.get_global_cycle();
                valid       [pos]       = true;
                return pos;
            } else {
                // Insert
                return this->insert(id, content);
            }

        }

        inline void clean (uint64_t id) {
            for (uint32_t i = 0; i < this->associativity; ++i) {
                if (ids[i] == id) {
                    ids[i] = 0;
                    lru_counters[i] = 0;
                    valid[i] = false;
                }
            }
        }

        inline uint32_t get_position (uint64_t id, bool *found) {
            for (uint32_t i = 0; i < this->associativity; ++i) {
                if ((ids[i] == id) && (valid[i] == true)) {
                    *found = true;
                    return i;
                }
            }
            *found = false;
            return 0;
        }

        inline C_TYPE get (uint64_t id, bool *found) {
            for (uint32_t i = 0; i < this->associativity; ++i) {
                if ((ids[i] == id) && (valid[i] == true)) {
                    *found = true;
                    return contents[i];
                }
            }
            *found = false;
            return contents[0];
        }

        inline uint32_t get_lru() {
            uint32_t oldest = 0;
            for (uint32_t i=1; i < this->associativity; ++i) {
                if (lru_counters[i] < lru_counters[oldest]) {
                    oldest = i;
                }
            }
            return oldest;
        }
    };

    // Content
    set_t *sets;
    uint32_t num_sets;
    uint32_t associativity;

    // Masks
    uint64_t set_mask;
    uint32_t mask_shift;
    
    public:


        /// Copy Assignment Operator
        set_associative_t& operator=(const set_associative_t& other)
        {
            ERROR_PRINTF("Avoid copying this structure for performance reasons.\n")
            return *this;
        }

    void allocate (uint32_t elements, uint32_t associativity, uint32_t mask_shift);
    inline C_TYPE* operator[](uint32_t index);
    inline C_TYPE* map_set(uint64_t value);
    inline uint32_t get_num_sets();
    inline uint32_t get_associativity();
    inline void insert (uint64_t id, C_TYPE *element);
    inline void  clean (uint64_t id);
    inline C_TYPE  get (uint64_t id);


    set_associative_t();
    ~set_associative_t();

};

    template <class C_TYPE>
    void set_associative_t<C_TYPE>::allocate(uint32_t elements, uint32_t associativity, uint32_t mask_shift) {
        uint32_t totalElements = (elements % associativity) + elements;

        // Set locals
        this->num_sets = totalElements / associativity;
        this->associativity = associativity;
        this->mask_shift = mask_shift;
        assert((this->num_sets & (this->num_sets - 1)) == 0); // Power of 2
        
        // Define mask
        uint32_t mask_bits = ceil(log2((double)this->num_sets));
        this->set_mask = 0;
        for (uint32_t i=0; i < mask_bits; ++i) this->set_mask |= (1 << i);

        // Allocation
        sets = new set_t[this->num_sets];
        for (uint32_t i = 0; i < this->num_sets; ++i) {
            sets[i].allocate(this->associativity);
        }
        
    }

    template <class C_TYPE>
    inline C_TYPE* set_associative_t<C_TYPE>::operator[](uint32_t index) {
        assert(index < this->num_sets);
        return this->sets[index];
    }

    template <class C_TYPE>
    inline uint32_t set_associative_t<C_TYPE>::get_num_sets() {
        return this->num_sets;
    }

    template <class C_TYPE>
    inline uint32_t set_associative_t<C_TYPE>::get_associativity() {
        return this->associativity;
    }

    template <class C_TYPE>
    inline void set_associative_t<C_TYPE>::insert (uint64_t id, C_TYPE *element) {
        // Insert on set
        this->sets[this->map_set(id)].insert(id, element);
    }

    template <class C_TYPE>
    inline void set_associative_t<C_TYPE>::clean (uint64_t id) {
        // Clear from set
        this->sets[this->map_set(id)].clean(id);
    }

    template <class C_TYPE>
    inline C_TYPE set_associative_t<C_TYPE>::get (uint64_t id) {
        return this->sets[this->map_set(id)].get(id);
    }

    template <class C_TYPE>
    inline C_TYPE* set_associative_t<C_TYPE>::map_set(uint64_t value) {
        return this->sets[(value >> mask_shift) & set_mask];
    }

    template <class C_TYPE>
    set_associative_t<C_TYPE>::set_associative_t(){
        // Content
        sets          = NULL; 
        num_sets      = 0;
        associativity = 0;

        // Masks
        set_mask      = 0x0;
        mask_shift    = 0;
    }

    template <class C_TYPE>
    set_associative_t<C_TYPE>::~set_associative_t() {
        for (uint32_t i=0; i < this->num_sets; ++i) {
            delete[] this->sets[i];
        }
        delete[] this->sets;
    }

#endif
