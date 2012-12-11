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

#ifndef _GRID_CLIENT_H_
#define _GRID_CLIENT_H_

#include "grid_data.h"

namespace neweraHPC 
{
   nhpc_status_t nhpc_register_to_server(const char **grid_uid, const char *host_addr, const char *host_port);
   nhpc_status_t nhpc_send_file(const char *grid_uid, const char *host_addr, 
				const char *host_port, const char *file_path);
   nhpc_status_t nhpc_send_instruction(const char *grid_uid, const char *host_addr, const char *host_port, 
				       nhpc_instruction_set_t *instruction_set, const char *extra_param1 = NULL,
				       const char *extra_param2 = NULL, const char *extra_param3 = NULL);
   nhpc_status_t nhpc_send_general_instruction(nhpc_instruction_set_t *instruction_set, const char *host_addr,
					       const char *host_port);
};

#endif