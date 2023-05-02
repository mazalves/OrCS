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

class reorder_buffer_line_t {
    public:
        // ==============================================
        uint32_t processor_id;
        uop_package_t uop; //*                         /// uOP stored
        processor_stage_t stage; //*                   /// Stage of the uOP
        /// Register Dependencies Control
        uint32_t wait_reg_deps_number;                  /// Must wait BEFORE execution
        reorder_buffer_line_t* *reg_deps_ptr_array;     /// Elements to wake-up AFTER execution
        uint32_t wake_up_elements_counter;              /// Counter of elements to wakeup
        conversion_status_t* *reg_deps_conv_ptr_array;     /// Conversions to wake-up AFTER execution
        uint32_t wake_up_conversions_counter;              /// Counter of conversions to wakeup
        memory_order_buffer_line_t* mob_base;            /// mob pointer to memory request 
        int32_t pos_mob;
        uint32_t mob_limit;
        uint32_t waiting_mem_request; // Requisições à memória que devem confirmar antes do commit
        
        bool sent;                                      /// Control flag to remove robs entry
        bool committed;
        // ====================================================================
        /// Methods
        // ====================================================================
        reorder_buffer_line_t();
        ~reorder_buffer_line_t();
        void package_clean();
        std::string content_to_string();
        std::string content_to_string2();
        
        static std::string print_all(reorder_buffer_line_t *input_array, uint32_t size_array);
};
