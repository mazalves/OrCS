#pragma once
#include "../simulator.hpp"

#ifndef _SET_ASSOCIATIVE_HPP_
#define _SET_ASSOCIATIVE_HPP_

template <class C_TYPE>
class set_associative_t {
    // Content
    C_TYPE **sets; 
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
        sets = new C_TYPE*[this->num_sets];
        for (uint32_t i=0; i < this->num_sets; ++i) {
            sets[i] = new C_TYPE[this->associativity];
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
