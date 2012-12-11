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
#include <iomanip>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/neweraHPC.h>
#include <include/grid.h>
#include <include/network.h>
#include <include/thread.h>
#include <include/file.h>

using namespace std;

namespace neweraHPC
{
   nhpc_grid_server_t *grid_server;
   
   nhpc_grid_server_t::nhpc_grid_server_t(const char *in_host, const char *in_cpu_time) 
   : network_t(&thread_manager), plugin_manager_t(&thread_manager), grid_scheduler_t(&thread_manager)
   {
      clients = new rbtree_t(NHPC_RBTREE_STR);
      thread_manager = new thread_manager_t;
      
      string_t *string = nhpc_substr(in_host, ':');
      nhpc_strcpy(&host_addr, string->strings[0]);
      
      if(string->count == 1)
      {
	 nhpc_strcpy(&host_port, "8080");
      }
      else 
	 nhpc_strcpy(&host_port, string->strings[1]);  
      
      if(in_cpu_time)
      {
	 host_cpu_time = nhpc_strtoi(in_cpu_time);
	 if(host_cpu_time > 100)
	    host_cpu_time = 100;
      }
      else 
	 host_cpu_time = 0;
      
      host_cores = sysconf(_SC_NPROCESSORS_CONF);
      
      nhpc_string_delete(string);
   }
   
   nhpc_grid_server_t::nhpc_grid_server_t() 
   : network_t(&thread_manager), plugin_manager_t(&thread_manager), grid_scheduler_t(&thread_manager)
   {
      clients = new rbtree_t(NHPC_RBTREE_STR);
      thread_manager = new thread_manager_t;
    
      host_cpu_time = 0;
      host_cores = sysconf(_SC_NPROCESSORS_CONF);
      
      host_port = NULL;
      host_addr = NULL;
   }      
   
   void print_help()
   {
      cout<<"Usage: server \t[-l host_ip:port] [-r remote_ip:port] \n\t\t[-c cpu_time] [-d daemon]"<<endl;
      cout<<"Options:"<<endl;
      cout<<setw(20)<<"-l host_ip:port"<<setw(50)<<":Ip address and port of local server"<<endl;
      cout<<setw(22)<<"-r remote_ip:port"<<setw(46)<<":Ip address and port of controller"<<endl;
      cout<<setw(16)<<"-c cpu_time"<<setw(38)<<":Mac cpu time to use"<<endl;
      cout<<setw(14)<<"-d daemon"<<setw(32)<<":Daemon mode"<<endl;
      cout<<setw(15)<<"-v verbose"<<setw(50)<<":Verbose mode: info, debug, all"<<endl;
      cout<<setw(12)<<"-h help"<<setw(37)<<":This help menu"<<endl;
      
      exit(0);
   }   
   
   nhpc_status_t nhpc_grid_server_t::grid_server_init()
   {
      nhpc_status_t nrv;
      
      char *host = NULL;
      char *controller = NULL;
      char *cpu_time = NULL;
      bool daemon = false;      
      
      key_pair_t *key_pair;
      
      for(int i = 1; i <= cmdline_arguments.ret_count(); i++)
      {
	 key_pair = (key_pair_t *)cmdline_arguments.search_str(i);
	 
	 switch(*(key_pair->key))
	 {
	    case 'c':
	       cpu_time = (char *)key_pair->data;
	       break;
	    case 'l':
	       host = (char *)key_pair->data;
	       break;
	    case 'r':
	       controller = (char *)key_pair->data;
	       break;
	    case 'h':
	       print_help();
	       break;
	    case 'd':
	       daemon = true;
	       break;
	    default:
	       print_help();
	 }	 
      }
      
      if(host)
      {
	 string_t *string = nhpc_substr(host, ':');
	 nhpc_strcpy(&host_addr, string->strings[0]);
	 
	 if(string->count == 1)
	 {
	    nhpc_strcpy(&host_port, "8080");
	 }
	 else 
	    nhpc_strcpy(&host_port, string->strings[1]);	 
	 
	 nhpc_string_delete(string);
      }
      else 
      {
	 LOG_ERROR("Host not specified");
	 print_help();
      }
	 
      if(controller)
      {
	 string_t *string = nhpc_substr(controller, ':');

	 const char *controller_addr = (const char *)(string->strings[0]);
	 const char *controller_port;
	 
	 if(string->count == 1)
	    controller_port = "8080";
	 else 
	    controller_port = string->strings[1];
	 
	 nhpc_strcpy(&grid_controller_addr, controller_addr);
	 nhpc_strcpy(&grid_controller_port, controller_port);
	 
	 LOG_INFO("Registering to controller: "<<grid_controller_addr<<":"<<grid_controller_port);
	 
	 nhpc_status_t nrv = nhpc_register_to_controller(grid_controller_addr, grid_controller_port, host_addr, host_port, 
							 host_cores, host_cpu_time);	 
	 if(nrv == NHPC_FAIL)
	    LOG_ERROR("Registration to the controller failed\n Running without controller");

	 nhpc_string_delete(string);
      }
      if(cpu_time)
      {
	 host_cpu_time = nhpc_strtoi(cpu_time);
	 if(host_cpu_time > 100)
	    host_cpu_time = 100;
      }
      else 
	 host_cpu_time = 0;      
      
      if(daemon)
      {
	 int rv;
	 
	 int pid = fork();
	 if(pid != 0)
	    exit(0);
	 setsid();
	 
	 int i;
	 
	 for (i = getdtablesize(); i >= 0; --i)
	    close(i);
	 
	 i = open("/dev/null", O_RDWR);
	 rv = dup(i);
	 rv = dup(i);
      }  
      
      jobs = new rbtree_t(NHPC_RBTREE_STR);
      
      nhpc_system.init_system();
      plugin_manager_init();
      grid_scheduler_init();
            
      mkdir("/tmp/neweraHPC", 0777);
      
      fnc_ptr_t *grid_request_handler = (fnc_ptr_t *)nhpc_grid_server_t::grid_request_init;
      int rv = network_addons->insert((void *)&grid_request_handler, "GRID");

      add_peer(host_addr, host_port, host_cores, host_cpu_time);

      grid_server = this;

      nrv = create_server(host_addr, host_port, AF_INET, SOCK_STREAM, 0);  
      if(nrv != NHPC_SUCCESS)
	 return nrv;
   }
   
