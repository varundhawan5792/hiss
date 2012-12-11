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

#include <config.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/http.h>
#include <include/rbtree.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t read_headers(rbtree_t *headers, http_data_t **http_data)
   {
      char *command = (char *)headers->search("command");
      
      char *http_version = NULL;
      char *status_str = NULL;
      char *referer = (char *)headers->search("Referer");
      char *origin = (char *)headers->search("Origin");
      char *content_type = (char *)headers->search("Content-Type");
      char *host = (char *)headers->search("Host");
      char *content_length = (char *)headers->search("Content-Length");
      char *user_agent = (char *)headers->search("User-Agent");
      int request_type;
      char *request_page = NULL;
      char *request_get = NULL;
      
      string_t *command_str = nhpc_substr(command, ' ');
      if(command_str->count == 2)
      {
	 /* TODO HTTP SERVER */
	 request_type = HTTP_INVALID;

	 nhpc_string_delete(command_str);
	 return NHPC_FAIL;
	 
	 nhpc_strcpy(&http_version, (command_str->strings[0]));
      }
      else if(command_str->count == 3)
      {
	 char *request_type_str = command_str->strings[0];
	 if(nhpc_strcmp(request_type_str, "GET") == NHPC_SUCCESS)
	    request_type = HTTP_REQUEST_GET;
	 else if(nhpc_strcmp(request_type_str, "POST") == NHPC_SUCCESS)
	    request_type = HTTP_REQUEST_POST;
	 else 
	 {
	    request_type = HTTP_INVALID;
	    return NHPC_FAIL;
	 }
	 
	 char *request = command_str->strings[1];
	 if(nhpc_strcmp(request, "*?*") == NHPC_SUCCESS)
	 {
	    int tmp_pos = nhpc_strfind(request, '?');
	    request_page = nhpc_substr(request, 1, tmp_pos - 1);
	    request_get = nhpc_substr(request, tmp_pos, strlen(request));
	 }
	 else 
	    nhpc_strcpy(&request_page, (command_str->strings[1]));
	    	 
	 nhpc_strcpy(&http_version, (command_str->strings[2]));
      }
      nhpc_string_delete(command_str);
      
      (*http_data) = new http_data_t;
      http_data_t *local_data = (*http_data);
      memset(local_data, 0, sizeof(http_data_t));
      if(content_length)
      {
	 local_data->content_length = nhpc_strtoi(content_length);
      }

      local_data->http_version = http_version;
      local_data->user_agent = user_agent;
      local_data->request_page = request_page;
      local_data->request_get = request_get;
      local_data->status_str = status_str;
      local_data->request_type = request_type;
      local_data->host = host;
      local_data->origin = origin;
      local_data->referer = referer;
      local_data->content_type = content_type;
      local_data->headers = headers;
      
      return NHPC_SUCCESS;
   }
   
   void delete_http_header(http_data_t *http_data)
   {
      if(http_data->request_page)
	 delete[] (http_data->request_page);
      if(http_data->request_get)
	 delete[] (http_data->request_get);
      if(http_data->status_str)
	 delete[] (http_data->status_str);
      if(http_data->http_version)
	 delete[] (http_data->http_version);    
   }
   
   nhpc_status_t http_content_length(rbtree_t *headers, nhpc_size_t *size)
   {
      char *file_size_str = (char *)headers->search("Content-Length");
      if(file_size_str)
      {
	 (*size) = nhpc_strtoi(file_size_str);
	 return NHPC_SUCCESS;
      }
      
      (*size) = -1;
      return NHPC_FAIL;
   }
};
