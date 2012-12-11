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
#include <fstream>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/network.h>
#include <include/general.h>
#include <include/web_ui.h>

using namespace std;

namespace neweraHPC
{
   const char *request_type_strings[] = 
   {
      "HTTP_INVALID",
      "HTTP_REQUEST_GET",
      "HTTP_REQUEST_POST",
      "HTTP_RESPONSE_GET",
      "HTTP_RESPONSE_POST"
   };
   
   rbtree_t *http_handlers;
   void sig_action(int);
   
   nhpc_status_t http_handler_register(const char *handler_string, fnc_ptr_nhpc_t handler_function)
   {
      fnc_ptr_nhpc_t *func_trigger_local = new fnc_ptr_nhpc_t;
      (*func_trigger_local) = handler_function;
      
      nhpc_status_t rv = http_handlers->insert(func_trigger_local, handler_string);
      
      return rv;      
   }
   
   void http_init()
   {
      LOG_INFO("Initialize http handler");
      http_handlers = new rbtree_t(NHPC_RBTREE_STR);
   }
   
   void http_init(nhpc_socket_t *sock)
   {
      http_data_t *http_data;
      nhpc_status_t nrv = read_headers(sock->headers, &http_data);
      http_data->sock = sock;
      if(nrv == NHPC_SUCCESS)
      {
	 LOG_INFO("HTTP Request type: "<<request_type_strings[http_data->request_type]);
	 http_request(http_data);
	 
	 delete_http_header(http_data);
	 delete http_data;
      }
   }
   
   void http_request(http_data_t *http_data)
   {
      nhpc_socket_t *sock = http_data->sock;
      
      if((http_data->request_type) == HTTP_REQUEST_GET || (http_data->request_type) == HTTP_REQUEST_POST)
      {
	 if(((http_data->request_type) == HTTP_REQUEST_POST) == NHPC_SUCCESS)
         {
            nhpc_json_t post_json;
            post_json.build_structure(sock->partial_content); 
            nhpc_json_t::key_pair_t *key_pair;
            int json_status;
            while(json_status = (post_json.search(&key_pair)) != JSON_END)
            {
                cout << key_pair->key << endl;
                cout << (char *)key_pair->value << endl;
            }

            cout << http_data->request_page << endl;

            http_data->custom_response_data = "{\"data\": {\"firefox\", \"nautilus\"}}"; 

	    //cout<<sock->partial_content<<endl;
         }
	 
	 string_t *tmp_str = nhpc_substr(http_data->request_page, '/');
	 char *app_name = tmp_str->strings[0];
	 
	 LOG_INFO("Checking for: " << app_name);
	 fnc_ptr_nhpc_t *func_trigger_local = (fnc_ptr_nhpc_t *)http_handlers->search(app_name);
	 if(func_trigger_local != NULL)
	 {
	    LOG_INFO("Found http handler: " << app_name);
	    nhpc_status_t nrv = (*func_trigger_local)(http_data);
	 }
	 
	 nhpc_string_delete(tmp_str);
	 
	 char *file_path = NULL;
	 file_path = nhpc_strconcat(HTTP_ROOT, http_data->request_page);
	 
	 nhpc_size_t file_size;
	 nhpc_status_t nrv = nhpc_file_size(file_path, &file_size);
	 
	 if(nrv == NHPC_FILE_NOT_FOUND)
	 {
	    const char *mssg = "HTTP/1.1 404 Content Not Found\r\n\r\nContent Not Found\r\n";
	    nhpc_size_t size = strlen(mssg);
	    socket_send(sock, (char *)mssg, &size);
	 }
	 else if(nrv == NHPC_DIRECTORY)
	 {
	    const char *mssg = "HTTP/1.1 404 Content Not Found\r\n\r\nServer Doesn't Know How To Handle Directory\r\n";
	    nhpc_size_t size = strlen(mssg);
	    socket_send(sock, (char *)mssg, &size);
	 }
	 else 
	 {
	    nhpc_headers_t *headers = new nhpc_headers_t;
	    headers->insert("HTTP/1.1 200 OK");

            if(!(http_data->custom_response_data))
            {
               cout << "waiting for file" << endl;
	       FILE *fp = fopen(file_path, "r");
	       char *file_size_str = nhpc_itostr(file_size);
	    
	       headers->insert("Content-Length", file_size_str);

	       /* Deciding mime types */
	       if(nhpc_strcmp(file_path, "*.json"))
	          headers->insert("Content-Type: application/json");
	       else if(nhpc_strcmp(file_path, "*.js"))
	          headers->insert("Content-Type: application/javascript");
	       else if(nhpc_strcmp(file_path, "*.css"))
	          headers->insert("Content-Type: text/css");
	       	    
	    
	       headers->write(sock);
	    
	       delete headers;
	       nhpc_string_delete(file_size_str);
	    
	       nhpc_status_t nrv;
	    
	       char buffer[10000];	    
	       nhpc_size_t len;
	    
	       do
	       {
	          bzero(buffer, sizeof(buffer));
	          len = fread(buffer, 1, sizeof(buffer), fp);
	       
	          nrv = socket_sendmsg(sock, buffer, &len);	
	       }while(!feof(fp) && errno != EPIPE);
	    
	       fclose(fp);
            }
            else
            {
               nhpc_size_t len = strlen(http_data->custom_response_data);
	       headers->insert("Content-Length", nhpc_itostr(len));
	       headers->insert("Connection: Keep-Alive");
	       headers->insert("Content-Type: application/json");
	       headers->insert("Keep-Alive: timeout=5, max=100");
	       headers->write(sock);
	       delete headers;
               socket_sendmsg(sock, http_data->custom_response_data, &len);
            }
	 }
	 
	 nhpc_string_delete(file_path);	 
      }
      else if((http_data->request_type) == HTTP_INVALID)
      {
	 const char *mssg = "HTTP/1.1 403 Invalid Request\r\n\r\nInvalid request\r\n";
	 nhpc_size_t size = strlen(mssg);
	 socket_send(sock, (char *)mssg, &size);	 
      }
   }
   
