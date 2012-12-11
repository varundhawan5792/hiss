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
#include <include/headers.h>
#include <include/sockets.h>
#include <include/strings_pool.h>

using namespace std;

namespace neweraHPC
{
   nhpc_headers_t::nhpc_headers_t()
   {
      headers = new rbtree_t;
   }
   
   nhpc_headers_t::~nhpc_headers_t()
   {
      for(int i = 1; i <= headers->ret_count(); i++)
      {
	 char *tmp_string = (char *)headers->search(i);
	 nhpc_string_delete(tmp_string);
	 headers->erase(i);
      }
      
      delete headers;
   }
   
   void nhpc_headers_t::insert(const char *string)
   {
      char *new_str = nhpc_strconcat(string, "\r\n");
      
      headers->insert(new_str);
   }
   
   void nhpc_headers_t::insert(const char *property, const char *argument)
   {
      char *tmp_property = nhpc_strconcat(property, ": ", argument);
      insert(tmp_property);
      delete[] tmp_property;
      //nhpc_deallocate_str(tmp_property);
   }
   
   const char *nhpc_headers_t::operator[](int id)
   {
      const char *tmp_str = (const char *)headers->search(id + 1);
      
      return tmp_str;
   }
   
   int nhpc_headers_t::count()
   {
      return headers->ret_count();
   }
   
   nhpc_status_t nhpc_headers_t::write(nhpc_socket_t *sock)
   {
      nhpc_size_t size;
      nhpc_status_t nrv;
      
      int count = headers->ret_count();
      
      for(int i = 1; i <= count; i++)
      {
	 char *str = (char *)headers->search(i);
	 size = strlen(str);
	 
	 nrv = socket_sendmsg(sock, str, &size);
	 
	 nhpc_string_delete(str);
	 headers->erase(i);
      }
      
      size = 2;
      nrv = socket_sendmsg(sock, "\r\n", &size);
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_headers_insert_param(rbtree_t *headers, const char *str)
   {
      char *argument;
      
      if(headers->ret_count() == 0)
      {
	 nhpc_strcpy(&argument, str);
	 headers->insert(argument, "command");
	 return NHPC_SUCCESS;
      }
      
      if(nhpc_strcmp(str, "*: *") == NHPC_FAIL)
	 return NHPC_FAIL;
      
      string_t *string = nhpc_substr(str, ':');
      if((string->count == 1) || (strlen(string->strings[1]) == 1) || string->strings[1][1] == ' ')
      {
	 nhpc_string_delete(string);
	 return NHPC_FAIL;
      }
      
      if(string->count > 2)
      {
	 char *tmp_str;
	 nhpc_strcpy(&tmp_str, string->strings[1]);
	 int count = string->count;
	 
	 for(int i = 2; i < count; i++)
	 {
	    char *tmp_str_2 = nhpc_strconcat(tmp_str, ":", string->strings[i]);
	    //nhpc_deallocate_str(tmp_str);
	    delete[] tmp_str;
	    tmp_str = tmp_str_2;
	 }
	 
	 nhpc_strcpy(&argument, (tmp_str + 1));
	 nhpc_string_delete(tmp_str);
      }
      else 
	 nhpc_strcpy(&argument, (string->strings[1] + 1));
      
      headers->insert(argument, string->strings[0]);
      
      nhpc_string_delete(string);
      
      return NHPC_SUCCESS;
   }
   
   void nhpc_delete_headers(rbtree_t *headers)
   {
      const char *key;
      char *string;
      
      do 
      {
	 string = (char *)headers->search_first(&key);
	 if(string)
	 {
	    //nhpc_deallocate_str(string);
	    delete[] string;
	    headers->erase(key);
	 }
      }while(string);
   }
}
