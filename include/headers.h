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

/* These routines are for generating and reading http like headers */

#ifndef _NETWORK_HEADERS_H_
#define _NETWORK_HEADERS_H_

#include "containers.h"
#include "network_data.h"

namespace neweraHPC
{
   class nhpc_headers_t
   {
   private:
      rbtree_t *headers;
   public:
      nhpc_headers_t();
      ~nhpc_headers_t();
      void insert(const char *line);
      void insert(const char *property, const char *argument);
      const char *operator[](int id);
      int count();
      nhpc_status_t write(nhpc_socket_t *sock);
   };
   
   nhpc_status_t nhpc_headers_insert_param(rbtree_t *headers, const char *str);
   void nhpc_delete_headers(rbtree_t *headers);
}

#endif