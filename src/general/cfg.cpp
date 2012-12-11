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
#include <iostream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/cfg.h>

using namespace std;

namespace neweraHPC
{
   cfg_t::cfg_t()
   {
      symbols  = NULL;
      rule_cnt = 0;
   }
   
   cfg_t::~cfg_t()
   {
      if(symbols == NULL)
	 delete symbols;
   }
   
   int cfg_t::add_rule(const char *word, const char **words, int count)
   {
      if(symbols == NULL)
	 symbols = new rbtree_t;
      
      drvtn_rule_t *tmp_rule = new drvtn_rule_t;
      tmp_rule->word  = word;
      tmp_rule->words = new rbtree_t;
      tmp_rule->wrd_count = count;
      
      for(int cntr = 0; cntr < count; cntr++)
      {
	 cout<<words[cntr]<<endl;
	 (*tmp_rule->words).insert((void *)words[cntr]);
      }
      
      rule_cnt++;
      int rv = (*symbols).insert((void *)tmp_rule);
      
      return rv;
   }
   
   void cfg_t::display(int rule_id)
   {
      if(symbols == NULL)return;
      
      drvtn_rule_t *tmp_rule = (drvtn_rule_t *)(*symbols).search(rule_id);
      
      if(tmp_rule == NULL)
      {
	 return;
      }
      
      cout<<"Word: "<<tmp_rule->word<<" (with "<<tmp_rule->wrd_count<<" derivation rules)"<<endl;
      for(int cntr = 1; cntr <= tmp_rule->wrd_count; cntr++)
      {
	 cout<<"\t"<<(const char *)(*tmp_rule->words).search(cntr);
      }
      cout<<endl;
   }
   
   void cfg_t::display()
   {
      for(int cntr = 1; cntr < (rule_cnt + 1); cntr++)
      {
	 display(cntr);
      }
   }
}