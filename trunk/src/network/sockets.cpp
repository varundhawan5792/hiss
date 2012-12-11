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

/* These routines are usefull for creating listening sockets(opening sockets)
 or connecting to other machines on given port */

#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/network.h>
#include <include/sockets.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t socket_create(nhpc_socket_t **sock, int family, int type, int protocol)
   {
      (*sock)->sockfd = socket(family, type, protocol);
      if((*sock)->sockfd == -1)
      {
	 perror("error at creating sockets");
	 return errno;
      }
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t socket_create(nhpc_socket_t **sock)
   {
      addrinfo **hints_res = &((*sock)->hints_res);
      
      int rv = socket_create(sock, (*hints_res)->ai_family, (*hints_res)->ai_socktype, (*hints_res)->ai_protocol);
      return rv;
   }
   
   nhpc_status_t socket_connect(nhpc_socket_t *sock) 
   {
      int rv;
      
      if (!(sock->hints_res))
      {
	 return 0;
      }
      
      addrinfo *hints_res = sock->hints_res;
      
      do 
      {
	 rv = connect(sock->sockfd, hints_res->ai_addr, hints_res->ai_addrlen);
      }while (rv == -1 && errno == EINTR);
      
      if(rv == -1 && (errno == EINPROGRESS || errno == EALREADY))
      {
	 rv = nhpc_wait_for_io_or_timeout(sock, 0);
	 if(rv != NHPC_SUCCESS)
	    return rv;
	 else if(rv == NHPC_SUCCESS)
	 {
	    rv = connect(sock->sockfd, hints_res->ai_addr, hints_res->ai_addrlen);
	 }
      }
	 
      if(rv == -1 && errno != EISCONN)
	 return errno;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t socket_init(nhpc_socket_t **sock)
   {
      *sock = new nhpc_socket_t;
      memset(*sock, 0, sizeof(nhpc_socket_t));
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t socket_connect(nhpc_socket_t **sock, const char *host_addr, const char *host_port,
				int family, int type, int protocol)
   {
      nhpc_status_t nrv;
      
      socket_init(sock);
      
      nrv = socket_getaddrinfo(sock, host_addr, host_port, family, type, protocol);
      if(nrv != NHPC_SUCCESS)
      {
	 socket_delete(*sock);
	 return nrv;
      }
      
      nrv = socket_create(sock);     
      if(nrv != NHPC_SUCCESS)
      {
	 socket_delete(*sock);
	 return nrv;
      }   
      
      socket_options_set(*sock, NHPC_NONBLOCK, 1);
      
      do 
      {
	 nrv = socket_connect(*sock);
      }while(nrv != NHPC_TIMEUP && (errno == EINPROGRESS || errno == EALREADY));
      
      if(nrv != NHPC_SUCCESS)
      {
	 socket_close(*sock);
	 socket_delete(*sock);
	 return nrv;
      }      

      return NHPC_SUCCESS;      
   }
     
   nhpc_status_t socket_bind(nhpc_socket_t *sock)
   {
      int rv;
      rv = bind(sock->sockfd, sock->hints_res->ai_addr, sock->hints_res->ai_addrlen);
      
      if(rv == -1)
	 return errno;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t socket_listen(nhpc_socket_t *sock, int *connection_queue)
   {
      int rv;
      rv = listen(sock->sockfd, *connection_queue);
      
      if(rv < 0)
	 return errno;
      
      return NHPC_SUCCESS;
   }
      
   nhpc_status_t socket_getaddrinfo(nhpc_socket_t **sock, const char *host_addr, const char *host_port,
				    int family, int type, int protocol) 
   {
      int rv;
      
      addrinfo **hints, **res, *p;
      
      hints = &((*sock)->hints);
      res   = &((*sock)->hints_res);
      
      (*hints) = new addrinfo;
      memset(*hints, 0, sizeof(addrinfo));
      
      (*hints)->ai_family   = family;
      (*hints)->ai_socktype = type;
      (*hints)->ai_protocol = protocol;
      
      nhpc_status_t nrv = nhpc_wait_for_io_or_timeout(*sock, 0);
      
      rv = getaddrinfo(host_addr, host_port, *hints, res);
      if (rv != 0)
      {
	 delete (*hints);
	 (*hints) = NULL;
	 (*res) = NULL;
	 return rv;
      }
      
      return NHPC_SUCCESS;
   }
   
   void *get_in_addr(struct sockaddr *sa)
   {
      if(sa->sa_family == AF_INET) 
      {
	 return &(((struct sockaddr_in*)sa)->sin_addr);
      }
      return &(((struct sockaddr_in6*)sa)->sin6_addr);
   }      
   
   nhpc_status_t socket_delete(nhpc_socket_t *sock)
   {
      if(sock)
      {
	 if(sock->hints_res)
	    freeaddrinfo(sock->hints_res);
	 if(sock->hints)
	    delete sock->hints;
	 
	 if(sock->headers)
	 {
	    nhpc_delete_headers(sock->headers);
	    delete (sock->headers);
	 }

	 delete sock;
	 sock = NULL;
      }
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t socket_close(nhpc_socket_t *sock)
   {
      close(sock->sockfd);
      
      return NHPC_SUCCESS;
   }
}