   nhpc_grid_server_t::~nhpc_grid_server_t()
   {
      LOG_INFO("Shuting Down Grid Server");
      

      nhpc_string_delete(host_addr);
      nhpc_string_delete(host_port);
      
      char *tmp_host_addr;
      const char *key_str;
      LOG_INFO("Deleting grid client details");
      while(1)
      {
	 tmp_host_addr = (char *)(*clients).search_first(&key_str);
	 
	 if(!tmp_host_addr)
	    break;
	 
	 nhpc_string_delete(tmp_host_addr);
	 (*clients).erase(key_str);
      }
      
      delete clients;
   }
   
   void nhpc_grid_server_t::grid_request_init(nhpc_socket_t *sock)
   {
      char *command = (char *)sock->headers->search("command");
      string_t *string = nhpc_substr(command, ' ');
      
      if(string->count < 3)
      {
	 return;
      }
      
      nhpc_status_t nrv = NHPC_FAIL;
      const char *fnc_str = string->strings[1];
      
      if(nhpc_strcmp(fnc_str, "CLIENT_REGISTRATION") == NHPC_SUCCESS)
	 nrv = (*grid_server).grid_client_registration(sock);
      else if(nhpc_strcmp(fnc_str, "NODE_REGISTRATION") == NHPC_SUCCESS)
	 nrv = (*grid_server).grid_node_registration(sock);
      else 
      {
	 const char *uid = (char *)sock->headers->search("Grid-Uid");
	 
	 if(uid == NULL || grid_server->grid_client_verify_uid(uid) != NHPC_SUCCESS)
	 {
	    nhpc_string_delete(string);
	    return;
	 }
	 
	 else if(nhpc_strcmp(fnc_str, "FILE_EXCHANGE") == NHPC_SUCCESS)
	    nrv = grid_server->grid_file_download(sock, &uid);
	 else if(nhpc_strcmp(fnc_str, "INSTRUCTION") == NHPC_SUCCESS)
	 {
	    LOG_DEBUG("My Max Cpu time: " << grid_server->host_cpu_time);
	    
	    nhpc_instruction_set_t *instruction_set;
	    nrv = nhpc_generate_instruction(&instruction_set, sock->headers);
	    nrv = grid_server->grid_execute(instruction_set, sock, &uid);
	 }
	 else if(nhpc_strcmp(fnc_str, "SUBMISSION") == NHPC_SUCCESS)
	 {
	    char *peer_id = (char *)sock->headers->search("Peer");
	    int peer_id_n = nhpc_strtoi(peer_id);
	    
	    if(sock->partial_content)
	    {
	       task_t *task = (task_t *)sock->partial_content;
	       nhpc_systeminfo_t *systeminfo = &(task->systeminfo);
	       
	       LOG_DEBUG("Node:load_avg_max: " << systeminfo->cpuinfo.load_avg_max);
	       LOG_DEBUG("Node:meminfo: " << systeminfo->meminfo.free_mem);
	       
	       grid_server->free_peer(nhpc_strtoi(peer_id), systeminfo->cpuinfo.load_avg_max, &(systeminfo->meminfo));
	    }
	    else 
	       grid_server->free_peer(nhpc_strtoi(peer_id));
	    
	    nrv = NHPC_SUCCESS;
	 }
	 
	 char *response = nhpc_itostr(nrv);
	 nhpc_size_t size = strlen(response);
	 socket_sendmsg(sock, response, &size);
	 nhpc_string_delete(response);
      }
      
      LOG_INFO("Executed function with status: " << nrv);
      
      nhpc_string_delete(string);
   }
   
