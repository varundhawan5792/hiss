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
#include <math.h>
#ifdef linux
#include <stdlib.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/strings.h>
#include <include/strings_pool.h>
#include <include/constants.h>
#include <include/error.h>
#include <include/alloc.h>

using namespace std;

namespace neweraHPC{
   const char *alphabets = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   strings_pool_t strings_pool;
   bool garbage_collector_ready = false;
   
   void init_garbage_collector()
   {
#ifdef ENABLE_GARBAGE_COLLECTOR
      garbage_collector_ready = true;
#endif
   }
   
   strings_pool_t::strings_pool_t()
   {
      strings_free = new rbtree_t(NHPC_RBTREE_NUM_HASH);
      strings_allocated = new rbtree_t(NHPC_RBTREE_NUM_HASH);
      
      mutex_free = new nhpc_mutex_t;
      thread_mutex_init(mutex_free);
      
      mutex_allocated = new nhpc_mutex_t;
      thread_mutex_init(mutex_allocated);
      
      free_count = 0;
      allocated_count = 0;
      
      free_bytes = 0;
      allocated_bytes = 0;
   }
   
   strings_pool_t::~strings_pool_t()
   {
      garbage_collector_ready = false;
      
      LOG_INFO("Shuting Down String Pool");
      
      delete strings_free;
      delete strings_allocated;
      
      LOG_INFO("String Pool closed");
   }
   
   char *strings_pool_t::search_string(nhpc_size_t str_len)
   {
      LOG_DEBUG("Searching for available string" << str_len);
      
      char *string = NULL;
      char *str = NULL;
      
      nhpc_size_t try_len_limit = str_len + 5;
      nhpc_size_t try_len = str_len;
      
      thread_mutex_lock(mutex_free, NHPC_THREAD_LOCK_WRITE);      	 
      do 
      {
	 string = (char *)strings_free->search(try_len, 1);
	 
	 if(string != NULL)
	 {
	    int ret = strings_free->erase(try_len, string);
	    LOG_DEBUG("FOUND EXISTING STRING AND REMOVED FROM FREE LIST with return status" << ret << " and size: " << try_len<< " original size: " << str_len);
	    if(ret == 0)
	       return NULL;
	    
	    free_count--;
	    free_bytes -= try_len;
	    
	    break;
	 }
	 
	 try_len++; 
      }while(try_len < try_len_limit);
      thread_mutex_unlock(mutex_free, NHPC_THREAD_LOCK_WRITE);      
      
      if(string == NULL)
      {
	 LOG_DEBUG("ALLOCATING NEW STRING");
	 
	 string = alloc<char>(sizeof(nhpc_size_t) + sizeof(char) * str_len);
	 memset(string, 0, (sizeof(nhpc_size_t) + sizeof(char) * str_len));
	 
	 nhpc_size_t *str_len_string = (nhpc_size_t *)string;
	 *str_len_string = str_len;
	 try_len = str_len;
	 
	 allocated_bytes += (sizeof(nhpc_size_t) + sizeof(char) * str_len);
      }
      
      str = (char *)(string + sizeof(nhpc_size_t));
      
      thread_mutex_lock(mutex_allocated, NHPC_THREAD_LOCK_WRITE);
      strings_allocated->insert(string, try_len);
      allocated_count++;
      thread_mutex_unlock(mutex_allocated, NHPC_THREAD_LOCK_WRITE);
      
      return str;
   }
   
