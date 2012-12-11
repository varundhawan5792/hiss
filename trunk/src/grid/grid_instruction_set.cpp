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

using namespace std;

namespace neweraHPC
{
   nhpc_status_t nhpc_create_instruction(nhpc_instruction_set_t **instruct_set, const char *plugin_name,
					 const char *host_grid_uid)
   {
      if(plugin_name == NULL)
	 return NHPC_FAIL;
      
      *instruct_set = new nhpc_instruction_set_t;
      memset((*instruct_set), 0, sizeof(nhpc_instruction_set_t));
      (*instruct_set)->arguments = new rbtree_t;
            
      if(host_grid_uid)
      {
	 nhpc_strcpy(&((*instruct_set)->host_grid_uid), host_grid_uid);
      }
      else 
	 (*instruct_set)->host_grid_uid = NULL;
      
      nhpc_strcpy(&((*instruct_set)->plugin_name), plugin_name);
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_delete_instruction(nhpc_instruction_set_t *instruct_set)
   {
      if(instruct_set->arguments)
      {
	 int count = instruct_set->arguments->ret_count();
	 
	 for(int i = 1; i <= count; i++)
	 {
	    char *argument_value = (char *)instruct_set->arguments->search(i);
	    nhpc_string_delete(argument_value);
	 }
	 
	 delete (instruct_set->arguments);
      }
      
      if(instruct_set->grid_uid)
	 nhpc_string_delete((instruct_set->grid_uid));
      if(instruct_set->host_grid_uid)
	 nhpc_string_delete((instruct_set->host_grid_uid));
      if(instruct_set->host_peer_port)
	 nhpc_string_delete((instruct_set->host_peer_port));
      if(instruct_set->host_peer_addr)
	 nhpc_string_delete((instruct_set->host_peer_addr));
      
      nhpc_string_delete((instruct_set->plugin_name));
      delete instruct_set;
   }
   
   nhpc_status_t nhpc_generate_instruction(nhpc_instruction_set_t **instruction_set, rbtree_t *headers)
   {
      nhpc_status_t nrv = NHPC_SUCCESS;
            
      nrv = nhpc_generate_general_instruction(instruction_set, headers);
      
      char *arg_count_str = (char *)headers->search("Argument-Count");
      int arg_count = nhpc_strtoi(arg_count_str);
      (*instruction_set)->argument_count = arg_count;
      for(int i = 1; i <= arg_count; i++)
      {
	 char *i_str = nhpc_itostr(i);
	 char *search_string = nhpc_strconcat("Argument", i_str);
	 char *arg_value = (char *)headers->search(search_string);
	 if(!arg_value)
	    nrv = NHPC_FAIL;
	 
	 char *tmp_str;
	 nhpc_strcpy(&tmp_str, arg_value);
	 
	 (*instruction_set)->arguments->insert(tmp_str);
	 nhpc_string_delete(search_string);
	 nhpc_string_delete(i_str);
      }
      
      if(nrv != NHPC_SUCCESS)
	 return nrv;
      
      return NHPC_SUCCESS;
   } 
   
   nhpc_status_t nhpc_generate_general_instruction(nhpc_instruction_set_t **instruction_set,
						   rbtree_t *headers)
   {
      nhpc_status_t nrv = NHPC_SUCCESS;
      
      char *plugin_name = (char *)headers->search("Plugin");
      char *host_grid_uid = (char *)headers->search("Host-Grid-Uid");
      char *grid_uid = (char *)headers->search("Grid-Uid");      
      char *peer_id_str = (char *)headers->search("Peer");
      int peer_id;
      char *peer_addr = (char *)headers->search("Peer-Host");
      char *peer_port = (char *)headers->search("Peer-Port");
      char *execution_state = (char *)headers->search("Execution-State");

      nhpc_create_instruction(instruction_set, plugin_name);

      if(host_grid_uid)
	 nhpc_strcpy(&((*instruction_set)->host_grid_uid), host_grid_uid);
      else 
	 nhpc_strcpy(&((*instruction_set)->host_grid_uid), grid_uid);
      
      if(grid_uid)
	 nhpc_strcpy(&((*instruction_set)->grid_uid), grid_uid);
      
      if(peer_id_str)
      {
	 peer_id = nhpc_strtoi(peer_id_str);
	 if(peer_id > 0)
	    (*instruction_set)->host_peer_id = peer_id;
      }
      
      nhpc_strcpy(&((*instruction_set)->host_peer_addr), peer_addr);
      nhpc_strcpy(&((*instruction_set)->host_peer_port), peer_port);
      
      if(execution_state)
	 (*instruction_set)->execute = true;
      else 
	 (*instruction_set)->execute = false;  
      
      if(nrv != NHPC_SUCCESS)
	 return nrv;
      
      return NHPC_SUCCESS;
   }

   nhpc_status_t nhpc_add_argument(nhpc_instruction_set_t *instruction, enum GRID_ARG_TYPE option, 
				   const void *arg1, const void *arg2)
   {
      char *tmp_str1;
      char *tmp_str2;
      char *tmp_str3;
      char *argument;
      char *option_str = nhpc_itostr(option);
      
      int *arg1_n;
      int *arg2_n;
      char *arg1_n_str;
      char *arg2_n_str;
      
      char *arg1_str;
      char *arg2_str;
      
      switch(option)
      {
	 case VALUE:
	    if(!arg1)
	       return NHPC_FAIL;
	    arg1_n = (int *)arg1;
	    arg1_n_str = nhpc_itostr(*arg1_n);
	    
	    tmp_str1 = nhpc_strconcat(option_str, ",");
	    argument = nhpc_strconcat(tmp_str1, arg1_n_str);
	    nhpc_string_delete(arg1_n_str);
	    
	    instruction->arguments->insert(argument);
	    
	    nhpc_string_delete(tmp_str1);
	    break;
	    
	 case RANGE:
	    if(!arg1 || !arg2)
	       return NHPC_FAIL;
	    arg1_n = (int *)arg1;
	    arg2_n = (int *)arg2;
	    arg1_n_str = nhpc_itostr(*arg1_n);
	    arg2_n_str = nhpc_itostr(*arg2_n);

	    tmp_str1 = nhpc_strconcat(option_str, ",");
	    tmp_str2 = nhpc_strconcat(arg1_n_str, ",");
	    tmp_str3 = nhpc_strconcat(tmp_str1, tmp_str2);
	    argument = nhpc_strconcat(tmp_str3, arg2_n_str);
	    
	    instruction->arguments->insert(argument);
	    
	    nhpc_string_delete(tmp_str1);
	    nhpc_string_delete(tmp_str2);
	    nhpc_string_delete(tmp_str3);    
	    nhpc_string_delete(arg1_n_str);
	    nhpc_string_delete(arg2_n_str);
	    
	    break;
	    
	 case LITERAL:
	    if(!arg1)
	       return NHPC_FAIL;

	    arg1_str = (char *)arg1;
	    tmp_str1 = nhpc_strconcat(option_str, ",");
	    argument = nhpc_strconcat(tmp_str1, arg1_str);
	    
	    instruction->arguments->insert(argument);
	    
	    nhpc_string_delete(tmp_str1);
	    break;
	    
	 case COMMAND:
	    if(!arg1)
	       return NHPC_FAIL;
	    
	    arg1_str = (char *)arg1;
	    tmp_str1 = nhpc_strconcat(option_str, ",");
	    argument = nhpc_strconcat(tmp_str1, (char *)arg1_str);
	    
	    instruction->arguments->insert(argument);
	    
	    nhpc_string_delete(tmp_str1);
	    break;	    
	    
	 case GRID_FILE:
	    if(!arg1)
	       return NHPC_FAIL;
	    
	    arg1_str = (char *)arg1;
	    tmp_str1 = nhpc_strconcat(option_str, ",");
	    argument = nhpc_strconcat(tmp_str1, (char *)arg1_str);
	    
	    instruction->arguments->insert(argument);
	    
	    nhpc_string_delete(tmp_str1);
	    break;	    	    
      };
      
      nhpc_string_delete(option_str);
   }
};
