/*
*
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

#ifndef _GRID_SERVER_H_
#define _GRID_SERVER_H_

namespace neweraHPC
{
   nhpc_status_t nhpc_register_to_controller(const char *grid_controller_addr, const char *grid_controller_port,
					     const char *host_addr, const char *host_port, int host_core_count,
					     int host_cpu_time);
};

#endif
