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

#ifndef _CFG_H_
#define _CFG_H_

#include "rbtree.h"

namespace neweraHPC
{
   struct parse_node
   {
      parse_node *parent;
      parse_node **children;
      void       *symbol;
   };
   
   class cfg_t
   {
   private:
      struct drvtn_rule_t
      {
	 int        count;
	 const char *word;
	 rbtree_t	    *words;
	 int        wrd_count;
      };
      rbtree_t *symbols;
      int     rule_cnt;
      
   public:
      cfg_t();
      ~cfg_t();
      int add_rule(const char *word, const char **words, int count);
      void display(int rule_id);
      void display();
   };
   
   class parse_tree
   {
   private:
      parse_node *root;
      
   public:
      parse_tree();
      ~parse_tree();
   };
}

#endif