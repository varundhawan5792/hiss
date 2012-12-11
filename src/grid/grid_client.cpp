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
#include <include/file.h>
#include <include/headers.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t nhpc_register_to_server(const char **grid_uid, const char *host_addr, const char *host_port)
   {
      nhpc_socket_t *sock;
      
      nhpc_status_t nrv = socket_connect(&sock, host_addr, host_port, AF_INET, SOCK_STREAM, 0);
      
      if(nrv != NHPC_SUCCESS)
      {	 
	 return errno;
      }
      
      const char *mssg = "GRID CLIENT_REGISTRATION 2.90\r\n\r\n";
      size_t size = strlen(mssg);
      nrv = socket_sendmsg(sock, (char *)mssg, &size);
      if(nrv == NHPC_FAIL)
      {
	 socket_close(sock);
	 socket_delete(sock);
	 
	 return NHPC_FAIL;
      }
      
      size = 1000;
      char buffer[1000];
      
      nrv = 0;
      
      nrv = socket_recv(sock, buffer, &size);
      if(nrv == NHPC_SUCCESS)
      {
	 nhpc_strcpy((char **)grid_uid, buffer);
      }
      else 
	 nrv = NHPC_FAIL;
      
      socket_close(sock);
      socket_delete(sock);
      
      return nrv;
   }
   
   nhpc_status_t nhpc_send_file(const char *grid_uid, const char *host_addr, 
				const char *host_port, const char *file_path)
   {
      nhpc_socket_t *sock;
      
      nhpc_status_t nrv;
      
      nrv = nhpc_fileordirectory(file_path);
      if(nrv != NHPC_FILE)
      {
	 return NHPC_FAIL;
      }
      
      const char *file_type;
      if((nhpc_strcmp(file_path, "*.so") == NHPC_SUCCESS) || (nhpc_strcmp(file_path, "*.nxi") == NHPC_SUCCESS))
	 file_type = "plugin";
      else 
	 file_type = "regular";
      
      nrv = socket_connect(&sock, host_addr, host_port, AF_INET, SOCK_STREAM, 0);
      nhpc_size_t size;
      
      if(nrv != NHPC_SUCCESS)
      {
	 return NHPC_FAIL;
      }      
      
      string_t *string = nhpc_substr(file_path, '/');
      nhpc_size_t file_len;
      nhpc_file_size(file_path, &file_len);
      
      char *content_len_str = nhpc_itostr(file_len);
      nhpc_headers_t *headers = new nhpc_headers_t;
      headers->insert("GRID FILE_EXCHANGE 2.90");
      headers->insert("Grid-Uid", grid_uid);
      headers->insert("File-Type", file_type);
      headers->insert("File-Name", string->strings[string->count - 1]);
      headers->insert("Content-Length", content_len_str);
      nrv = headers->write(sock);
      
      nhpc_string_delete(content_len_str);
      delete headers;
      
      nhpc_string_delete(string);
      
      if(nrv != NHPC_SUCCESS)
      {
	 return NHPC_FAIL;
      }
      
      FILE *fp = fopen(file_path, "r");
      nhpc_size_t len;
      char buffer[10000];
      
      int bytes_writen = 0;
      
      do
      {
	 len = fread(buffer, 1, sizeof(buffer), fp);
	 nrv = socket_sendmsg(sock, buffer, &len);
	 bytes_writen += len;
      }while(nrv != EPIPE && !feof(fp));      
      
      fclose(fp);
      
      bzero(buffer, 1000);
      len = 5;
      do 
      {
	 nrv = socket_recv(sock, buffer, &len);
      }while(nrv != NHPC_EOF);
      
      socket_close(sock);
      socket_delete(sock);
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_send_instruction(const char *grid_uid, const char *host_addr, const char *host_port, 
				       nhpc_instruction_set_t *instruction_set, const char *extra_param1,
				       const char *extra_param2, const char *extra_param3)
   {
      nhpc_socket_t *sock;
      
      nhpc_status_t nrv = socket_connect(&sock, host_addr, host_port, AF_INET, SOCK_STREAM, 0);
      
      if(nrv != NHPC_SUCCESS)
      {
	 return NHPC_FAIL;
      }
      
      nhpc_headers_t *headers = new nhpc_headers_t;
      int argument_count = instruction_set->arguments->ret_count();
      
      char *argument_count_str = nhpc_itostr(argument_count);
      headers->insert("GRID INSTRUCTION 2.90");
      headers->insert("Grid-Uid", grid_uid);
      headers->insert("Plugin", instruction_set->plugin_name);
      headers->insert("Peer-Host", host_addr);
      headers->insert("Peer-Port", host_port);
      headers->insert("Argument-Count", argument_count_str);
      if(extra_param1)
	 headers->insert(extra_param1);
      if(extra_param2)
	 headers->insert(extra_param2);
      if(extra_param3)
	 headers->insert(extra_param3);
      
      for(int i = 1; i <= argument_count; i++)
      {
	 char *i_str = nhpc_itostr(i);
	 char *tmp_str = nhpc_strconcat("Argument", i_str);
	 char *argument = (char *)instruction_set->arguments->search(i);
	 nhpc_string_delete(i_str);
	 
	 headers->insert(tmp_str, argument);
	 nhpc_string_delete(tmp_str);
      }
      
      headers->write(sock);
      
      nhpc_string_delete(argument_count_str);
      delete headers;
      
      char buffer[5];
      bzero(buffer, 5);
      nhpc_size_t len = 5;
      do 
      {
	 nrv = socket_recv(sock, buffer, &len);
      }while(nrv != NHPC_SUCCESS && nrv != NHPC_EOF);
      nrv = nhpc_strtoi(buffer);
      
      socket_close(sock);
      socket_delete(sock);
      
      return NHPC_SUCCESS;      
   }
   
   nhpc_status_t nhpc_send_general_instruction(nhpc_instruction_set_t *instruction_set, const char *host_addr,
					       const char *host_port)
   {
      char *grid_uid = instruction_set->grid_uid;
      char *plugin_name = instruction_set->plugin_name;
      char *host_grid_uid = instruction_set->host_grid_uid;
      int argument_count = instruction_set->arguments->ret_count();
      bool *execute = &(instruction_set->execute);
      char *host_peer_addr = instruction_set->host_peer_addr;
      char *host_peer_port = instruction_set->host_peer_port;

      char *peer_id_str = nhpc_itostr(instruction_set->host_peer_id);
      char *argument_count_str = nhpc_itostr(instruction_set->arguments->ret_count());
      
      nhpc_socket_t *sock;
      
      nhpc_status_t nrv = socket_connect(&sock, host_addr, host_port, AF_INET, SOCK_STREAM, 0);
      
      if(nrv != NHPC_SUCCESS)
      {
	 nhpc_string_delete(argument_count_str);
	 nhpc_string_delete(peer_id_str);

	 return NHPC_FAIL;
      }      
      
      nhpc_headers_t *headers = new nhpc_headers_t;
      headers->insert("GRID INSTRUCTION 2.90");
      headers->insert("Plugin", instruction_set->plugin_name);
      headers->insert("Peer", peer_id_str);
      headers->insert("Peer-Host", host_peer_addr);
      headers->insert("Peer-Port", host_peer_port);
      headers->insert("Argument-Count", argument_count_str);
      if(grid_uid)
	 headers->insert("Grid-Uid", grid_uid);
      if(host_grid_uid)
	 headers->insert("Host-Grid-Uid", host_grid_uid);
      if(*execute)
	 headers->insert("Execution-State: Ready");
      
      for(int i = 1; i <= argument_count; i++)
      {
	 char *i_str = nhpc_itostr(i);
	 char *tmp_str = nhpc_strconcat("Argument", i_str);
	 char *argument = (char *)instruction_set->arguments->search(i);
	 nhpc_string_delete(i_str);
	 
	 headers->insert(tmp_str, argument);
	 nhpc_string_delete(tmp_str);
      }
      
      headers->write(sock);
      
      nhpc_string_delete(argument_count_str);
      nhpc_string_delete(peer_id_str);
      delete headers; 
      
      socket_close(sock);
      socket_delete(sock);
      
      return NHPC_SUCCESS;
   }
};
