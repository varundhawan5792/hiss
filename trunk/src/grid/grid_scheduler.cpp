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
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/grid.h>
#include <include/network.h>
#include <include/grid_client.h>
#include <include/error.h>

using namespace std;

namespace neweraHPC
{	
   grid_scheduler_t *scheduler;
   pthread_mutex_t mutex_handler;
   
   grid_scheduler_t::grid_scheduler_t(thread_manager_t **_thread_manager)
   {
      peers = new rbtree_t;
      jobs = new rbtree_t(NHPC_RBTREE_STR);
      queued_instructions = new rbtree_t;
      child_processes = new rbtree_t;
      
      mutex = new pthread_mutex_t;
      mutex_child_processes = new pthread_mutex_t;
      pthread_mutex_init(mutex, NULL);
      pthread_mutex_init(mutex_child_processes, NULL);      
      
      thread_manager = _thread_manager;
      scheduler = this;
   }
   
   grid_scheduler_t::~grid_scheduler_t()
   {
      LOG_INFO("Shuting down grid scheduler");
      
      peer_details_t *peer;
      int key;
      while(1)
      {
	 lock();
	 peer = (peer_details_t *)(*peers).search_first(&key);
	 unlock();
	 
	 if(!peer)
	    break;
	 
	 remove_peer(key);
      }
      delete peers;
      
      delete jobs;
      delete child_processes;
      
      nhpc_instruction_set_t *instruction_set;
      while(1)
      {
	 lock();
	 instruction_set = (nhpc_instruction_set_t *)(*queued_instructions).search_first(&key);
	 unlock();
	 
	 if(!instruction_set)
	    break;
	 
	 lock();
	 (*queued_instructions).erase(key);
	 nhpc_delete_instruction(instruction_set);
	 unlock();
      }
      
      delete mutex;
      delete mutex_child_processes;
   }
   
   void grid_scheduler_t::grid_scheduler_init()
   {
      extern_system = &(nhpc_system);
      
      pthread_mutex_init(&mutex_handler, NULL);
      
      int thread_id;
      (*thread_manager)->init_thread(&thread_id, NULL);
      (*thread_manager)->create_thread(&thread_id, NULL, (void* (*)(void*))grid_scheduler_t::monitor_jobs_pending, 
				       this, NHPC_THREAD_DEFAULT);
      
      thread_id;
      (*thread_manager)->init_thread(&thread_id, NULL);
      (*thread_manager)->create_thread(&thread_id, NULL, (void* (*)(void*))grid_scheduler_t::refresh_node_status, 
				       this, NHPC_THREAD_DEFAULT);

      nhpc_system.register_trigger_child_process((char *)"GRID", (fnc_ptr_int_t)child_exit_trigger);
   }
   
   int grid_scheduler_t::cores()
   {
      lock();
      int count = peers->ret_count();
      int cores = 0;
      for(int i = 1; i <= count; i++)
      {
         peer_details_t *peer_details = (peer_details_t *)(*peers)[i];
         cores+= (peer_details->processors - peer_details->weight);
      }
      unlock();

      return cores;
   }

   void grid_scheduler_t::add_peer(const char *host, const char *port, int processors, int cpu_time)
   {      
      peer_details_t *peer_details = new peer_details_t;
      nhpc_strcpy(&(peer_details->host), host);
      nhpc_strcpy(&(peer_details->port), port);
      peer_details->processors = processors;
      peer_details->weight = 0;
      peer_details->processor_time = cpu_time;
      peer_details->threads = processors * cpu_time / 100;
      
      peer_details->id = peers->insert(peer_details);
   }
   
   void grid_scheduler_t::remove_peer(int peer_id)
   {
      lock();
      peer_details_t* peer_details = (peer_details_t *)peers->search(peer_id);
      peers->erase(peer_id);
      unlock();
      if(peer_details)
      {
	 nhpc_string_delete((peer_details->host));
	 nhpc_string_delete((peer_details->port));
	 delete peer_details;
      }
   }
   
   nhpc_status_t grid_scheduler_t::increase_thread(int peer_id)
   {
      peer_details_t *peer_details = (peer_details_t *)peers->search(peer_id);
      peer_details->threads += 1;
   }
   
   nhpc_status_t grid_scheduler_t::decrease_thread(int peer_id)
   {
      peer_details_t *peer_details = (peer_details_t *)peers->search(peer_id);
      peer_details->threads = peer_details->processors;
   }
   
   peer_details_t *grid_scheduler_t::schedule()
   {
      peer_details_t *peer_details = NULL;
      
      int count = peers->ret_count();
      
      for(int i = 1; i <= count; i++)
      {
	 peer_details = (peer_details_t *)peers->search(i);	 
	 
	 if(peer_details->weight < peer_details->threads)
	 {
	    lock();
	    peer_details->weight++;
	    unlock();
	    return peer_details;
	 }
      }
      
      return NULL;
   }
   
   nhpc_status_t grid_scheduler_t::queue_job(nhpc_instruction_set_t *instruction_set)
   {
      lock();
      (*queued_instructions).insert(instruction_set);
      unlock();
      
      push_jobs();
   }
   
