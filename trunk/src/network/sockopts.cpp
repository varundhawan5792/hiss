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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/network.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t socket_options_set(nhpc_socket_t *sock, int sockopt, int on)
   {
      int one;
      nhpc_status_t status;
      
      one = on ? 1 : 0;
      
      switch (sockopt)
      {
	 case NHPC_REUSEADDR:
	    if (on != socket_is_option_set(sock, sockopt))
	    {
	       status = setsockopt(sock->sockfd, SOL_SOCKET, NHPC_REUSEADDR, 
				   (void *)&one, sizeof(int));
	       if (status == -1)
		  return errno;
	    }
	    break;
	 case NHPC_NONBLOCK:
	    if (socket_is_option_set(sock, NHPC_NONBLOCK) != on) 
	    {
	       if (on) 
	       {
		  if ((status = socket_nonblock(sock)) != NHPC_SUCCESS) 
		     return status;
	       }
	       else 
	       {
		  if ((status = socket_block(sock)) != NHPC_SUCCESS)
		     return status;
	       }
	       socket_set_option(sock, NHPC_NONBLOCK, on);
	       sock->timeout = 3000;
	    }
	    break;	    
      }
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t socket_nonblock(nhpc_socket_t *sock)
   {
      int rv = fcntl(sock->sockfd, F_SETFL, O_NONBLOCK);
      if(rv == -1)return errno;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t socket_block(nhpc_socket_t *sock)
   {
      int fd_flags = fcntl(sock->sockfd, F_GETFL, 0);
      fd_flags &= ~O_NONBLOCK;
      
      int rv = fcntl(sock->sockfd, F_SETFL, fd_flags);
      if(rv == -1)return errno;
      
      return NHPC_SUCCESS;
   }
}