   nhpc_status_t nhpc_grid_server_t::grid_client_registration(nhpc_socket_t *sock)
   {
      nhpc_status_t nrv;
      nrv = NHPC_FAIL;
      
      char *uid;
      nrv = grid_client_gen_uid(sock->host, (const char **)&uid);
      
      nhpc_size_t size = strlen(uid);
      nrv = socket_sendmsg(sock, uid, &size);
      
      char *dir = nhpc_strconcat(grid_directory, uid);
      mkdir(dir, 0777);
      nhpc_string_delete(dir);
      nhpc_string_delete(uid);
      
      return nrv;      
   }
   
   nhpc_status_t nhpc_grid_server_t::grid_node_registration(nhpc_socket_t *sock)
   {
      rbtree_t *headers = sock->headers;
      char *node_addr = (char *)headers->search("Node-Addr");
      char *node_port = (char *)headers->search("Node-Port");
      char *node_cores = (char *)headers->search("Node-Cores");
      char *node_cpu_time = (char *)headers->search("Node-Cpu-Time");
      
      if(!node_addr || !node_port || !node_cores || !node_cpu_time)
	 return NHPC_FAIL;
      
      add_peer(sock->host, node_port, nhpc_strtoi(node_cores), nhpc_strtoi(node_cpu_time));
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_grid_server_t::grid_client_gen_uid(const char *client_addr, const char **uid)
   {
      char *tmp_client_addr;
      char *random_string = NULL;
      
      do 
      {
	 if(random_string != NULL)
	    nhpc_string_delete(random_string);
	 
	 random_string = nhpc_random_string(8);
	 
	 tmp_client_addr = (char *)clients->search(random_string);
      }while(tmp_client_addr != NULL);

      *uid = random_string;
      nhpc_strcpy(&tmp_client_addr, client_addr);
      clients->insert((void *)tmp_client_addr, random_string);
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_grid_server_t::grid_client_verify_uid(const char *uid)
   {      
      if(clients->search(uid) == NULL)
	 return NHPC_FAIL;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_grid_server_t::grid_file_download(nhpc_socket_t *sock, const char **grid_uid)
   {
      char *file_name     = (char *)sock->headers->search("File-Name");
      char *file_type     = (char *)sock->headers->search("File-Type");
      char *file_size_str = (char *)sock->headers->search("Content-Length");

      if(!file_name || !file_type || !file_size_str)
	 return NHPC_FAIL;
      
      nhpc_size_t file_size = nhpc_strtoi(file_size_str);
      
      nhpc_size_t size_downloaded = 0;
      char buffer[10000];
      nhpc_status_t nrv;
      nhpc_size_t size;
      
      const char *grid_path = nhpc_strconcat("/www/grid/", *grid_uid);
      const char *tmp_path = nhpc_strconcat(grid_path, "/");
      const char *final_path = nhpc_strconcat(tmp_path, file_name);

      nhpc_string_delete((char *)grid_path);
      nhpc_string_delete((char *)tmp_path);
      
      int fd = -1;
      int retry_count = 0;
      do 
      {
	 fd = open (final_path, O_WRONLY | O_CREAT | O_NONBLOCK | O_NOCTTY,
		    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	 retry_count++;
      }while(fd == -1 && retry_count < 5);
      
      if(fd == -1 && close(fd) < 0)
      {
	 nhpc_string_delete((char *)final_path);
	 return NHPC_FAIL;
      }
      
      FILE *fp = fopen(final_path, "wb");

      if(sock->partial_content != NULL)
      {
	 fwrite(sock->partial_content, 1, sock->partial_content_len, fp);
	 size_downloaded += sock->partial_content_len;
      }
      
      do 
      {
	 bzero(buffer, sizeof(buffer));
	 size = sizeof(buffer);
	 nrv = socket_recv(sock, buffer, &size); 
	 fwrite(buffer, 1, size, fp);	 
	 size_downloaded += size;
      }while((nrv != NHPC_EOF) && file_size != size_downloaded);
      
      fclose(fp);
      
      if(nhpc_strcmp(file_type, "plugin") == NHPC_SUCCESS)
      {
	 nrv = install_plugin(final_path, *grid_uid);
	 if(nrv != NHPC_SUCCESS)
	 {
	    nhpc_string_delete((char *)final_path);
	    return NHPC_FAIL;
	 }
      }
      
      nhpc_string_delete((char *)final_path);
      
      return NHPC_SUCCESS;
   }
};
