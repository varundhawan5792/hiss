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

#include <errno.h>
#include <iostream>
#include <iomanip>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/network.h>
#include <include/error.h>

using namespace std;

namespace neweraHPC
{
   network_t *network;
   
   network_t::network_t()
   {
      external_thread_manager = false;
      thread_manager = new thread_manager_t*;
      *thread_manager = new thread_manager_t;
      
      network_init();
   }
   
   network_t::network_t(thread_manager_t **in_thread_manager)
   {
      external_thread_manager = true;
      thread_manager = in_thread_manager;
      
      network_init();
   }
   
   void network_t::network_init()
   {
      client_connections = new rbtree_t;
      mutex = new pthread_mutex_t;   
      mutex_addons = new pthread_mutex_t;
      network_addons = new rbtree_t(NHPC_RBTREE_STR);
      server_sock = NULL;    
      network = this;
      accept_thread_id = 0;
   }
   
   void network_t::network_quit()
   {
      LOG_INFO("Initiating Server Shutdown");
      (**thread_manager).cancel_thread(accept_thread_id);
   }
   
   network_t::~network_t()
   {
      LOG_INFO("Deleting network structure");
      
      lock();
      
      LOG_INFO("Closing all active sockets\t");
      nhpc_socket_t *client_sock;
      int key;
      while(1)
      {
	 client_sock = (nhpc_socket_t *)(*client_connections).search_first(&key);
	 if(!client_sock)
	    break;
	 
	 socket_close(client_sock);
	 socket_delete(client_sock);
	 (*client_connections).erase(key);
      }
      
      delete client_connections;
      
      LOG_INFO("Deleting network addons");
      delete network_addons;
      
      LOG_INFO("Closing server socket");
      if(server_sock)
      {
	 socket_close(server_sock);
	 socket_delete(server_sock);
      }
      
      if(!external_thread_manager) 
      {
	 LOG_INFO("Shuting down thread manager");
	 delete (*thread_manager);
      }
      
      delete mutex;
      
      LOG_INFO("Network stack deleted");
   }
   
   void exit_handler(int signum)
   {
      network->network_quit();
   }
   
   inline void network_t::lock()
   {
      pthread_mutex_lock(mutex);
   }
   
   inline void network_t::unlock()
   {
      pthread_mutex_unlock(mutex);
   }
   
   inline void network_t::lock_addons()
   {
      pthread_mutex_lock(mutex_addons);
   }
   
   inline void network_t::unlock_addons()
   {
      pthread_mutex_unlock(mutex_addons);
   }
   
   int network_t::add_client_connection(nhpc_socket_t *sock, int sockfd)
   {
      int id;
      lock();
      id = (*client_connections).insert(sock);
      unlock();
      
      return id;
   }
   
