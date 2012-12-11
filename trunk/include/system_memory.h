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

#ifndef _SYSTEM_MEMORY_H_
#define _SYSTEM_MEMORY_H_

#include "constants.h"

namespace neweraHPC
{
   struct nhpc_meminfo_t
   {
      long int total_mem;
      long int free_mem;
      long int total_swap;
      long int free_swap;
   };
   
   nhpc_status_t system_meminfo(nhpc_meminfo_t *meminfo);
};

#endif