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

#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/grid.h>
#include <include/network.h>
#include <include/system.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t nhpc_grid_range_plugin_init(plugin_details_t **plugin_details)
   {
      (*plugin_details) = new plugin_details_t;
      memset(*plugin_details, 0, sizeof(plugin_details_t));
      
      (*plugin_details)->fnc_exec = (fnc_ptr_nhpc_plugin_t)nhpc_grid_range_plugin_exec;
      (*plugin_details)->fnc_client_exec = (fnc_ptr_nhpc_plugin_t)nhpc_grid_range_plugin_client_exec;
      (*plugin_details)->fnc_processor = (fnc_ptr_nhpc_plugin_t)nhpc_grid_range_plugin_processor;
      (*plugin_details)->plugin_name = (char *)GRID_RANGE_PLUGIN;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_grid_range_plugin_exec(nhpc_grid_server_t *grid_server, nhpc_instruction_set_t *instruction_set,
					     nhpc_socket_t *sock, char **grid_uid)
   {
      network_t *network = sock->server_details->main_network;
      int *argument_count = &(instruction_set->argument_count);
      rbtree_t *headers = sock->headers;
      
      int start = 0;
      int end   = 0;
      char *file = NULL;
      int range_pos = -1;
      
      for(int i = 1; i <= *argument_count; i++)
      {
	 char *argument = (char *)instruction_set->arguments->search(i);
	 
	 string_t *string = nhpc_substr(argument, ',');
	 int option = nhpc_strtoi(string->strings[0]);
	 
	 switch(option)
	 {
	    case RANGE:
	       start = nhpc_strtoi(string->strings[1]);
	       end   = nhpc_strtoi(string->strings[2]);
	       range_pos = i;
	       break;	       
	 };
	 
	 nhpc_string_delete(string);
      }
      
      for(int i = start; i <= end; i++)
      {
	 nhpc_instruction_set_t *new_instruction_set;
	 nhpc_generate_general_instruction(&new_instruction_set, headers);
	 
	 for(int j = 1; j <= *argument_count; j++)
	 {
	    char *j_str = nhpc_itostr(j);
	    char *argument = nhpc_strconcat("Argument", j_str);
	    nhpc_string_delete(j_str);
	    char *argument_value = (char *)headers->search(argument);
	    nhpc_string_delete(argument);
	    
	    if(j == range_pos)
	       nhpc_add_argument(new_instruction_set, VALUE, &i);
	    else 
	    {
	       char *tmp_str1;
	       nhpc_strcpy(&tmp_str1, argument_value);
	       
	       new_instruction_set->arguments->insert(tmp_str1);
	    }
	 }
	 
	 new_instruction_set->execute = true;
	 grid_server->queue_job(new_instruction_set);
	 LOG_INFO("queueing job no: " << i);
      }
      
      nhpc_delete_instruction(instruction_set);
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_grid_range_plugin_client_exec(nhpc_grid_server_t *grid_server, nhpc_instruction_set_t *instruction_set, 
						    nhpc_socket_t *sock, char **grid_uid)
   {
   }
   
   nhpc_status_t nhpc_grid_range_plugin_processor(nhpc_grid_server_t *grid_server, nhpc_instruction_set_t *instruction_set, 
						  nhpc_socket_t *sock, char **grid_uid)
   {
      network_t *network = sock->server_details->main_network;
      rbtree_t *headers = sock->headers;      
      int *argument_count = &(instruction_set->argument_count);
      
      char *exec = NULL;
      
      int rv, pid;
      nhpc_status_t nrv;
      
      for(int i = 1; i <= *argument_count; i++)
      {
	 char *argument = (char *)instruction_set->arguments->search(i);
	 
	 string_t *string = nhpc_substr(argument, ',');
	 int option = nhpc_strtoi(string->strings[0]);
	 
	 char *tmp_str1;
	 char *tmp_str2;
	 char *tmp_str3;
	 char *tmp_str4;
	 
	 switch(option)
	 {
	    case LITERAL:
	       if(exec)
	       {
		  tmp_str1 = nhpc_strconcat(exec, string->strings[1], " ");
		  nhpc_string_delete(exec);
		  exec = tmp_str1;
	       }
	       break;
	       
	    case COMMAND:
	       exec = nhpc_strconcat(string->strings[1], " ");
	       break;
	       
	    case GRID_FILE:
	       tmp_str1 = nhpc_strconcat("/www/grid/", *grid_uid, "/");
	       tmp_str2 = nhpc_strconcat(tmp_str1, string->strings[1], " ");
	       if(exec)
	       {
		  tmp_str3 = nhpc_strconcat(exec, tmp_str2);
		  nhpc_string_delete(exec);
		  exec = tmp_str3;
	       }
	       else 
		  nhpc_strcpy(&exec, tmp_str2);
	       
	       nhpc_string_delete(tmp_str1);
	       nhpc_string_delete(tmp_str2);
	       
	       break;
	       
	    case VALUE:
	       if(exec)
	       {
		  tmp_str1 = nhpc_strconcat(exec, string->strings[1], " ");
		  nhpc_string_delete(exec);
		  exec = tmp_str1;
	       }
	       break;
	 };
	 
	 nhpc_string_delete(string);
      }
      
      string_t *exec_args = nhpc_substr(exec, ' ');
      string_t *exec_path = nhpc_substr(exec_args->strings[0], '/');
      
      char **args = new char *[exec_args->count];
      memcpy((args + 1), (exec_args->strings + 1), (sizeof(char**) * (exec_args->count - 1)));
      args[0] = exec_path->strings[exec_path->count - 1];
      args[exec_args->count] = NULL;

      grid_server->extern_system->create_child_process((char *)"GRID", &pid);
      if(pid == 0)
      {
	 execve(exec_args->strings[0], args, NULL);
	 
	 _exit(EXIT_SUCCESS);
      }
      else 
      {
	 task_t *task = new task_t;
	 time(&(task->t));
	 task->instruction_set = instruction_set;
	 grid_server->add_child_process(task, &pid);
	 
	 nhpc_string_delete(exec_args);
	 nhpc_string_delete(exec_path);
	 delete[] args;
      }
      
      if(exec)
	 nhpc_string_delete(exec);
      
      return NHPC_SUCCESS;
   }
};