   nhpc_status_t http_get_file(const char **file_path, nhpc_socket_t *sock, const char *target_file, const char *host_addr)
   {
      nhpc_create_tmp_file_or_dir(file_path, "/tmp/neweraHPC", NHPC_FILE);
      
      const char *command = nhpc_strconcat("GET /", target_file, " HTTP/1.1");
      nhpc_headers_t *headers = new nhpc_headers_t;
      headers->insert(command);
      headers->insert("User-Agent: neweraHPC");
      headers->insert("Host", host_addr);
      headers->write(sock);
      delete headers;
      nhpc_string_delete((char *)command);
      
      FILE *fp = fopen(*file_path, "w+");
      nhpc_status_t nrv;
      nhpc_size_t size;
      nhpc_size_t size_downloaded = 0;
      nhpc_size_t file_size;
      
      char buffer[10000];
      nhpc_size_t header_size = 0;
      
      do 
      {
	 bzero(buffer, sizeof(buffer));
	 size = sizeof(buffer);
	 header_size = 0;
	 
	 do 
	 {
	    nrv = socket_recv(sock, buffer, &size);
	 }while(nrv != NHPC_SUCCESS && nrv != NHPC_EOF);
	 
	 if(sock->have_headers == false)
	 {
	    nrv = nhpc_analyze_stream(sock, buffer, &size, &header_size);
	    if(nrv == NHPC_SUCCESS)
	    {
	       http_content_length(sock->headers, &file_size);
	       nhpc_display_headers(sock);
	    }
	 }
	 
	 fwrite((buffer + header_size), 1, (size - header_size), fp); 
	 
	 size_downloaded += (size - header_size);

      }while(nrv != NHPC_EOF && size_downloaded != file_size);
      
      cout<<"Size Downloaded: "<<size_downloaded<<endl;
      
      fclose(fp);
      
      return nrv;
   }
   
   void http_response(nhpc_socket_t *sock)
   {
      
   }
};