   nhpc_status_t grid_scheduler_t::dispatch_job(nhpc_instruction_set_t *instruction_set)
   {
      peer_details_t *peer_details;
      nhpc_status_t nrv;
      
      if(!(instruction_set->host_grid_uid))
      {	 
	 return NHPC_FAIL;
      }
      
      peer_details = schedule();
      if(!peer_details)
      {
	 lock();
	 queued_instructions->insert(instruction_set);
	 unlock();	 
	 return NHPC_FAIL;
      }
      
      scheduler_thread_data_t *thread_data = new scheduler_thread_data_t;
      thread_data->instruction_set = instruction_set;
      thread_data->scheduler = this;
      thread_data->thread_manager = *thread_manager;
      thread_data->peer_details = peer_details;
      
      (**thread_manager).init_thread(&(thread_data->thread_id), NULL);
      nrv = (**thread_manager).create_thread(&(thread_data->thread_id), NULL, (void* (*)(void*))job_dispatcher, 
					     thread_data, NHPC_THREAD_DETACH);
            
      return nrv;
   }
   
   void *grid_scheduler_t::job_dispatcher(scheduler_thread_data_t *data)
   {
      nhpc_instruction_set_t *instruction_set = data->instruction_set;
      grid_scheduler_t *scheduler = data->scheduler;
      thread_manager_t *thread_manager = data->thread_manager;
      peer_details_t *peer_details = data->peer_details;      
      
      nhpc_status_t nrv = NHPC_SUCCESS;
      
      const char *host_addr;
      const char *host_port;
      const char *grid_uid;
      const char *host_grid_uid = (const char *)instruction_set->host_grid_uid;
      const char *base_dir = nhpc_strconcat("/www/grid/", host_grid_uid, "/");
      
      int argument_count = 0;
            
      if(!peer_details)
      {
	 nrv = NHPC_FAIL;
	 goto exit_thread;
      }
      
      host_addr = (const char *)peer_details->host;
      host_port = (const char *)peer_details->port;
      
      nrv = nhpc_register_to_server(&grid_uid, peer_details->host, peer_details->port);
      if(nrv != NHPC_SUCCESS)
      {
	 (*scheduler).free_peer(peer_details->id);
	 //(*scheduler).remove_peer(peer_details->id);
	 
	 nrv = NHPC_FAIL;
	 goto exit_thread;
      }
       
      argument_count = instruction_set->arguments->ret_count();
      for(int i = 1; i < argument_count; i++)
      {
	 char *argument = (char *)instruction_set->arguments->search(i);
	 char *grid_file_str = nhpc_itostr(GRID_FILE);
	 char *search_value = nhpc_strconcat(grid_file_str, "*");
	 nhpc_string_delete(grid_file_str);
	 if(nhpc_strcmp(argument, search_value) == NHPC_SUCCESS)
	 {
	    string_t *string = nhpc_substr(argument, ',');
	    char *file_path = nhpc_strconcat(base_dir, string->strings[1]);
	    nhpc_string_delete((char *)base_dir);
	    base_dir = NULL;
	    
	    nrv = nhpc_send_file(grid_uid, host_addr, host_port, file_path);
	    
	    nhpc_string_delete(file_path);
	    nhpc_string_delete(string);
	    
	    if(nrv != NHPC_SUCCESS)
	    {  
	       (*scheduler).free_peer(peer_details->id);

	       nrv = NHPC_FAIL;
	       goto exit_thread;
	    }
	 }
	 
	 nhpc_string_delete(search_value);
      }
      
      instruction_set->host_peer_id = peer_details->id;
      if(instruction_set->grid_uid)
	 nhpc_string_delete(instruction_set->grid_uid);
      nhpc_strcpy(&(instruction_set->grid_uid), grid_uid);
      
      nrv = nhpc_send_general_instruction(instruction_set, host_addr, host_port);
      
      nhpc_string_delete((char *)grid_uid);
      
   exit_thread:
      if(nrv != NHPC_SUCCESS)
      {
	 instruction_set->host_peer_id = 0;
	 (*scheduler).lock();
	 (*scheduler).queued_instructions->insert(instruction_set);
	 (*scheduler).unlock();
      }
      else 
	 nhpc_delete_instruction(instruction_set);   
      
      (*thread_manager).delete_thread_data(data->thread_id);
      delete data;
      
      if(base_dir)
	 nhpc_string_delete((char *)base_dir);
   }
   
   void grid_scheduler_t::free_peer(int id)
   {
      lock();
      
      peer_details_t *peer_details = (peer_details_t *)peers->search(id);
      if(peer_details)
      {
	 peer_details->weight--;
      }
      
      unlock();
      
      push_jobs();
   }
   
