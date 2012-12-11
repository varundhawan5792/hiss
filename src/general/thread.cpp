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
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/thread.h>
#include <include/error.h>

using namespace std;

namespace neweraHPC
{
   thread_manager_t::thread_manager_t()
   {          
      active_threads = new rbtree_t;
      
      thread_count = 0;
      thread_mutex_init(&mutex_count);
      thread_mutex_init(&mutex);
   }
   
   thread_manager_t::~thread_manager_t()
   {
      LOG_INFO("Shuting Down Thread Manager");
      
      pthread_t *thread;
      int thread_id;      
      while(1)
      {
	 lock(&mutex, NHPC_THREAD_LOCK_READ);
	 thread = (pthread_t *)(*active_threads).search_first(&thread_id);
	 unlock(&mutex, NHPC_THREAD_LOCK_READ);
	 
	 if(!thread)
	    break;
	 
	 cancel_thread(thread_id);
	 delete_thread_data(thread_id);
      }
      
      delete active_threads;
   }
   
   inline void thread_manager_t::lock(nhpc_mutex_t *in_mutex, int lock_mode)
   {
      thread_mutex_lock(in_mutex, lock_mode);
   }
   
   inline void thread_manager_t::unlock(nhpc_mutex_t *in_mutex, int lock_mode)
   {
      thread_mutex_unlock(in_mutex, lock_mode);
   }
   
   nhpc_status_t thread_manager_t::init_thread(int *thread_id, pthread_t **thread)
   {
      pthread_t *thread_new = new pthread_t;
      memset(thread_new, 0, sizeof(pthread_t));

      lock(&mutex, NHPC_THREAD_LOCK_WRITE);
      (*thread_id) = (*active_threads).insert((void *)thread_new);
      unlock(&mutex, NHPC_THREAD_LOCK_WRITE);
      
      if(thread)
	 (*thread) = thread_new;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t thread_manager_t::create_thread(int *thread_id, const pthread_attr_t *attr, 
						 void *(*start_routine)(void*), void *arg, int thread_state)
   {      
      int rv;
      pthread_t *thread = (pthread_t *)(*active_threads).search(*thread_id);
      if(!thread)
	 return NHPC_FAIL;
      
      while(1)
      {
	 int tmp_thread_count;
	 thread_mutex_lock(&mutex_count, NHPC_THREAD_LOCK_READ);
	 tmp_thread_count = thread_count;
	 thread_mutex_unlock(&mutex_count, NHPC_THREAD_LOCK_READ);      
	 
	 if(tmp_thread_count > MAX_THREADS)
	    sleep(1);
	 else 
	    break;
      };

      rv = pthread_create(thread, attr, start_routine, arg);
      if(rv != 0)
      {
	 perror("Thread Creation Failed");
	 return errno;
      }
      
      if(thread_state == NHPC_THREAD_JOIN)
      {
	 rv = pthread_join(*thread, NULL); 
	 if(rv != 0)
	 {
	    perror("Thread Creation Failed");
	    return errno;
	 }
      }
      else if(thread_state == NHPC_THREAD_DETACH)
      {
	 rv = pthread_detach(*thread);
	 if(rv != 0)
	 {
	    perror("Thread Creation Failed");
	    return errno;
	 }
      }      
      
      thread_mutex_lock(&mutex_count, NHPC_THREAD_LOCK_WRITE);
      thread_count++;
      thread_mutex_unlock(&mutex_count, NHPC_THREAD_LOCK_WRITE);
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t thread_manager_t::join_thread(int thread_id)
   {
      nhpc_status_t nrv;
      pthread_t *thread;
      
      lock(&mutex, NHPC_THREAD_LOCK_READ);
      thread = (pthread_t *)(*active_threads).search(thread_id);
      unlock(&mutex, NHPC_THREAD_LOCK_READ);
      
      if(!thread)
	 return NHPC_FAIL;
      
      nrv = pthread_detach(*thread);
      if(nrv != 0)
	 return errno;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t thread_manager_t::detach_thread(int thread_id)
   {
      nhpc_status_t nrv;
      pthread_t *thread;
      
      lock(&mutex, NHPC_THREAD_LOCK_READ);
      thread = (pthread_t *)(*active_threads).search(thread_id);
      unlock(&mutex, NHPC_THREAD_LOCK_READ);
      
      if(!thread)
	 return NHPC_FAIL;
      
      nrv = pthread_join(*thread, NULL);
      if(nrv != 0)
	 return errno;
      
      return NHPC_SUCCESS;      
   }
   
   void thread_manager_t::delete_thread_data(int rbtree_t_id)
   {
      lock(&mutex, NHPC_THREAD_LOCK_WRITE);
      pthread_t *thread = (pthread_t *)(*active_threads).search(rbtree_t_id);
      if(thread){
	 (*active_threads).erase(rbtree_t_id);
	 delete thread;
      }      
      
      thread_count--;
      
      unlock(&mutex, NHPC_THREAD_LOCK_WRITE);
   }
   
   nhpc_status_t thread_manager_t::cancel_thread(int rbtree_t_id)
   {
      nhpc_status_t nrv;
      
      lock(&mutex, NHPC_THREAD_LOCK_READ);
      pthread_t *thread = (pthread_t *)(*active_threads).search(rbtree_t_id);
      unlock(&mutex, NHPC_THREAD_LOCK_READ);
      
      if(!thread)
	 return NHPC_FAIL;
      
      nrv = pthread_cancel(*thread);
      if(nrv != 0)
	 return errno;
      
      return NHPC_SUCCESS;
   }    
   
   nhpc_status_t thread_mutex_init(nhpc_mutex_t *mutex)
   {
      pthread_rwlock_init(&(mutex->lock) , NULL);
      pthread_mutex_init(&(mutex->lock_write), NULL);

      return NHPC_SUCCESS;
   }
   
   nhpc_status_t thread_mutex_lock(nhpc_mutex_t *mutex, int for_read)
   {
      if(for_read)
	 pthread_rwlock_rdlock(&(mutex->lock));
      else 
	 pthread_rwlock_wrlock(&(mutex->lock));
	 
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t thread_mutex_unlock(nhpc_mutex_t *mutex, int for_read)
   {
      pthread_rwlock_unlock(&(mutex->lock));
   }
};
