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

#ifndef _GRID_PLUGIN_H_
#define _GRID_PLUGIN_H_

#include <pthread.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "rbtree.h"
#include "thread.h"
#include "constants.h"
#include "strings.h"

namespace neweraHPC 
{
   class plugin_manager_t
   {
   private:
      struct plugin_request_t
      {
         int peer_id;
         int request_sent;
         int status;
      };

      rbtree_t *plugins_installed;
      rbtree_t *plugins_requested;
      thread_manager_t **thread_manager;
      pthread_mutex_t *mutex;
      nhpc_status_t install_plugin_dll(const char *dll_path, plugin_details_t **plugin_details);
      nhpc_status_t copy_filetogrid(const char *file_path, const char **base_dir, char **file_path_new);
      
      nhpc_mutex_t *nhpc_mutex;
   public:
      char *grid_directory;

      plugin_manager_t(thread_manager_t **in_thread_manager);
      ~plugin_manager_t();
      void plugin_manager_init();
      void lock();
      void unlock();
      nhpc_status_t search_plugin(const char *plugin_name, plugin_details_t **plugin_details);

      nhpc_status_t request_plugin(int peer_id, char *plugin_name);
      nhpc_status_t recieve_plugin(const char *plugin_name, const char *plugin_type, const char *file_path);

      nhpc_status_t install_plugin(const char *file_path, const char *base_dir = NULL);
   };
   
   void *nhpc_plugin_request_thread(plugin_manager_t *plugin_manager);
   
   nhpc_status_t nhpc_check_nxi(const char *file_path);
   nhpc_status_t nhpc_nxitodll(const char **dll_path, const char *file_path);
   
   nhpc_status_t nhpc_grid_range_plugin_init(plugin_details_t **plugin_details);
   static nhpc_status_t nhpc_grid_range_plugin_exec(nhpc_grid_server_t *grid_server, nhpc_instruction_set_t *instruction_set, 
						    nhpc_socket_t *sock, char **grid_uid);
   static nhpc_status_t nhpc_grid_range_plugin_client_exec(nhpc_grid_server_t *grid_server, nhpc_instruction_set_t *instruction_set, 
							   nhpc_socket_t *sock, char **grid_uid);
   static nhpc_status_t nhpc_grid_range_plugin_processor(nhpc_grid_server_t *grid_server, nhpc_instruction_set_t *instruction_set, 
							      nhpc_socket_t *sock, char **grid_uid);
   
};

#endif
