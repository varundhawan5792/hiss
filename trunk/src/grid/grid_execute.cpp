/*
 *	(C) 2011 Varun Mittal <varunmittal91@gmail.com>
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

#include <iostream>
#include <unistd.h>
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/grid.h>
#include <include/headers.h>
#include <include/network.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t nhpc_grid_server_t::grid_execute(nhpc_instruction_set_t *instruction_set, 
						  nhpc_socket_t *sock, const char **grid_uid)
   {
      char *plugin = instruction_set->plugin_name;
      
      plugin_details_t *plugin_details;
      search_plugin(plugin, &plugin_details);
      if(!plugin_details)
	 return NHPC_FAIL;
      
      nhpc_status_t nrv = NHPC_FAIL;
      
      if(instruction_set->execute)
      {
	 nrv = plugin_details->fnc_processor(this, instruction_set, sock, (void **)grid_uid);
      }
      else 
      {
	 nrv = plugin_details->fnc_exec(this, instruction_set, sock, (void **)grid_uid);
      }
      
      return nrv;
   }
};
