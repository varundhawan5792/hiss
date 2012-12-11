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
#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/alloc.h>
#include <include/system.h>
#include <include/error.h>
#include <include/strings_pool.h>

using namespace std;

namespace neweraHPC
{   
   void child_prepare()
   {
      system_prefork_routine();
      sleep(2);
   }
   
   nhpc_status_t nhpc_system_t::register_trigger_child_process(char *service_name, fnc_ptr_int_t function)
   {
      thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);
      child_handlers->insert((void *)function, service_name);
      thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);
   }
   
   nhpc_status_t nhpc_system_t::create_child_process(char *service_name, int *pid)
   {
      void *check_handler;
      
      thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_READ);
      check_handler = child_handlers->search(service_name);
      thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_READ);
      
      if(!check_handler)
      {
	 exit(0);
	 return NHPC_FAIL;
      }
      
      child_process_t *child_process = new child_process_t;
      nhpc_strcpy(&(child_process->service_name), service_name);
      child_process->active = true;
      
      *pid = fork();
      if(*pid < 0)
      {
	 return NHPC_FAIL;
	 nhpc_string_delete(child_process->service_name);
	 delete child_process;
      }
      else if(*pid == 0)
      {
	 //system_prefork_routine();
	 //sleep(1);
      }
      else 
      {
	 child_process->pid = *pid;	 
	 
	 thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);
	 child_processes->insert(child_process, *pid);
	 thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);
	 
	 return NHPC_SUCCESS;
      }
   }
   
   nhpc_status_t nhpc_system_t::free_child_process()
   {
      pid_t pid;
      int status;
      
      while(1)
      {
	 pid = waitpid(0, &status, WUNTRACED);
	 if(pid == -1)
	    break;
	 
	 thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_READ);
	 child_process_t *child_process = (child_process_t *)child_processes->search(pid);
	 thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_READ);
	 
	 if(!child_process)
	    continue;
	 else 
	 {
	    thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);
	    child_process->active = false;
	    thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);
	 }
      }
      
      return NHPC_SUCCESS;
   }   
   
   void nhpc_system_t::clean_inactive_children()
   {
      thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_READ);	 
      int count = child_processes->ret_count();
      thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_READ);	 
            
      for(int i = 1; i <= count; i++)
      {
	 thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_READ);	 
	 child_process_t *child_process = (child_process_t *)(*child_processes)[i];
	 
	 if(!child_process)
	 {
	    thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_READ);	 
	    continue;
	 }
	 
	 if(!(child_process->active))
	 {
	    thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_READ);

	    thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);	 
	    child_processes->erase(child_process->pid);
	    thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_WRITE);	 
	    
	    thread_mutex_lock(&mutex_chld, NHPC_THREAD_LOCK_READ);	 
	    fnc_ptr_int_t trigger = (fnc_ptr_int_t )child_handlers->search(child_process->service_name);
	    thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_READ);	 

	    int *_pid = new int;
	    *_pid = child_process->pid;
	    (trigger)(_pid);	    
	    
	    delete[] (child_process->service_name);
	    delete child_process;
	 }

	 thread_mutex_unlock(&mutex_chld, NHPC_THREAD_LOCK_READ);
      }
   }
   
   void system_prefork_routine()
   {
#ifdef ENABLE_GARBAGE_COLLECTOR
      garbage_collector_ready = false;
#endif
   }
};