   void strings_pool_t::free_string(char *str_address)
   {
      if(!str_address)
	 return;
      
      LOG_DEBUG("Free count: " << free_count << " Allocated count: " << allocated_count);
      
      char *str = str_address - sizeof(nhpc_size_t);      
      nhpc_size_t *str_len_string = (nhpc_size_t *)(str);
      LOG_DEBUG("Length of string input: " << *str_len_string);
      
      if(*str_len_string < 0)
      {
	 LOG_ERROR("Invalid memory allocation");
	 return;
      }
      else if(*str_len_string == 0)
      {
	 LOG_ERROR("Not in strings pool");
	 
	 return;
      }
      
      thread_mutex_lock(mutex_allocated, NHPC_THREAD_LOCK_WRITE);
      int ret = strings_allocated->erase(*str_len_string, str);
      if(ret != 0)
      {
	 allocated_count--;
	 allocated_bytes -= (*str_len_string);
	 free_bytes += (*str_len_string);
      }
      
      thread_mutex_unlock(mutex_allocated, NHPC_THREAD_LOCK_WRITE);
      
      if(ret == 0)
      {
	 return;
      }
      else 
      {
	 thread_mutex_lock(mutex_free, NHPC_THREAD_LOCK_WRITE);
	 strings_free->insert(str, *str_len_string);	 
	 free_count++;
	 thread_mutex_unlock(mutex_free, NHPC_THREAD_LOCK_WRITE);
	 
	 LOG_DEBUG("ADDING STRING TO FREE POOL erased with return status " << ret << " Allocated bytes: " << allocated_bytes << " Free bytes: "<< free_bytes);
      }
      
      clean_strings();
   }
   
   void strings_pool_t::clean_strings()
   {
      if(free_count > MAX_STRING_COUNT)
      {	 
	 thread_mutex_lock(mutex_free, NHPC_THREAD_LOCK_WRITE);
	 
	 free_count = 0;
	 
	 int key;
	 bool exit_loop;
	 void *node_data;
	 
	 while(1)
	 {
	    node_data = strings_free->search_first(&key);
	    
	    if(!node_data)
	       break;
	    
	    char *string;
	    
	    while(1)
	    {
	       string = (char *)strings_free->search(key, 1);
	       if(string)
	       {
		  free(string);
		  
		  strings_free->erase(key, 1);
	       }
	       else 
	       {
		  exit_loop = true;
		  break;
	       }
	    }
	    
	    strings_free->erase(key);
	 }
	 
	 thread_mutex_unlock(mutex_free, NHPC_THREAD_LOCK_WRITE);
      }
   }
   
   char *nhpc_allocate_str(nhpc_size_t str_len)
   {
      char *str;
      
      if(garbage_collector_ready)
	 str = strings_pool.search_string(str_len);
      else 
	 str = new char [str_len];
      
      return str;
   }
   
   void nhpc_deallocate_str(char *str_address)
   {
      if(garbage_collector_ready)
	 strings_pool.free_string(str_address);
   }
   
