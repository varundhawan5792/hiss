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

#include <iostream>
#include <stdio.h>
#ifdef linux
#include <stdlib.h>
#endif

#include <include/rbtree.h>

using namespace std;
using namespace neweraHPC;

typedef struct queue_struct{
   int front;
   int rear;
   int task_total;
   int task_completed;
}queue_t;

queue_t *q;
rbtree_t *tree;

void func_sleep(void)
{
   while(1)sleep(60);
}

bool enqueue(int value, int id)
{
   int status = -1;
   if(q[id].front == q[id].rear && q[id].front == -1)
   {
      q[id].front = q[id].rear = 1;
   }
   else
   {
      q[id].rear++;
   }
   q[id].task_total++;
   status = tree[id].insert(&value, q[id].rear);
   //cout<<"[key = "<<q[id].rear<<"; status = "<<status<<"]";
   if(status > -1)
      return true;
   return false;
}

int dequeue(int id)
{
   int *value, status = -1;
   if(q[id].front == q[id].rear && q[id].front == -1)
   {
      return -9999;
   }
   else
   {
      value = (int*)tree[id].search(q[id].front);
      status = tree[id].erase(q[id].front);
      //cout<<"[key = "<<q[id].front<<"; status = "<<status<<"]";
      if(status != 1)
         return -9999;
      q[id].front++;
      q[id].task_completed++;
      return *value;
   }
}

int find_min(int length)
{
   int min = tree[0].ret_count();
   int pos = 0;
   int i;
   for(i=1; i<length; i++)
      if(tree[i].ret_count() < min)
      {
          min = tree[i].ret_count();
          pos = i;
      }
   return pos;

}

void rbqueue()
{
   int number;
   cout<<endl<<":: Scheduler Simulation ::"<<endl;
   cout<<"Enter number of systems: 4"<<endl;
   //cin>>number;
   number = 4;
   q = new queue_t[number];
   tree = new rbtree_t[number];
   for(int i=0; i<number; i++)
   {
      q[i].front = q[i].rear = -1;
      q[i].task_total = q[i].task_completed = 0;
   }
   cout<<"enqueue task to sys 0: "<<enqueue(12, 0)<<endl;
   cout<<"enqueue task to sys 0: "<<enqueue(15, 0)<<endl;
   cout<<"enqueue task to sys 1: "<<enqueue(15, 1)<<endl;
   cout<<"enqueue task to sys 2: "<<enqueue(15, 2)<<endl;
   cout<<"enqueue task to sys 2: "<<enqueue(15, 2)<<endl;
   cout<<"enqueue task to sys 0: "<<enqueue(15, 0)<<endl;
   cout<<"dequeue task from sys 0: "<<dequeue(0)<<endl;
   cout<<"enqueue task to sys 0: "<<enqueue(15, 0)<<endl;
   cout<<"enqueue task to sys 2: "<<enqueue(15, 2)<<endl;
   cout<<"enqueue task to sys 3: "<<enqueue(15, 3)<<endl;
   cout<<"enqueue task to sys 1: "<<enqueue(15, 1)<<endl;
   //cout<<"ret_count() = "<<tree[1].ret_count()<<endl;
   //cout<<"dequeue: "<<dequeue(1)<<endl;
   //cout<<"ret_count() = "<<tree[1].ret_count()<<endl;
   int next = find_min(number);
   cout<<endl<<"System State:"<<endl;
   for(int i=0; i<number; i++)
   {
      cout<<"System "<<i<<":= Task Running - "<<tree[i].ret_count()<<"; Total Jobs - "<<q[i].task_total<<"; Jobs Completed - "<<q[i].task_completed<<endl;
   }
   cout<<endl<<"next task should go to sys "<<next<<endl;
}

void rbmulti_test()
{
   rbtree_t *tree = new rbtree_t[5];
   int cntr=0, cnt=0, temp;
   do
   {

     for(int i=0; i<5; i++)
     {
	temp = rand()%50;
        cout<<"Inserting "<<temp<<" to tree["<<i<<"] : \treturn: "<<tree[i].insert(&temp, cntr);
        cout<<"\tcount: "<<tree[i].ret_count()<<endl;
        cntr++;
     }
     cnt++;

   }while(cnt<5);

   cntr=0;
   cnt=0;
   do
   {

     for(int i=0; i<5; i++)
     {
        cout<<"Remove key["<<cntr<<"] from tree["<<i<<"] : \treturn: "<<tree[i].erase(cntr)<<"\tcount: "<<tree[i].ret_count()<<endl;
        cntr++;
     }
     cnt++;

   }while(cnt<5);

   /*
    delete tree;
   */
}