   void grid_scheduler_t::free_peer(int id, double loadavg, nhpc_meminfo_t *meminfo)
   {
      lock();
      
      int mem_free;
      
      if(meminfo->free_mem == 0)
      {
	 mem_free = 11;
      }
      else 
	 mem_free = meminfo->total_mem / meminfo->free_mem;
      
      peer_details_t *peer_details = (peer_details_t *)peers->search(id);
      if(peer_details)
      {
	 peer_details->weight--;
      }
      
      double processor_time = (loadavg * 100);
      
      cout<<"Debug: "<<processor_time<<endl;
      
      /*
      if(processor_time < (peer_details->processor_time * peer_details->processors) && mem_free <= 10)
      {
	 increase_thread(id); 
      }
      else 
      {
	 decrease_thread(id);
      }
      */
      
      unlock();
      
      push_jobs();
   }

   void grid_scheduler_t::lock_child_processes()
   {
      
   }
   
   void grid_scheduler_t::unlock_child_processes()
   {
      
   }
   
   void grid_scheduler_t::lock()
   {
      pthread_mutex_lock(mutex);
   }
   
   void grid_scheduler_t::unlock()
   {
      pthread_mutex_unlock(mutex);      
   }
   
   nhpc_status_t grid_scheduler_t::push_jobs()
   {
      nhpc_status_t nrv = NHPC_FAIL;
      int id;
      nhpc_instruction_set_t *instruction_set;
      
      while(1)
      {
	 lock();
	 instruction_set = (nhpc_instruction_set_t *)queued_instructions->search_first(&id);
	 unlock();

	 if(!instruction_set)
	 {
	    break;
	 }
	 
	 if(instruction_set && (instruction_set->host_grid_uid))
	 {
	    lock();
	    queued_instructions->erase(id);
	    unlock();
	    nrv = dispatch_job(instruction_set);
	    if(nrv != NHPC_SUCCESS)
	       break;
	 }
      }
      
      return nrv;
   }
   
   nhpc_status_t grid_scheduler_t::add_child_process(task_t *task, pid_t *pid)
   {
      pthread_mutex_lock(mutex_child_processes);
      (*child_processes).insert(task, *pid);
      pthread_mutex_unlock(mutex_child_processes);
   }
      
   void grid_scheduler_t::monitor_jobs_pending(grid_scheduler_t *grid_scheduler)
   {
      rbtree_t *queued_instructions = grid_scheduler->queued_instructions;
      rbtree_t *child_processes = grid_scheduler->child_processes;
      struct rusage r_usage;
      pid_t pid;
      
      while(1)
      {
	 int id;

	 grid_scheduler->lock();
	 nhpc_instruction_set_t *instruction_set = (nhpc_instruction_set_t *)queued_instructions->search_first(&id);
	 grid_scheduler->unlock();
	 
	 int core_count = grid_scheduler->cores();

	 if(instruction_set)
	 {
	    if(core_count > 0)
	    {
	       grid_scheduler->push_jobs();
	       LOG_DEBUG("Instruction Pending: "<<grid_scheduler->cores());
	    }
	 }
	 else 
	    LOG_INFO("No job pending available cores: "<<core_count);
	 
	 sleep(1);
      }
   }
   
   void *grid_scheduler_t::refresh_node_status(grid_scheduler_t *grid_scheduler)
   {
      while(1)
      {
	  
	 sleep(1);
      }
   }
   
   void *grid_scheduler_t::child_exit_trigger(int *pid)
   {
      int status = 0;
      
      cout<<"trigger executed pid "<<*pid<<endl;
      
      task_t *task = (task_t *)(*(scheduler->child_processes)).search(*pid);
      
      if(task == NULL)
	 return NHPC_FAIL;
      
      nhpc_instruction_set_t *instruction_set = task->instruction_set;
      system_systeminfo(&(task->systeminfo));
      
      if(!instruction_set)
	 return NULL;
	 
      if(status != 0)
      {
	 scheduler->free_peer(instruction_set->host_peer_id);
	 scheduler->queue_job(instruction_set);
	 return NULL;
      }
      
      nhpc_status_t nrv;
      
      char *peer_host = instruction_set->host_peer_addr;
      char *peer_port = instruction_set->host_peer_port;
      char *peer_id   = nhpc_itostr(instruction_set->host_peer_id);
      char *host_uid  = instruction_set->host_grid_uid;
      
      nhpc_socket_t *sock;
      nrv = socket_connect(&sock, peer_host, peer_port, AF_INET, SOCK_STREAM, 0);
      
      if(nrv != NHPC_SUCCESS)
      {
	 nhpc_string_delete(peer_id);
	 
	 return NULL;
      }
      
      nhpc_headers_t *headers = new nhpc_headers_t;
      
      headers->insert("GRID SUBMISSION 2.90");
      headers->insert("Grid-Uid", host_uid);
      headers->insert("Peer", peer_id);
      nrv = headers->write(sock);	       
      delete headers;
      
      nhpc_string_delete(peer_id);
      
      nhpc_size_t size = sizeof(task_t);
      socket_sendmsg(sock, (const char *)(task), &size);
      
      socket_close(sock);
      socket_delete(sock);
      
      nhpc_delete_instruction(instruction_set);
      delete task;
      
      pthread_mutex_lock(scheduler->mutex_child_processes);
      scheduler->child_processes->erase(*pid);
      pthread_mutex_unlock(scheduler->mutex_child_processes);	       
   }   
};
