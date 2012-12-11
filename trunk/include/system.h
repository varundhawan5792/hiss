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

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#define MAX_TERMINATED_KEYS 10

#include <sys/wait.h>

#include "system_memory.h"
#include "alloc.h"
#include "thread.h"

namespace neweraHPC
{
   struct nhpc_cpuinfo_t
   {
      double load_avg_5;
      double load_avg_15;
      double load_avg_25;
      double load_avg_max;
   };
   
   struct nhpc_systeminfo_t
   {
      nhpc_cpuinfo_t cpuinfo;
      nhpc_meminfo_t meminfo;
   };
   
   class nhpc_system_t
   {
   private:
      struct child_process_t
      {
	 char *service_name;
	 bool active;
	 int pid;
      };
      
      thread_manager_t **thread_manager;
      nhpc_systeminfo_t *systeminfo;
      nhpc_mutex_t mutex;
      
      nhpc_mutex_t mutex_chld;
      rbtree_t *child_processes;
      rbtree_t *child_handlers;
      int terminated[MAX_TERMINATED_KEYS];
      
   public:
      nhpc_system_t();
      nhpc_system_t(thread_manager_t **_thread_manager);
      ~nhpc_system_t();
      void init_system();
      
      nhpc_status_t system_memstats(nhpc_meminfo_t **meminfo);
      nhpc_status_t system_cpustats(nhpc_cpuinfo_t **cpuinfo);
      nhpc_status_t system_stats(nhpc_systeminfo_t **systeinfo);
      
      nhpc_status_t register_trigger_child_process(char *service_name, fnc_ptr_int_t function);
      nhpc_status_t create_child_process(char *service_name, int *child_pid);
      nhpc_status_t free_child_process();      
      void clean_inactive_children();
      
      static void *monitor_system(nhpc_system_t *system);
   };
   
   double cpu_usage();
   nhpc_status_t system_cpuinfo(nhpc_cpuinfo_t *cpuinfo);
   nhpc_status_t system_systeminfo(nhpc_systeminfo_t *systeminfo);  
   nhpc_status_t system_loadavg(nhpc_cpuinfo_t *cpuinfo);
   
   void system_prefork_routine();
   
   extern nhpc_system_t nhpc_system;
   
   void child_prepare();
};

#endif
