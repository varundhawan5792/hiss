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

#include <sys/poll.h>
#include <errno.h>
#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/network.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t nhpc_wait_for_io_or_timeout(struct nhpc_socket_t *sock, int for_read)
   {
      struct pollfd pfd;
      int rc, timeout;
      
      timeout = 3000;
      pfd.fd = sock->sockfd;
      pfd.events = for_read ? POLLIN : POLLOUT;      
      
      do 
      {
	 rc = poll(&pfd, 1, timeout);
      }while (rc == -1 && errno == EINTR);
      
      if(rc == 0) 
      {
	 return NHPC_TIMEUP;
      }
      else if(rc > 0) 
      {
	 return NHPC_SUCCESS;
      }
      else 
      {
	 return errno;
      }
      
      return NHPC_SUCCESS;
   }   
}