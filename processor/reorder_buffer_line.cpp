/*
 * Copyright (C) 2010~2014  Marco Antonio Zanata Alves
 *                          (mazalves at inf.ufrgs.br)
 *                          GPPD - Parallel and Distributed Processing Group
 *                          Universidade Federal do Rio Grande do Sul
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../simulator.hpp"
#include <string>

// ============================================================================
reorder_buffer_line_t::reorder_buffer_line_t() {
    this->package_clean();
    this->reg_deps_ptr_array = NULL;
}

// ============================================================================
reorder_buffer_line_t::~reorder_buffer_line_t() {
    utils_t::template_delete_array<reorder_buffer_line_t*>(reg_deps_ptr_array);
}

// ============================================================================
void reorder_buffer_line_t::package_clean() {
    this->uop.package_clean(); 
    this->stage = PROCESSOR_STAGE_DECODE;
    this->mob_ptr = NULL;
    this->wait_reg_deps_number = 0; 
    this->wake_up_elements_counter = 0;
    this->sent=false;
    this->processor_id=0;
    }

// ============================================================================
std::string reorder_buffer_line_t::content_to_string() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->uop.status == PACKAGE_STATE_FREE) {
            return content_string;
        }
    #endif

    content_string = this->uop.content_to_string();
    content_string = content_string + " | Stage:" + get_enum_processor_stage_char(this->stage);
    content_string = content_string + " | Reg.Wait:" + utils_t::uint32_to_string(this->wait_reg_deps_number);
    content_string = content_string + " | WakeUp:" + utils_t::uint32_to_string(this->wake_up_elements_counter);
    content_string = content_string + " | ReadyAt: " + utils_t::uint64_to_string(this->uop.readyAt);
    content_string = content_string + " | Sent: " + utils_t::bool_to_string(this->sent);
    if(this->mob_ptr != NULL){
        content_string = content_string + this->mob_ptr->content_to_string();
    }
    return content_string;
}
std::string reorder_buffer_line_t::content_to_string2() {
    std::string content_string;
    content_string = "";

    #ifndef SHOW_FREE_PACKAGE
        if (this->uop.status == PACKAGE_STATE_FREE) {
            return content_string;
        }
    #endif

    content_string = this->uop.content_to_string();
    content_string = content_string + " | Stage:" + get_enum_processor_stage_char(this->stage);
    content_string = content_string + " | Reg.Wait:" + utils_t::uint32_to_string(this->wait_reg_deps_number);
    content_string = content_string + " | WakeUp:" + utils_t::uint32_to_string(this->wake_up_elements_counter);
    content_string = content_string + " | ReadyAt: " + utils_t::uint64_to_string(this->uop.readyAt);
    content_string = content_string + " | Sent: " + utils_t::bool_to_string(this->sent);
    return content_string;
}
// ============================================================================
/// STATIC METHODS
// ============================================================================

std::string reorder_buffer_line_t::print_all(reorder_buffer_line_t *input_array, uint32_t size_array) {
    std::string content_string;
    std::string final_string;

    final_string = "";
    for (uint32_t i = 0; i < size_array ; i++) {
        content_string = "";
        content_string = input_array[i].content_to_string();
        if (content_string.size() > 1) {
            final_string = final_string + "[" + utils_t::uint32_to_string(i) + "] " + content_string + "\n";
        }
    }
    return final_string;
}