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

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/alloc.h>
#include <include/system.h>

namespace neweraHPC
{
   double cpu_usage()
   {
      double sample[3];
      getloadavg(sample, 3);
      double max = sample[0];
      if(max < sample[1])
	 max = sample[1];
      if(max < sample[2])
	 max = sample[2];
      
      return max;
   }
   
   nhpc_status_t system_cpuinfo(nhpc_cpuinfo_t *cpu_info)
   {
      system_loadavg(cpu_info);
   }
      
   
   nhpc_status_t system_loadavg(nhpc_cpuinfo_t *cpuinfo)
   {
      getloadavg(&(cpuinfo->load_avg_5), 3);

      double max = cpuinfo->load_avg_5;
      if(max < cpuinfo->load_avg_15)
	 max = cpuinfo->load_avg_15;
      if(max < cpuinfo->load_avg_25)
	 max = cpuinfo->load_avg_25;
      
      cpuinfo->load_avg_max = max;
      
      return NHPC_SUCCESS;
   }
};