   nhpc_status_t nhpc_strcpy(char **dst, const char *src)
   {
      nhpc_size_t len = strlen(src);
      
      (*dst) = new char [len + 1];
      memcpy(*dst, src, len);
      (*dst)[len] = '\0';
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_strcpy_noalloc(char *dst, const char *src)
   {
      nhpc_size_t len = strlen(src);
      
      memcpy(dst, src, len);
      dst[len] = '\0';
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_strcmp(const char *s1, const char *s2)
   {
      const char *tmp_s1 = s1;
      const char *tmp_s2 = s2;
      bool loop = false;
      const char *tmp_loop_s2;
      
      if(*tmp_s2 == '*')
	 while(*(tmp_s2 + 1) == '*')
	    tmp_s2++;
      
      if(*tmp_s2 == '\0')
	 return NHPC_SUCCESS;
      
      while(*tmp_s1 != '\0' || *tmp_s2 != '\0')
      {
	 if(*tmp_s1 == *tmp_s2)
	 {
	    tmp_s1++;
	    tmp_s2++;
	    continue;
	 }
	 else if(*tmp_s1 == '\0' && *tmp_s2 == '*')
	 {
	    while(*tmp_s2 == '*')
	       tmp_s2++;
	    continue;
	 }
	 else if(*tmp_s2 == '*')
	 {
	    while(*(tmp_s2 + 1) == '*')
	       tmp_s2++;
	    
	    loop = true;
	    tmp_loop_s2 = tmp_s2;
	    
	    while(*tmp_s1 != *(tmp_s2 + 1) && *tmp_s1 != '\0')
	    {
	       *tmp_s1++;
	    }
	    
	    *tmp_s2++;
	 }
	 else 
	 {
	    if(loop != true)
	       return NHPC_FAIL;
	    else 
	    {
	       loop = false;
	       tmp_s2 = tmp_loop_s2;
	    }
	 }
      }
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_strcmpi(const char *s1, const char *s2)
   {
      const char *tmp_s1 = s1;
      const char *tmp_s2 = s2;
      bool loop = false;
      const char *tmp_loop_s2;
      
      if(*tmp_s2 == '*')
	 while(*(tmp_s2 + 1) == '*')
	    tmp_s2++;
      
      if(*tmp_s2 == '\0')
	 return NHPC_SUCCESS;
      
      while(*tmp_s1 != '\0' || *tmp_s2 != '\0')
      {
	 if(tolower(*tmp_s1) == tolower(*tmp_s2))
	 {
	    tmp_s1++;
	    tmp_s2++;
	    continue;
	 }
	 else if(*tmp_s1 == '\0' && *tmp_s2 == '*')
	 {
	    while(*tmp_s2 == '*')
	       tmp_s2++;
	    continue;
	 }
	 else if(*tmp_s2 == '*')
	 {
	    while(*(tmp_s2 + 1) == '*')
	       tmp_s2++;
	    
	    loop = true;
	    tmp_loop_s2 = tmp_s2;
	    
	    while(tolower(*tmp_s1) != tolower(*(tmp_s2 + 1)) && *tmp_s1 != '\0')
	    {
	       *tmp_s1++;
	    }
	    
	    *tmp_s2++;
	 }
	 else 
	 {
	    if(loop != true)
	       return NHPC_FAIL;
	    else 
	    {
	       loop = false;
	       tmp_s2 = tmp_loop_s2;
	    }
	 }
      }
      
      return NHPC_SUCCESS;
   }
   
   int nhpc_strfind(const char *s1, const char s2)
   {
      return nhpc_strfind(s1, s2, 0);
   }
   
   int nhpc_strfind(const char *s1, const char s2, int old_position)
   {
      const char *tmp_s1 = s1;
      int position = 0;
      
      while(*tmp_s1 != '\0' && old_position != 0)
      {
	 tmp_s1++;
	 old_position--;
	 position++;
      }
      
      if(old_position > 0)
      {
	 position = 0;
	 return position;
      }
      
      while(*tmp_s1 != '\0')
      {
	 position++;
	 if(*tmp_s1 == s2)
	 {
	    return position;
	 }	 
	 tmp_s1++;
      }
      
      position = 0;
      return position;
   }
   
   char *nhpc_substr(const char *s1, int start_pos, int end_pos)
   {
      if(start_pos < 1 || end_pos < 1 || end_pos > strlen(s1))
	 return NULL;
      
      nhpc_size_t len = end_pos - start_pos + 2;
      char *new_string = new char[len];
      memcpy(new_string, (s1 + start_pos - 1), len - 1);
      new_string[len - 1] = '\0';
      
      return new_string;
   }
   
   string_t *nhpc_substr(const char *s1, const char s2)
   {
      nhpc_size_t s1_len = strlen(s1);
      
      string_t *string = NULL;
      char **strings = NULL;
      int count = 0;
      
      const char *tmp_s1 = s1;
      int old_pos = 1;
      int current_pos = 1;
      size_t len = 0;
      
      while(current_pos <= s1_len)
      {
	 if(*tmp_s1 == s2 || (current_pos == s1_len && current_pos > old_pos))
	 {
	    len = current_pos - old_pos;

	    if(current_pos == s1_len)
	       len++;
	       
	    if(len != 0)
	    {
	       char *extracted_string = new char [len + 1];
	       memcpy(extracted_string, s1 + old_pos - 1, sizeof(char) * len);
	       extracted_string[len] = '\0';
	       
	       char **tmp_strings = new char* [count + 1];
	       if(strings != NULL)
	       {
		  memcpy(tmp_strings, strings, (sizeof(char *) * count));
		  delete[] strings;
	       }
	       strings = tmp_strings;
	       strings[count] = extracted_string;
	       count++;
	       
	       old_pos = current_pos;
	    }
	    
	    while(*tmp_s1 == s2)
	    {
	       tmp_s1++;
	       current_pos++;
	       old_pos++;
	    }
	 }
	 
	 tmp_s1++;
	 current_pos++;
      }

      if(count > 0)
      {
	 string = new string_t;
	 string->strings = new char* [count + 1];
	 string->count = count;
	 memcpy((string->strings), strings, sizeof(char*) * count);
	 string->strings[count] = NULL;
	 delete[] strings;
      }
      
      return string;
   }
   
   void nhpc_string_delete(string_t *string)
   {
      for(int i = 0; i < string->count; i++)
      {
	 delete[] (string->strings[i]);
      }
      
      delete[] (string->strings);
      
      delete string;
   }
   
   void nhpc_string_delete(char *string)
   {
      delete[] string;
   }
   
   char *nhpc_strconcat(const char *s1, const char *s2)
   {
      char *string = NULL;
      
      if(s1 == NULL || s2 == NULL)
	 return NULL;
      
      nhpc_size_t len_s1 = strlen(s1);
      nhpc_size_t len_s2 = strlen(s2);
      
      if((len_s1 == 0) && (len_s2 == 0))
	 return NULL;
      else if(len_s1 == 0)
      {
	 string = new char [len_s2 + 1];
	 memcpy(string, s2, len_s2);
	 string[len_s2] = '\0';
      }
      else if(len_s2 == 0)
      {
	 string = new char [len_s1 + 1];
	 memcpy(string, s1, len_s1);
	 string[len_s1] = '\0';
      }
      else 
      {
	 nhpc_size_t len = strlen(s1) + strlen(s2);
	 
	 string = new char [len + 1];
	 memcpy(string, s1, len_s1);
	 memcpy(string + len_s1, s2, len_s2);
	 string[len] = '\0';
      }
      
      return string;
   }
   
   char *nhpc_strconcat(const char *s1, const char *s2, const char *s3)
   {
      char *string = NULL;
      
      if(s1 == NULL || s2 == NULL || s3 == NULL)
	 return NULL;
      
      nhpc_size_t len_s1 = strlen(s1);
      nhpc_size_t len_s2 = strlen(s2);
      nhpc_size_t len_s3 = strlen(s3);
      
      if((len_s1 == 0) && (len_s2 == 0) && (len_s3 == 0))
	 return NULL;
      
      nhpc_size_t len = strlen(s1) + strlen(s2) + strlen(s3);
      
      string = new char [len + 1];
      memcpy(string, s1, len_s1);
      memcpy(string + len_s1, s2, len_s2);
      memcpy(string + (len_s1 + len_s2), s3, len_s3);
      string[len] = '\0';
      
      return string;
   }
   
   char *nhpc_itostr(int num)
   {
      int tmp_num = num;
      int count = 0;
      
      if(num <= 0)
      {
	 char *string = new char [2];
	 string[0] = '0';
	 string[1] = '\0';
	 return string;
      }
      
      while(tmp_num != 0)
      {
	 int digit = tmp_num % 10;
	 tmp_num = tmp_num / 10;	 
	 count++;
      }
      
      tmp_num = num;
      char *string = new char [count + 1];
      string[count] = '\0';
      int i = 0;
      
      while(tmp_num != 0)
      {
	 int digit = tmp_num % 10;
	 tmp_num = tmp_num / 10;
	 string[count - i - 1] = digit + 48;
	 i++;
      }      
      
      return string;
   }
   
   int nhpc_strtoi(const char *str)
   {
      const char *tmp_str = str;
      int size = strlen(str);
      int num = 0;
      
      int tmp_num = 0;
      while(*tmp_str != '\0')
      {
	 tmp_num = (int)(*tmp_str) - 48;
	 num += tmp_num * pow(10 ,(size - 1));
	 
	 size--;
	 tmp_str++;
      }
      
      return num;
   }
   
   char *nhpc_random_string(nhpc_size_t len)
   {
      char *dst = new char [len + 1];
      
      for(int i = 0; i < len; i++)
      {
	 time_t ct;
	 time(&ct);
	 
	 srand(rand() + ct);
	 dst[i] = alphabets[rand() % 52];
      }
      
      dst[len] = '\0';
      
      return dst;
   }
   
   int skip_white_spaces(char **string_ptr)
   {
      int skipped_steps = 0;
      
      while(**string_ptr == ' ')
      {
	 skipped_steps++;
	 (*string_ptr)++;
      }
      
      return skipped_steps;
   }
   
   char *nhpc_strtolower(char *s1)
   {
      for(int i = 0; s1[i]; i++)
	 s1[i] = tolower(s1[i]);
      
      return s1;
   }
}
