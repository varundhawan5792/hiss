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

#include <fstream>
#include <cstring>
#include <cstdlib>

#ifdef __APPLE__
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <sys/sysctl.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/alloc.h>
#include <include/system.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t system_meminfo(nhpc_meminfo_t *meminfo)
   {
#ifdef __APPLE__
      int mib[2];
      long int physical_memory;
      size_t length;
      
      mib[0] = CTL_HW;
      mib[1] = HW_MEMSIZE;
      length = sizeof(long int);
      sysctl(mib, 2, &(meminfo->total_mem), &length, NULL, 0);      
      
      vm_size_t page_size;
      mach_port_t mach_port;
      mach_msg_type_number_t count;
      vm_statistics_data_t vm_stats;
      
      mach_port = mach_host_self();
      count = sizeof(vm_stats) / sizeof(natural_t);
      
      if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
	  KERN_SUCCESS == host_statistics(mach_port, HOST_VM_INFO, 
					  (host_info_t)&vm_stats, &count))
      {
	 meminfo->free_mem = (int64_t)vm_stats.free_count * (int64_t)page_size;
	 
      }
      
      return NHPC_SUCCESS;
#else 
      ifstream in;
      char buffer[20];
      in.open("/proc/meminfo");
      while(in.good())
      {
	 in>>buffer;
	 if(strcmp(buffer, "MemTotal:") == 0){
	    in>>buffer;
	    meminfo->total_mem = atoi(buffer);
	 }
	 if(strcmp(buffer, "MemFree:") == 0){
	    in>>buffer;
	    meminfo->free_mem = atoi(buffer);
	 }
	 if(strcmp(buffer, "SwapTotal:") == 0){
	    in>>buffer;
	    meminfo->total_swap = atoi(buffer);
	 }
	 if(strcmp(buffer, "SwapFree:") == 0){
	    in>>buffer;
	    meminfo->free_swap = atoi(buffer);
	 }
      }      
      
      in.close();
#endif
   }
};
