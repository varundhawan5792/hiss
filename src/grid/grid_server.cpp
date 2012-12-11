/*
*
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/grid.h>

namespace neweraHPC 
{
   nhpc_status_t nhpc_register_to_controller(const char *grid_controller_addr, const char *grid_controller_port,
					     const char *host_addr, const char *host_port, int host_core_count,
					     int host_cpu_time)
   {
      nhpc_status_t nrv;
      nhpc_socket_t *sock;
      
      nrv = socket_connect(&sock, grid_controller_addr, grid_controller_port, AF_INET, SOCK_STREAM, 0);
      
      if(nrv != NHPC_SUCCESS)
      {
	 return errno;
      }      
      
      char *node_cores_str = nhpc_itostr(host_core_count);
      char *node_cpu_time_str = nhpc_itostr(host_cpu_time);
      
      nhpc_headers_t *headers = new nhpc_headers_t;
      headers->insert("GRID NODE_REGISTRATION 2.90");
      headers->insert("Node-Addr", host_addr);
      headers->insert("Node-Port", host_port);
      headers->insert("Node-Cores", node_cores_str);
      headers->insert("Node-Cpu-Time", node_cpu_time_str);
      headers->write(sock);
      
      nhpc_string_delete(node_cores_str);
      nhpc_string_delete(node_cpu_time_str);
      delete headers;
      
      socket_close(sock);
      socket_delete(sock);
   
      return NHPC_SUCCESS;
   }
};


