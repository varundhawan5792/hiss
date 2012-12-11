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

#ifndef _SOCKOPTS_H_
#define _SOCKOPTS_H_

namespace neweraHPC
{
   
   nhpc_status_t socket_options_set(nhpc_socket_t *sock, int sockopt, int on);
   nhpc_status_t socket_nonblock(nhpc_socket_t *sock);
   nhpc_status_t socket_block(nhpc_socket_t *sock);
   
#define socket_is_option_set(skt, option)  \
(((skt)->sockopt & (option)) == (option))
   
#define socket_set_option(skt, option, on) \
do {                                 \
if (on)                          \
(skt)->sockopt |= (option);         \
else                             \
(skt)->sockopt &= ~(option);        \
} while (0)
   
}

#endif
