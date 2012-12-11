/*
 *	(C) 2011 Varun Mittal <varunmittal91@gmail.com> & Varun Dhawan <varundhawan5792@gmail.com>
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
#include <string>

#include <include/json.h>

using namespace std;

namespace neweraHPC
{
   nhpc_json_t::nhpc_json_t()
   {
      current_level = 1;
      
      stream_length = 2;
      
      nodes = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
      backtrack = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
      search_queue = NULL;
      
      backtrack->insert(nodes);
   }
   
   nhpc_json_t::~nhpc_json_t()
   {
      rbtree_t *temp = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
      temp->insert(nodes);
      
   loop:
      rbtree_t *current = (rbtree_t *)temp->search(1);
      if(current)
      {
	 for(int i = 1; i <= current->ret_count(); i++)
	 {
	    key_pair_t *key_pair = (key_pair_t *)current->search(i);
	    if(key_pair->key)
	       delete[] (key_pair->key);
	    
	    if(key_pair->json_object == JSON_OBJECT || key_pair->json_object == JSON_ARRAY)
	    {
	       temp->insert(key_pair->value);
	    }
	    else 
	    {
	       if(key_pair->value)
	       {
		  delete[] (char *)(key_pair->value);
	       }
	    }
	    
	    delete key_pair;
	 }
	 
	 delete current;
	 temp->erase(1);
	 
	 goto loop;
      }
      
      delete temp;
      delete backtrack;
      
      if(search_queue != NULL)
      {
	 search_elem_t *search_elem;
	 while(search_elem = (search_elem_t *)search_queue->search(1))
	 {
	    delete search_elem;
	 }
	 delete search_queue;
	 search_queue = NULL;
      }
   }
   
   nhpc_status_t nhpc_json_t::add_element(int json_object, const char *key, const void *value)
   {      
      bool is_arrayobject = false;
      
      rbtree_t *current = (rbtree_t *)backtrack->search(backtrack->ret_count());
      if(current == NULL)
      {
	 return NHPC_FAIL;
      }
      
      key_pair_t *key_pair_last = (key_pair_t *)current->search(current->ret_count());
       
      int stream_length_new = stream_length;
      if(current->ret_count() > 0)
	 stream_length_new += 1;
      
      key_pair_t *key_pair = new key_pair_t;
      memset(key_pair, 0, sizeof(key_pair_t));
	     
      if(key)
      {
	 nhpc_strcpy(&(key_pair->key), key);
	 
	 stream_length_new += strlen(key_pair->key) + 4;
      }
      key_pair->json_object = json_object;
      key_pair->level = current_level;
      
      if(json_object == JSON_STRING || json_object == JSON_NUMBER)
      {
	 if(value == NULL)
	 {
	    goto error_state;
	 }
	 
	 nhpc_strcpy((char **)&(key_pair->value), (char *)value);
	 
	 if(json_object == JSON_STRING)
	    stream_length_new += strlen((char *)key_pair->value) + 2;
	 else 
	    stream_length_new += strlen((char *)key_pair->value);
      }
      else if(json_object == JSON_TRUE)
      {
	 nhpc_strcpy((char **)&(key_pair->value), (char *)"true");
	 stream_length_new += 4;
      }
      else if(json_object == JSON_FALSE)
      {
	 nhpc_strcpy((char **)&(key_pair->value), (char *)"false");	 
	 stream_length_new += 5;
      }
      else if(json_object == JSON_NULL)
      {
	 nhpc_strcpy((char **)&(key_pair->value), (char *)"null");	 
	 stream_length_new += 4;
      }      
      else if(json_object == JSON_ARRAY)
      {
	 rbtree_t *new_child = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
	 key_pair->value = new_child;
	 backtrack->insert(new_child);
	 stream_length_new += 2;
	 
	 current_level++;
      }
      else if(json_object == JSON_OBJECT)
      {
	 rbtree_t *new_child = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
	 key_pair->value = new_child;
	 backtrack->insert(new_child);
	 stream_length_new += 2;
	 
	 current_level++;
      }
      else 
      {
	 goto error_state;
      }
      
      stream_length = stream_length_new;
      current->insert(key_pair);
      
      return NHPC_SUCCESS;
      
   error_state:
      delete[] (key_pair->key);
      delete key_pair;
      return NHPC_FAIL;      
   }
   
   nhpc_status_t nhpc_json_t::close_element()
   {
      nhpc_status_t nrv = backtrack->erase(backtrack->ret_count());
      if(nrv == NHPC_SUCCESS)
      {
	 current_level--;
      }
      
      return nrv;
   }
      
   int nhpc_json_t::search(key_pair_t **_key_pair, int *child_count)
   {
      *_key_pair = NULL;
      
      if(backtrack->ret_count() != 0)
      {
	 return JSON_INCOMPLETE;
      }
      
   loop:
      search_elem_t *search_elem;
      int search_queue_count = 1;
      
      if(search_queue == NULL)
      {
	 search_queue = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
	 search_elem = new search_elem_t;
	 memset(search_elem, 0, sizeof(search_elem_t));
	 
	 search_elem->position = 1;
	 search_elem->branch = nodes;
	 search_queue->insert(search_elem);
      }
      else 
      {
	 search_queue_count = search_queue->ret_count();
	 
	 search_elem = (search_elem_t *)search_queue->search(search_queue_count);
	 if(search_elem == NULL)
	 {
	    delete search_queue;
	    search_queue = NULL;
	    return JSON_END;
	 }
      }
      
      key_pair_t *key_pair = (key_pair_t *)search_elem->branch->search(search_elem->position);
      if(!key_pair)
      {
	 search_queue->erase(search_queue_count);
	 delete search_elem;
	 goto loop;
      }
      else 
      {
	 (search_elem->position)++;
	 
	 if(key_pair->json_object == JSON_ARRAY || key_pair->json_object == JSON_OBJECT)
	 {
	    search_elem = new search_elem_t;
	    memset(search_elem, 0, sizeof(search_elem_t));
	    
	    search_elem->branch = (rbtree_t *)key_pair->value;
	    search_elem->position = 1;
	    cout << search_queue << endl;
	    search_queue->insert(search_elem);

	    if(child_count)
	       *child_count = search_elem->branch->ret_count();
	 }  
	 
	 *(_key_pair) = key_pair;
	 return search_queue_count;
      }
   }
   
   const char *nhpc_json_t::get_stream()
   {
      if(backtrack->ret_count() > 0)
	 return NULL;
      
      struct object_status_t
      {
	 rbtree_t *tree;
	 int elements_done;
	 int elements;
	 int json_object;
      };
      rbtree_t *stack = new rbtree_t(NHPC_RBTREE_NUM_MANAGED);
      
      key_pair_t *key_pair = NULL;
      key_pair_t *key_pair_current;
      
      int json_object = 0;
      int json_object_old = 0;      
      int tab_count = 0;
      
      int child_count;
      int child_count_tmp = 0;
      
      object_status_t *object_status = new object_status_t;
      memset(object_status, 0, sizeof(object_status_t));
      
      object_status->tree = nodes;
      object_status->elements_done = 0;
      object_status->elements = nodes->ret_count();
      object_status->json_object = JSON_OBJECT;
      stack->insert(object_status);
      
      char *json_out_str = new char [stream_length + 1];
      json_out_str[stream_length] = '\0';
      char *json_out_str_tmp = json_out_str;
      
      *json_out_str_tmp = '{';
      json_out_str_tmp++;
      
      //string json_out = "{";
      
      do 
      {
	 object_status_t *object_status_current = (object_status_t *)stack->search(stack->ret_count());
	 
	 if(key_pair)
	 {
	    if(object_status_current)
	       (object_status_current->elements_done) += 1;
	    
	    if(key_pair->json_object == JSON_OBJECT || key_pair->json_object == JSON_ARRAY)
	    {	       
	       object_status_t *object_status = new object_status_t;
	       memset(object_status, 0, sizeof(object_status_t));
	       
	       object_status->tree = (rbtree_t *)(key_pair->value);
	       object_status->elements_done = 0;
	       object_status->elements = ((rbtree_t *)(key_pair->value))->ret_count();
	       object_status->json_object = key_pair->json_object;
	       stack->insert(object_status);
	       
	       if(key_pair->json_object == JSON_OBJECT)
	       {
		  if(object_status_current && object_status_current->json_object == JSON_ARRAY)
		  {
		     //json_out = json_out + "{";
		     
		     *json_out_str_tmp = '{';
		     json_out_str_tmp++;
		  }
		  else 
		  {
		     //json_out = json_out + "\"" + key_pair->key + "\"" + ": {";

		     *json_out_str_tmp = '"';
		     json_out_str_tmp++;
		     memcpy((json_out_str_tmp), key_pair->key, strlen(key_pair->key));
		     json_out_str_tmp = json_out_str_tmp + strlen(key_pair->key);		     
		     memcpy((json_out_str_tmp), "\": {", 4);
		     json_out_str_tmp += 4;
		  }
	       }
	       else 
	       {
		  //json_out = json_out + "\"" + key_pair->key + "\"" + ": [";

		  *json_out_str_tmp = '"';
		  json_out_str_tmp++;
		  memcpy((json_out_str_tmp), key_pair->key, strlen(key_pair->key));
		  json_out_str_tmp = json_out_str_tmp + strlen(key_pair->key);
		  memcpy((json_out_str_tmp), "\": [", 4);
		  json_out_str_tmp += 4;
	       }
	       
	       object_status_current = object_status;
	    }
	    else 
	    {
	       //json_out = json_out + "\"" + key_pair->key + "\"";
	       
	       *json_out_str_tmp = '"'; 
	       json_out_str_tmp++;
	       memcpy((json_out_str_tmp), key_pair->key, strlen(key_pair->key));
	       json_out_str_tmp = json_out_str_tmp + strlen(key_pair->key);
	       memcpy((json_out_str_tmp), "\"", 1);
	       json_out_str_tmp++;
	       
	       if(key_pair->json_object == JSON_NULL)
	       {
		  //json_out = json_out + ": null";

		  memcpy((json_out_str_tmp), ": null", 6);
		  json_out_str_tmp += 6;
	       }
	       else if(key_pair->json_object == JSON_TRUE)
	       {
		  //json_out = json_out + + ": true";

		  memcpy((json_out_str_tmp), ": true", 6);
		  json_out_str_tmp += 6;
	       }
	       else if(key_pair->json_object == JSON_FALSE)
	       {
		  //json_out = json_out + + ": false";
		  
		  memcpy((json_out_str_tmp), ": false", 7);
		  json_out_str_tmp += 7;
	       }
	       else if(key_pair->json_object == JSON_NUMBER)
	       {
		  //json_out = json_out + + ": " + (char *)key_pair->value;
		  
		  memcpy((json_out_str_tmp), ": ", 2);
		  json_out_str_tmp += 2;
		  memcpy((json_out_str_tmp), (const char *)key_pair->value, strlen((char *)key_pair->value));
		  json_out_str_tmp += strlen((char *)key_pair->value);
	       }
	       else
	       {
		  //json_out = json_out + + ": \"" + (char *)key_pair->value + "\"";
		  
		  memcpy((json_out_str_tmp), ": ", 2);
		  json_out_str_tmp += 2;
		  
		  *json_out_str_tmp = '"';
		  json_out_str_tmp++;
		  memcpy((json_out_str_tmp), (const char *)key_pair->value, strlen((char *)key_pair->value));
		  json_out_str_tmp += strlen((char *)key_pair->value);		  
		  *json_out_str_tmp = '"';
		  json_out_str_tmp++;
	       }
	       
	       if(object_status_current && object_status_current->elements_done < object_status_current->elements)
	       {
		  //json_out = json_out + ",";
		  
		  *json_out_str_tmp = ',';
		  json_out_str_tmp++;
	       }
	    }	    
	    
	    while(object_status_current && object_status_current->elements_done == object_status_current->elements)
	    {
	       stack->erase(stack->ret_count());
	       
	       if(object_status_current->json_object == JSON_OBJECT)
	       {
		  //json_out = json_out + "}";
		  
		  *json_out_str_tmp = '}';
		  json_out_str_tmp++;
	       }
	       else 
	       {
		  //json_out = json_out + "]";

		  *json_out_str_tmp = ']';
		  json_out_str_tmp++;
	       }
	       
	       object_status_current = (object_status_t *)stack->search(stack->ret_count());
	       if(object_status_current && object_status_current->elements_done < object_status_current->elements)
	       {
		  //json_out = json_out + ",";
		  
		  *json_out_str_tmp = ',';
		  json_out_str_tmp++;
	       }
	    }
	 }
	 
	 json_object_old = json_object;
	 json_object = search(&key_pair);
      }while(json_object != JSON_END);
      
      return json_out_str;
   }
   
   bool nhpc_json_t::is_delimiter(char in_char)
   {
      if(in_char == ',' || in_char == '}' || in_char == ']')
	 return true;
      
      return false;
   }
   
   nhpc_status_t nhpc_json_t::build_structure(const char *json_string)
   {
      char *json_string_ptr = (char *)json_string;
      char *key = NULL;
      char *value = NULL;
      int start_pos, end_pos;
      
      start_pos = 1 + skip_white_spaces(&json_string_ptr);
      
      if(*json_string_ptr != '{')
	 goto error_state;
      json_string_ptr++;
      start_pos++;
      end_pos = start_pos;
      
      while((*json_string_ptr) != '\0')
      {
	 char **key_value_str;
	 if(key == NULL)
	    key_value_str = &key;
	 else 
	    key_value_str = &value;
	 
	 start_pos += skip_white_spaces((char **)&json_string_ptr);
	 end_pos = start_pos;
	 
	 if(*json_string_ptr == '"')
	 {
	    start_pos++;
	    json_string_ptr++;
	    
	    while(*json_string_ptr != '"')
	    {
	       if(*json_string_ptr == '\0')
		  goto error_state;
	       
	       json_string_ptr++;
	       end_pos++;
	    }
	    
	    *key_value_str = nhpc_substr(json_string, start_pos, end_pos);
	    start_pos = end_pos + 1;
	    json_string_ptr++;
	    
	    start_pos += skip_white_spaces(&json_string_ptr);
	    
	    if(*json_string_ptr != ':')
	    {
	       goto error_state;	       
	    }
	    
	    start_pos++;
	    json_string_ptr++;
	    if(*json_string_ptr == '\0')
	    {
	       goto error_state;
	    }
	    
	    start_pos += skip_white_spaces(&json_string_ptr);
	    end_pos = start_pos;
	    
	    if(*json_string_ptr == '{' || *json_string_ptr == '[')
	    {
	       if(*json_string_ptr == '[')
	       {
		  add_element(JSON_ARRAY, key);
	       }
	       else 
	       {
		  add_element(JSON_OBJECT, key);		  
	       }
	       delete[] key;
	       key = NULL;
	       
	       start_pos += 2;
	       json_string_ptr++;
	    }
	    else 
	    {
	       bool ignore_space = false;
	       bool close_string = true;
	       if(*json_string_ptr == '"')
	       {
		  ignore_space = true;
		  close_string = false;
	       }
	       
	       while(1)
	       {
		  if(close_string)
		  {
		     if(is_delimiter(*json_string_ptr))
			break;
		  }
		  
		  if(!ignore_space)
		     if(*json_string_ptr == ' ')
			break;
		  if(*json_string_ptr == '\0')
		  {		  
		     goto error_state;
		  }
		  
		  end_pos++;
		  *json_string_ptr++;

		  if(*json_string_ptr == '"')
		  {
		     ignore_space = false;
		     close_string = true;
		  }
	       }
	       
	       value = nhpc_substr(json_string, start_pos, end_pos);
	       start_pos = end_pos + 1;
	       
	       if(nhpc_strcmp(value, "true") == NHPC_SUCCESS)
	       {
		  add_element(JSON_TRUE, key);
	       }
	       else if(nhpc_strcmp(value, "false") == NHPC_SUCCESS)
	       {
		  add_element(JSON_FALSE, key);
	       }
	       else if(nhpc_strcmp(value, "null") == NHPC_SUCCESS)
	       {
		  add_element(JSON_NULL, key);
	       }
	       else if(nhpc_strcmp(value, "\"*\"") == NHPC_SUCCESS)
	       {
		  add_element(JSON_STRING, key, value);
	       }
	       else 
	       {
		  add_element(JSON_NUMBER, key, value);
	       }
	       
	       delete[] key;
	       delete[] value;
	       key = NULL;
	       value = NULL;
	    }
	 }
	 else if(*json_string_ptr == '{')
	 {
	    add_element(JSON_OBJECT);
	    
	    json_string_ptr++;
	    start_pos++;
	 }
	 else if(is_delimiter(*json_string_ptr))
	 {
	    if(*json_string_ptr != ',')
	    {
	       close_element();
	    }

	    json_string_ptr++;
	    start_pos++;
	 }
	 else 
	 {
	    json_string_ptr++;
	    start_pos++;
	 }
      }
      
      skip_white_spaces(&json_string_ptr);
      
      if(*json_string_ptr == '}')
	 close_element();
      
      if(backtrack->ret_count() != 0)
      {
	 goto error_state;
      }
      
      return NHPC_SUCCESS;
      
   error_state:
      return NHPC_FAIL;
   }   
};
