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

#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "network.h"
#include "headers.h"

namespace neweraHPC
{
   struct communication_t
   {
      nhpc_socket_t *sock;
   };
   
   void read_communication(nhpc_socket_t *sock);
   
   nhpc_status_t nhpc_analyze_stream(nhpc_socket_t *sock, char *buffer, nhpc_size_t *len, nhpc_size_t *header_size);
   void nhpc_display_headers(nhpc_socket_t *sock);
};

#endif
