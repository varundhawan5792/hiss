/*
 *	(C) 2011 Varun Mittal <varunmittal91@gmail.com> & Varun Dhawan <varundhawan5792@gmail.com>
 *	NeweraHPC program is distributed under the terms of the GNU General Public License v2
 *
 *	This file is part of NeweraHPC.
 *
 *	NeweraHPC is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation version 2 of the License.
 *
 *	NeweraHPC is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with NeweraHPC.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GRID_INSTRUCTION_SET_H_
#define _GRID_INSTRUCTION_SET_H_

#include "grid_data.h"

namespace neweraHPC
{
   nhpc_status_t nhpc_create_instruction(nhpc_instruction_set_t **instruct_set, const char *plugin_name,
					 const char *host_grid_uid = NULL);   
   nhpc_status_t nhpc_generate_instruction(nhpc_instruction_set_t **instruction_set, rbtree_t *headers);
   nhpc_status_t nhpc_generate_general_instruction(nhpc_instruction_set_t **instruction_set,
						   rbtree_t *headers);
   nhpc_status_t nhpc_delete_instruction(nhpc_instruction_set_t *instruct_set);
   nhpc_status_t nhpc_add_argument(nhpc_instruction_set_t *instruction, enum GRID_ARG_TYPE option, 
				   const void *arg1, const void *arg2 = NULL);
}

#endif
