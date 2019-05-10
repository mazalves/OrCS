#ifndef LINHA_H
#define LINHA_H


class linha_t
{
    public:
        uint64_t tag;
        uint32_t dirty;
        uint64_t lru;
        uint32_t prefetched;
        uint32_t valid;
        uint64_t readyAt;
        linha_t* linha_ptr_l1;
        linha_t* linha_ptr_l2;
        linha_t* linha_ptr_llc;
        linha_t(){
            this->clean_line();
        }
        ~linha_t(){
            // deleting pointes
            if(this->linha_ptr_l1 != NULL) delete &linha_ptr_l1;
            if(this->linha_ptr_l2 != NULL) delete &linha_ptr_l2;
            if(this->linha_ptr_llc != NULL) delete &linha_ptr_llc;
            // Nulling pointers
            this->linha_ptr_l1 = NULL;
            this->linha_ptr_l2 = NULL;
            this->linha_ptr_llc = NULL;
        }
        void clean_line(){
            this->tag = 0;
            this->dirty = 0;
            this->lru = 0;
            this->prefetched = 0;
            this->valid = 0;
            this->readyAt = 0;
            this->linha_ptr_l1 = NULL;
            this->linha_ptr_l2 = NULL;
            this->linha_ptr_llc = NULL;
        }
        std::string content_to_string(){
            std::string content_string;
            content_string = "";
            
            content_string = content_string + " Valid:" + utils_t::uint32_to_string(this->valid);
            content_string = content_string + " TAG:" + utils_t::uint64_to_string(this->tag);
            content_string = content_string + " Prefetched:" + utils_t::uint32_to_string(this->prefetched);
            content_string = content_string + " LRU" + utils_t::big_uint64_to_string(this->lru);
            content_string = content_string + " readyAt" + utils_t::big_uint64_to_string(this->readyAt);

            return content_string;
        }
};

#endif // LINHA_H