   nhpc_status_t network_t::connect(nhpc_socket_t **sock, const char *host_addr, 
				    const char *host_port, int family, int type, int protocol)
   {
      nhpc_status_t nrv;
      
      nrv = socket_connect(sock, host_addr, host_port, family, type, protocol);
      if(nrv != NHPC_SUCCESS)
      {
	 perror("connects");
	 return nrv;
      }      
      
      add_client_connection(*sock, (*sock)->sockfd);
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t network_t::create_server(const char *host_addr, const char *host_port,
					  int family, int type, int protocol)
   {            
      int rv, nrv;
      int connection_queue = CONNECTION_QUEUE;
      
      int enable_opts = 1;
      
      if(server_sock == NULL)
	 socket_init(&server_sock);
      
      nrv = socket_getaddrinfo(&server_sock, host_addr, host_port, family, type, protocol);
      if(nrv != NHPC_SUCCESS)
      {
	 delete server_sock;
	 server_sock = NULL;
	 return nrv;
      }      
      
      nrv = socket_create(&server_sock);     
      if(nrv != NHPC_SUCCESS)
      {
	 delete server_sock;
	 return nrv;
      }    
      
      socket_options_set(server_sock, NHPC_NONBLOCK, 1);
      
      rv = setsockopt(server_sock->sockfd, SOL_SOCKET, SO_REUSEADDR, &enable_opts, sizeof(int));
      if(rv == -1)
      {
	 delete server_sock;
	 server_sock = NULL;
	 return errno;
      }
      
      nrv = socket_bind(server_sock);
      if(nrv != NHPC_SUCCESS)
      {
	 delete server_sock;
	 server_sock = NULL;
	 return errno;
      }
      
      nrv = socket_listen(server_sock, &connection_queue);
      if(nrv != NHPC_SUCCESS)
      {
	 delete server_sock;
	 server_sock = NULL;
	 return errno;
      }
      
      nhpc_thread_details_t *accept_thread = new nhpc_thread_details_t;
      accept_thread->sock           = server_sock;
      accept_thread->thread_manager = *thread_manager;
      accept_thread->client_socks   = client_connections;
      accept_thread->network        = this;
      (**thread_manager).init_thread(&accept_thread_id, NULL);
      (**thread_manager).create_thread(&accept_thread_id, NULL, (void * (*)(void *))network_t::accept_connection, 
				       (void *)accept_thread, NHPC_THREAD_JOIN);
      
      return NHPC_SUCCESS;      
   }
   
   void sig_action(int sig)
   {
      if(sig == SIGPIPE)
	 LOG_ERROR("PIPE Broken at thread - "<<pthread_self());
   }
   
   void *network_t::accept_connection(nhpc_thread_details_t *main_thread)
   {
      signal(SIGPIPE, SIG_IGN);
      signal(SIGINT, exit_handler);
      
      nhpc_size_t rv;
      nhpc_status_t nrv;
      pthread_mutex_t mutex;
      pthread_mutex_init(&mutex, NULL);
      
      thread_manager_t *thread_manager = main_thread->thread_manager;
      nhpc_socket_t *server_sock = main_thread->sock;
      rbtree_t *client_socks = main_thread->client_socks;
      
      struct pollfd *fds = new pollfd;
      int timeout        = (3 * 60 * 1000);
      
      nhpc_server_details_t *server_details = new nhpc_server_details_t;
      server_details->mutex        = &mutex;
      server_details->client_socks = client_socks;
      server_details->thread_manager = thread_manager;
      server_details->main_network = main_thread->network;
      
      int *server_sockfd = &(main_thread->sock->sockfd);
      
      fds[0].fd     = *server_sockfd;
      fds[0].events = POLLIN;
      
      bool end_server = false;
      
      do 
      {
	 rv = poll(fds, 1, timeout);
	 
	 if(rv < 0)
	 {
	    break;
	 }
	 
	 if(rv == 0)
	 {
	    continue;
	 }
	 
	 int new_sd;
	 
	 do
	 {
	    nhpc_size_t size = sizeof(sockaddr_in);
	    struct sockaddr_in *client_sockaddr = new sockaddr_in;
	    
	    new_sd = accept(*server_sockfd, (sockaddr *)client_sockaddr, (socklen_t *)&size);
	    
	    if(new_sd < 0)
	    {
	       delete client_sockaddr;  
	       
	       if(errno != EWOULDBLOCK)
	       {
		  end_server = true;
		  break;
	       }
	       break;
	    }
	    
	    nhpc_socket_t *client_sock;
	    socket_init(&client_sock);
	    client_sock->sockfd = new_sd;
	    client_sock->server_details = server_details;
	    client_sock->timeout = 3 * 60 * 60;
	    nhpc_strcpy_noalloc((client_sock->host), inet_ntoa(client_sockaddr->sin_addr));
	    char *tmp_port = nhpc_itostr(ntohs(client_sockaddr->sin_port));
	    nhpc_strcpy_noalloc((client_sock->port), tmp_port);
	    nhpc_string_delete(tmp_port);
	    
	    delete client_sockaddr;
	    
	    pthread_mutex_lock(&mutex);
	    client_socks->insert(client_sock, new_sd);
	    pthread_mutex_unlock(&mutex);
	    
	    
	     (*thread_manager).init_thread(&(client_sock->thread_id), NULL);
	     (*thread_manager).create_thread(&(client_sock->thread_id), NULL, (void* (*)(void*))read_communication, 
	     client_sock, NHPC_THREAD_DETACH);	
	    
	    //read_communication(client_sock);
	 }while(new_sd != -1);
      }while(true);
   }
   
   void nhpc_socket_cleanup(nhpc_socket_t *client_sock)
   {      
      if(client_sock != NULL)
      {
	 rbtree_t *client_socks = client_sock->server_details->client_socks;
	 pthread_mutex_t *mutex = client_sock->server_details->mutex;
	 thread_manager_t *thread_manager = client_sock->server_details->thread_manager;
	 
	 pthread_mutex_lock(mutex);
	 client_socks->erase(client_sock->sockfd);
	 pthread_mutex_unlock(mutex);
	 
	 thread_manager->delete_thread_data(client_sock->thread_id);
	 
	 socket_close(client_sock);
	 socket_delete(client_sock);
      }
   }
};
