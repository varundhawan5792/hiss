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


#ifndef _SOCKETS_H_
#define _SOCKETS_H_

#include "constants.h"
#include "communication.h"

namespace neweraHPC
{
   nhpc_status_t socket_init(nhpc_socket_t **sock);
   
   nhpc_status_t socket_create(nhpc_socket_t **sock);
   nhpc_status_t socket_create(nhpc_socket_t **sock, int family, int type, int protocol);
   
   nhpc_status_t socket_delete(nhpc_socket_t *sock);
   nhpc_status_t socket_close(nhpc_socket_t *sock);
   
   nhpc_status_t socket_connect(nhpc_socket_t *sock);
   nhpc_status_t socket_connect(nhpc_socket_t **sock, const char *host_addr, const char *host_port,
				int family, int type, int protocol);
   
   nhpc_status_t socket_bind(nhpc_socket_t *sock);
   nhpc_status_t socket_listen(nhpc_socket_t *sock, int *connection_queue);
   
   nhpc_status_t socket_getaddrinfo(nhpc_socket_t **sock, const char *host_addr, const char *host_port,
				    int family, int type, int protocol);
   void *get_in_addr(struct sockaddr *sa);
   
   /* Function for receiving data from scoket file description */
   nhpc_status_t socket_recv(nhpc_socket_t *sock, char *buffer, nhpc_size_t *length);
   
   /* Function for sending data to socket file description */
   nhpc_status_t socket_send(nhpc_socket_t *sock, const char *buffer, nhpc_size_t *length);
   nhpc_status_t socket_sendmsg(nhpc_socket_t *sock, const char *buffer, nhpc_size_t *length);
   nhpc_status_t socket_send_file(nhpc_socket_t *sock);
   
   nhpc_status_t nhpc_wait_for_io_or_timeout(nhpc_socket_t *sock, int for_read);
}

#endif
