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

#ifndef _GRID_SCHEDULER_H_
#define _GRID_SCHEDULER_H_

#include "grid_data.h"
#include "rbtree.h"
#include "system_memory.h"
#include "system.h"

namespace neweraHPC
{
   class grid_scheduler_t;
   
   struct task_t
   {
      time_t t;
      nhpc_instruction_set_t *instruction_set;
      nhpc_systeminfo_t systeminfo;
   };
   
   class grid_scheduler_t
   {
   private:
      struct scheduler_thread_data_t
      {
	 grid_scheduler_t *scheduler;
	 nhpc_instruction_set_t *instruction_set;
	 thread_manager_t *thread_manager;
	 int thread_id;
	 peer_details_t *peer_details;
      };
      
      rbtree_t *peers;
      rbtree_t *jobs;
      rbtree_t *queued_instructions;
      rbtree_t *child_processes;
      pthread_mutex_t *mutex;
      thread_manager_t **thread_manager;
      pthread_mutex_t *mutex_child_processes;
      
      void lock_child_processes();
      void unlock_child_processes();
      
      static void *job_dispatcher(scheduler_thread_data_t *data);
      static void *peer_status_updates();
      
   public:
      nhpc_system_t *extern_system;

      grid_scheduler_t(thread_manager_t **_thread_manager);
      ~grid_scheduler_t();
      void grid_scheduler_init();
      int cores();
      void add_peer(const char *host, const char *port, int processors, int cpu_time);
      void remove_peer(int peer_id);
      peer_details_t *schedule();
      void free_peer(int id);
      void free_peer(int id, double loadavg, nhpc_meminfo_t *meminfo);
      nhpc_status_t queue_job(nhpc_instruction_set_t *instruction_set);
      nhpc_status_t dispatch_job(nhpc_instruction_set_t *instruction_set);
      nhpc_status_t push_jobs();
      void lock();
      void unlock();
      
      nhpc_status_t add_child_process(task_t *task, pid_t *pid);

      static void monitor_jobs_pending(grid_scheduler_t *grid_scheduler);
      
      nhpc_status_t increase_thread(int peer_id);
      nhpc_status_t decrease_thread(int peer_id);
      
      static void *child_exit_trigger(int *pid);
      
      static void *refresh_node_status(grid_scheduler_t *grid_scheduler);
      
      nhpc_status_t request_node_status(peer_details_t *peer_details);
      nhpc_status_t update_node_status(peer_details_t *peer_details);
      
   };   
   
   void child_handler(int signum);
};

#endif
