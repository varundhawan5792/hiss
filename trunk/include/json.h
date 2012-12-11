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

#ifndef _JSON_H_
#define _JSON_H_

#include "rbtree.h"

#define JSON_END 0
#define JSON_INCOMPLETE -1

namespace neweraHPC
{
   enum json_object
   {
      JSON_STRING,
      JSON_NUMBER,
      JSON_TRUE,
      JSON_FALSE,
      JSON_NULL,
      JSON_OBJECT,
      JSON_ARRAY
   };
   
   enum json_mide
   {
      JSON_INSERT,
      JSON_UPDATE
   };
   
   class nhpc_json_t
   {
   private:
      struct search_elem_t
      {
	 rbtree_t *branch;
	 int position;
      };  
      struct stream_elem_t
      {
	 int json_object;
      };
      
      int stream_length;
      int current_level;
      rbtree_t *nodes;
      rbtree_t *backtrack;
      rbtree_t *search_queue;
   public:
      struct key_pair_t
      {
	 char *key;
	 void *value;
	 int json_object;
	 int level;
      };

      nhpc_json_t();
      ~nhpc_json_t();
      nhpc_status_t add_element(int json_object, const char *key = NULL, const void *value = NULL);
      nhpc_status_t close_element();
      int search(key_pair_t **key_pair, int *child_count = NULL);

      nhpc_status_t search(const char *key);
      
      const char *get_stream();
      nhpc_status_t build_structure(const char *json_string);
      bool is_delimiter(char in_char);
      void print();
   };
};

#endif
