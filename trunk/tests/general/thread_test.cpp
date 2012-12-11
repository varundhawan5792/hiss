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

#include <include/neweraHPC.h>
#include <iostream>
#include <signal.h>

using namespace std;
using namespace neweraHPC;

void *test(void *args){
   while(1)
   {
      cout<<"Hi thread created successfully"<<endl;
      pthread_testcancel();
   }
}

int main()
{
   thread_manager_t thread_manager;

   int thread_id;
   thread_manager.init_thread(&thread_id, NULL);
   thread_manager.create_thread(&thread_id, NULL, test, NULL, 0);
   if(thread_id==0)
      cout<<"Thread create failed"<<endl;
   thread_manager.cancel_thread(thread_id);
   thread_manager.delete_thread_data(thread_id);
   cout<<"thread created and destroyed"<<endl;
   while(1)
   {
      sleep(1);
   }
   return 0;
}
