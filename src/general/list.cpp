/*
 *	(C) 2012 Varun Mittal <varunmittal91@gmail.com>
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

#include <iostream>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/alloc.h>
#include <include/list.h>

namespace neweraHPC
{
   list_t::list_t(short int _mode)
   {
      mutex = new pthread_mutex_t;
      pthread_mutex_init(mutex, NULL);
      
      head = NULL;
      
      if(_mode == LIST_MAX_FIRST)
	 mode = LIST_MAX_FIRST;
      else 
	 mode = LIST_MIN_FIRST;
   }
   
   list_t::~list_t()
   {
      if(head)
      {
	 elem_t *tmp_elem = head;
	 elem_t *tmp_elem_next;
	 
	 while(tmp_elem)
	 {
	    tmp_elem_next = tmp_elem->next;
	    
	    delete tmp_elem;
	    tmp_elem = tmp_elem->next;
	 }
      }
      
      delete mutex;
   }
   
   void list_t::add_elem(int number)
   {
      elem_t *new_elem = new elem_t;
      memset(new_elem, 0, sizeof(elem_t));
      new_elem->data = number;
      
      pthread_mutex_lock(mutex);
      
      if(head == NULL)
      {
	 head = new_elem;
      }
      else 
      {
	 elem_t *tmp_elem = head;
	 elem_t *prev_elem = NULL;
	 
	 while(tmp_elem != NULL)
	 {
	    if(mode == LIST_MAX_FIRST)
	    {
	       if(tmp_elem->data < number)
		  break;
	    }
	    else 
	    {
	       if(tmp_elem->data > number)
		  break;	    
	    }
	    
	    prev_elem = tmp_elem;
	    tmp_elem = tmp_elem->next;
	 }
	 
	 if(tmp_elem == head)
	 {
	    head = new_elem;
	    head->next = tmp_elem;
	 }
	 else 
	 {
	    prev_elem->next = new_elem;
	    new_elem->next = tmp_elem;
	 }
      }
      
      pthread_mutex_unlock(mutex);
   }
   
   int list_t::pop_elem()
   {
      if(head == NULL)
      {
	 return -1;
      }
      else 
      {
	 elem_t *tmp_elem = head;
	 head = head->next;
	 int number = tmp_elem->data;
	 	 
	 delete tmp_elem;
	 
	 return number;
      }
   }
   
   void list_t::display()
   {
      pthread_mutex_lock(mutex);
      
      cout << "Printing list:" << endl;
      
      elem_t *tmp_elem = head;
      
      while(tmp_elem != NULL)
      {
	 cout << "\t Number " << tmp_elem->data << endl;
	 
	 tmp_elem = tmp_elem->next;
      }
      
      pthread_mutex_unlock(mutex);
   }   
};