int main()
{   
   cout<<"This program is mainly build built for testing the efficiency of rbtree_t module used in the neweraHPC library"<<endl;
   cout<<"Please use some tool to test for memory leaks\n"<<"Attach the binary to some tool and then press any key"<<endl;
   
   rbtree_t *test_tree = new rbtree_t;
   
   int *a,*b,*c,*d;
   a = new int[122];
   b = new int[122];
   c = new int[122];
   d = new int[122];
   
   cout<<a<<endl;
   cout<<b<<endl;
   cout<<c<<endl;
   cout<<d<<endl;
   
   cout<<(*test_tree).insert(a)<<endl;
   cout<<(*test_tree).insert(b)<<endl;
   cout<<(*test_tree).insert(c)<<endl;
   cout<<(*test_tree).insert(d)<<endl;

   delete test_tree;
   delete[] a; 
   delete[] b; 
   delete[] c; 
   delete[] d;
   
   test_tree = new rbtree_t(NHPC_RBTREE_STR);
   cout<<endl;
   cout<<test_tree->insert(a, "1")<<endl;
   cout<<test_tree->insert(b, "2")<<endl;
   cout<<test_tree->insert(c, "3")<<endl;
   cout<<test_tree->insert(d, "4")<<endl;
   
   int *temp = (int *)test_tree->search("1");
   cout<<temp<<endl;
   temp = (int *)test_tree->search("2");
   cout<<temp<<endl;
   temp = (int *)test_tree->search("3");
   cout<<temp<<endl;
   temp = (int *)test_tree->search("4");
   cout<<temp<<endl;
   
   delete test_tree;
   
   cout<<"\n\ntesting [] operator"<<endl;
   test_tree = new rbtree_t;

   cout<<a<<endl; 
   cout<<b<<endl;
   cout<<c<<endl;
   cout<<d<<endl;
   
   (*test_tree).insert(a);
   (*test_tree).insert(b);
   (*test_tree).insert(c);
   (*test_tree).insert(d);
   
   (*test_tree).erase(2);
   
   int count = (*test_tree).ret_count();
   for(int i = 1; i <= count; i++)
   {
      a = (int *)(*test_tree)[i];
      cout<<i<<" "<<a<<endl;
   }
   
   delete test_tree;
   
   cout<<"\n\nTesting managed mode"<<endl;

   test_tree = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
   cout<<"Inserting: "<<(*test_tree).insert(a)<<endl;
   cout<<"Inserting: "<<(*test_tree).insert(b)<<endl;
   cout<<"Inserting: "<<(*test_tree).insert(c)<<endl;
   cout<<"Inserting: "<<(*test_tree).insert(d)<<endl;
   //(*test_tree).insert(c);
   
   (*test_tree).erase(2);
   cout<<"Inserting: "<<(*test_tree).insert(b)<<endl;
   
   cout<<"Count: "<<(*test_tree).ret_count()<<endl;
   
   for(int i = 1; i <= (*test_tree).ret_count(); i++)
   {
      void *test = (*test_tree).search(i);
      //(*test_tree).erase(i);
      //i--;
      cout<<i<<" "<<test<<endl;
   }
   
   delete test_tree;
   
   
   test_tree = new rbtree_t(NHPC_RBTREE_NUM_HASH);
   cout<<(*test_tree).insert(a, 1)<<endl;
   cout<<(*test_tree).insert(b, 1)<<endl;
   cout<<(*test_tree).insert(c, 1)<<endl;
   cout<<(*test_tree).insert(d, 1)<<endl;
   
   cout<<(*test_tree).search(1, 1)<<endl;
   cout<<(*test_tree).search(1, 2)<<endl;
   cout<<(*test_tree).search(1, 3)<<endl;
   cout<<(*test_tree).search(1, 4)<<endl;
   
   delete test_tree;
   
   cout<<"All tree structures and data have been deleted there should not be any trace of memory still remaining\n";
   cout<<"If there is still some memory left that is not deallocated, we have a problem"<<endl;
   
   atexit(func_sleep);   
    
   return 0;
}
