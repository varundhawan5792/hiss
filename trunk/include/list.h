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

#ifndef _LIST_H_
#define _LIST_H_

#define LIST_MIN_FIRST 0
#define LIST_MAX_FIRST 1

namespace neweraHPC
{
   class list_t
   {
   private:
      struct elem_t
      {
	 int data;
	 elem_t *next;
      };
            
      short int mode;
      elem_t *head;
      pthread_mutex_t *mutex;
   public:
      list_t(short int _mode);
      ~list_t();
      
      void display();
      void add_elem(int number);
      int pop_elem();
   };
};

#endif