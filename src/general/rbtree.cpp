/*
 *  Original Author
 *	(C) 1999  Andrea Arcangeli <andrea@suse.de>
 *
 *	Modified by
 *	(C) 2011 Varun Mittal <varunmittal91@gmail.com>
 *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/rbtree.h>
#include <include/alloc.h>
#include <include/strings_pool.h>

using namespace std;

namespace neweraHPC
{
   rbtree_t::rbtree_t()
   {
      memset(&root, 0, sizeof(rb_root));
      
      last_assigned_key = 0;
      count = 0;
      operation_mode = NHPC_RBTREE_NUM;
   }
   
   rbtree_t::rbtree_t(int mode)
   {
      memset(&root, 0, sizeof(rb_root));
      
      last_assigned_key = 0;
      count = 0;

      if(mode == NHPC_RBTREE_STR)
	 operation_mode = NHPC_RBTREE_STR;
      else if(mode == NHPC_RBTREE_NUM_MANAGED)
	 operation_mode = NHPC_RBTREE_NUM_MANAGED;
      else if(mode == NHPC_RBTREE_NUM_HASH)
	 operation_mode = NHPC_RBTREE_NUM_HASH;
      else 
	 operation_mode = NHPC_RBTREE_NUM;
   }
   
   rbtree_t::~rbtree_t()
   {
      struct rb_node *node;
      
      node = rb_first(&root);
      rbtree_t::node *data_prev = NULL;
       
      for(node = rb_first(&root); ;node = rb_next(node))
      {
	 if(data_prev)
            free(data_prev);

         if(!node)
            break;

	 rbtree_t::node *data = rb_entry(node, rbtree_t::node, node_next);
	 rb_erase(&data->node_next, &root);
	 if(operation_mode == NHPC_RBTREE_STR)
	 {
	    if(data->key_pair)
	    {
	       delete (data->key_pair);
	    }
	    delete[] (data->node_key_str);
	 }

         data_prev = data;
      }
   }
   
   void *rbtree_t::search_first(int *key)
   {
      if(operation_mode == NHPC_RBTREE_STR)
	 return NULL;
      
      struct rb_node *node = root.rb_node;
      
      rbtree_t::node *data = container_of(node, rbtree_t::node, node_next);
      
      if(data)
	 (*key) = data->node_key;
      else 
	 return NULL;
      
      return data->node_data;
   }
   
   void *rbtree_t::search_first(const char **key_str)
   {
      if(operation_mode != NHPC_RBTREE_STR)
	 return NULL;
      
      struct rb_node *node = root.rb_node;
      
      rbtree_t::node *data = container_of(node, rbtree_t::node, node_next);
      
      if(data)
	 (*key_str) = data->node_key_str;
      else 
	 return NULL;
      
      return data->node_data;
   }
   
   void *rbtree_t::operator[](int position)
   {
      if(position == 0 || position > count)
	 return NULL;
      
      struct rb_node *node;
      rbtree_t::node *data = NULL;
      
      int pos_count = 1;
      
      for (node = rb_first(&root); node; node = rb_next(node), pos_count++)
      {
	 data = rb_entry(node, rbtree_t::node, node_next);
	 if(pos_count == (position))
	 {
	    break;
	 }
      }      
      
      if(data)
	 return data->node_data;
      else 
	 return NULL;
   }

   void *rbtree_t::search(int key)
   {
      if(operation_mode == NHPC_RBTREE_STR || key == 0)
	 return NULL;

      struct rb_node *node = root.rb_node;
      
      while (node) 
      {
	 int result = 0;
	 
	 rbtree_t::node *data = container_of(node, rbtree_t::node, node_next);
	 if(data == NULL)
	    return NULL;
	 
	 if(key<data->node_key)
	 {
	    result = 1;
	 }
	 else if(key>data->node_key){
	    result = -1;
	 }
	 if (result < 0)
	 {
	    node = node->rb_left;
	 }
	 else if (result > 0)
	 {
	    node = node->rb_right;
	 }
	 else
	    return data->node_data;
      }
      return NULL;
   }
   
   rbtree_t::node *rbtree_t::search_node(int key)
   {
      if(operation_mode == NHPC_RBTREE_STR)
	 return NULL;
      
      struct rb_node *node = root.rb_node;
      
      while (node) 
      {
	 int result = 0;
	 
	 rbtree_t::node *data = container_of(node, rbtree_t::node, node_next);
	 
	 if(key < data->node_key)
	 {
	    result = 1;
	 }
	 else if(key > data->node_key)
	 {
	    result = -1;
	 }
	 if (result < 0)
	 {
	    node = node->rb_left;
	 }
	 else if (result > 0)
	 {
	    node = node->rb_right;
	 }
	 else
	    return data;
      }
      return NULL;
   }
   
   void rbtree_t::reorganize(int *removed_key)
   {
      if(operation_mode != NHPC_RBTREE_NUM_MANAGED)
	 return;
      
      int last_key = *removed_key;
      rbtree_t::node *node = search_node(last_key);
      void **data = &(node->node_data);
      
      if(node == NULL)
	 return;
      
      while(node != NULL)
      {	 
	 last_key++;
	 
	 node = search_node(last_key);
	 if(node != NULL)
	 {
	    *data = node->node_data;
	    data = &(node->node_data);
	 }
      }
      
      *removed_key = count;
      last_assigned_key = count - 1;
   }
   
   int rbtree_t::insert_node(int key, rbtree_t::node **data)
   {
      struct rb_node **new_node = &(root.rb_node), *parent = NULL;

      while (*new_node) 
      {
	 node *this_node = container_of(*new_node, rbtree_t::node, node_next);
	 
	 parent = *new_node;
	 
	 if(key < this_node->node_key)
	 {
	    new_node = &((*new_node)->rb_right);
	 }
	 else if(key > this_node->node_key)
	 {
	    new_node = &((*new_node)->rb_left);
	 }
	 else 
	    return false;
      }      

      //(*data) = (rbtree_t::node *)malloc(sizeof(rbtree_t::node));
      (*data) = (rbtree_t::node *)alloc<rbtree_t::node>(1);
      memset((*data), 0, sizeof(rbtree_t::node));
      
      rb_link_node(&((*data)->node_next), parent, new_node);
      rb_insert_color(&((*data)->node_next), &root);
      count++;
      
      return true;	 
   }
   
   int rbtree_t::insert(void *in_data)
   {
      if(operation_mode == NHPC_RBTREE_STR)
	 return false;
      
      /* Create a new rbtree_t::node type and initialize values */
      rbtree_t::node *data;
      last_assigned_key++;
      int key = last_assigned_key;
      
      bool insert_node_status = insert_node(key, &data);
      if(!insert_node_status)
      {
	 return insert_node_status;
      }
      
      data->node_data = in_data;
      data->node_key = key;
      
      return key;      
   }
   
   int rbtree_t::insert(void *in_data, const char *key_str)
   {      
      if(operation_mode != NHPC_RBTREE_STR)
	 return false;
      
      /* Create a new rbtree_t::node type and initialize values */
      //rbtree_t::node *data = (rbtree_t::node *)malloc(sizeof(rbtree_t::node));
      rbtree_t::node *data = alloc<rbtree_t::node>(1);
      memset(data, 0, sizeof(rbtree_t::node));
      
      last_assigned_key++;
      data->node_data = in_data;
      data->node_key = last_assigned_key;
      data->key_pair = NULL;
      
      struct rb_node **new_node = &(root.rb_node), *parent = NULL;
      /* Figure out where to put new node */
      while (new_node && *new_node) 
      {
	 node *this_node = container_of(*new_node, node, node_next);
	 if(this_node == NULL)
	    return false;
	 
	 int key = strcmp(key_str, this_node->node_key_str);
	 
	 parent = *new_node;
	 if(key < 0)
	 {
	    new_node = &((*new_node)->rb_left);
	 }
	 else if(key > 0)
	 {
	    new_node = &((*new_node)->rb_right);
	 }
	 else 
	 {
	    LOG_ERROR("need to delete string in rbtree");
	    free(data);
	    return false;
	 }
      }
      
      nhpc_strcpy(&(data->node_key_str), key_str);
      
      /* Add new node and rebalance tree. */
      rb_link_node(&(data->node_next), parent, new_node);
      rb_insert_color(&(data->node_next), &root);
      count++;
      
      return NHPC_SUCCESS;
   }      
   
   int rbtree_t::insert(void *in_data, int key)
   {      
      if(operation_mode == NHPC_RBTREE_STR)
	 return false;
      
      /* Create a hash content structure for NUM_HASH mode */
      if(operation_mode == NHPC_RBTREE_NUM_HASH)
      {
	 hash_elem_t *hash_elem = alloc<hash_elem_t>(1);
	 //hash_elem_t *hash_elem = (hash_elem_t *)malloc(sizeof(hash_elem_t));
	 memset(hash_elem, 0, sizeof(hash_elem_t));
	 
	 hash_elem->data = in_data;
	 in_data = hash_elem;
	 
	 rbtree_t::node *data = search_node(key);
	 if(data != NULL)
	 {
	    int id = 2;
	    
	    hash_elem_t **new_elem = (hash_elem_t **)(&(data->node_data));
	    
	    while(*new_elem)
	    {
	       new_elem = &((*new_elem)->next);
	       
	       id++;
	    }
	    
	    *new_elem = hash_elem;
	    
	    return id;
	 }
      }
      
      /* Create a new rbtree_t::node type and initialize values */
      rbtree_t::node *data;
      last_assigned_key++;
      
      bool insert_node_status = insert_node(key, &data);
      if(!insert_node_status)
      {
	 return insert_node_status;
      }
      
      data->node_data = in_data;
      data->node_key = key;
      
      if(operation_mode == NHPC_RBTREE_NUM_HASH)
	 return 1;
      else 
	 return (data->node_key);
   }   
   
   int rbtree_t::erase(int key)
   {
      if(operation_mode == NHPC_RBTREE_STR)
	 return false;
      
      reorganize(&key);
      
      rbtree_t::node *data = rbtree_t::search_node(key);
      if(data){	 
	 rb_erase(&data->node_next, &root);
	 
	 if(operation_mode == NHPC_RBTREE_NUM_HASH)
	 {
	    hash_elem_t *hash_elem = (hash_elem_t *)data->node_data;
	    hash_elem_t *hash_elem_del;
	    
	    while(hash_elem != NULL)
	    {
	       hash_elem_del = hash_elem;
	       hash_elem = hash_elem->next;
	       
	       free(hash_elem_del);
	    }
	 }
	 
	 free(data);
	 count--;
	 
	 return true;
      }
      else
	 return false;
   }
   
   int rbtree_t::erase(int key, int subkey)
   {
      if(operation_mode == NHPC_RBTREE_STR)
	 return false;
      
      reorganize(&key);
      
      rbtree_t::node *data = rbtree_t::search_node(key);
      
      if(!data || subkey == 0)
	 return false;
      
      hash_elem_t *hash_elem = (hash_elem_t *)data->node_data;
      
      hash_elem_t **elem = (hash_elem_t **)&(data->node_data), *parent = NULL;
      int i = 0;
      
      while(*elem && i < (subkey - 1))
      {
	 parent = *elem;
	 elem = &((*elem)->next);
	 
	 i++;
      }
      
      /*
      cout<<(*elem)<<endl;
      cout<<(*elem)->next<<endl;
      
      if(subkey == 1 && !((*elem)->next))
      {
	 erase(key);
	 return true;
      }
	 
      cout<<i<<" "<<subkey<<endl;

      parent->next = (*elem)->next;
      
      return true;
       */
      
      if(subkey == 1)
      {
	 if(hash_elem->next == NULL)
	 {
	    erase(key);
	    return true;
	 }
	 
	 data->node_data = hash_elem->next;
	 free(hash_elem);
	 
	 return true;
      }
      
      for(int i = 0; i < (subkey - 2); i++)
      {
	 hash_elem = hash_elem->next;
	 
	 if(hash_elem == NULL)
	    return false;
      }
      
      hash_elem_t *hash_elem_del = hash_elem->next;
      
      if(hash_elem->next != NULL)
      {
	 hash_elem->next = hash_elem->next->next;
	 free(hash_elem_del);
      }      
      
      return true;
   }
   
   int rbtree_t::erase(int key, void *in_data)
   {
      if(operation_mode != NHPC_RBTREE_NUM_HASH)
	 return false;

      rbtree_t::node *node = search_node(key);
      if(node == NULL)
	 return false;
      
      hash_elem_t *hash_elem = (hash_elem_t *)(node->node_data);
      void *data_found = NULL;
      
      int count = 0;
      while(hash_elem != NULL)
      {
	 count++;
	 
	 if((hash_elem->data) == in_data)
	 {
	    data_found = in_data;
	    break;
	 }
	 
	 hash_elem = (hash_elem->next);
      }

      if(data_found != NULL)
      {
	 erase(key, count);
	 return count;
      }
      
      return false;
   }
   
   int rbtree_t::update(int key, void *new_in_data)
   {
      if(operation_mode == NHPC_RBTREE_STR)
	 return false;
      
      rbtree_t::node *data = search_node(key);
      if(data)
      {
	 data->node_data = new_in_data;
	 return true;
      }
      else
	 return false;
   }
   
   int rbtree_t::ret_count()
   {
      return count;
   }
   
   /* Experimental Code for adding string keys to rbtree */
   
   key_pair_t *rbtree_t::search_str(int key)
   {
      struct rb_node *node;
      
      node = rb_first(&root);
      key_pair_t *key_pair = NULL;
      int pos_count = 1;
      
      for (node = rb_first(&root); node; node = rb_next(node), pos_count++)
      {
	 rbtree_t::node *data = rb_entry(node, rbtree_t::node, node_next);
	 if(pos_count == key)
	 {
	    if(!(data->key_pair))
	       data->key_pair = new key_pair_t;
	    key_pair = data->key_pair;
	    key_pair->key = data->node_key_str;
	    key_pair->data = data->node_data;
	    
	    break;
	 }
      }      
      
      return key_pair;
   }
   
   void *rbtree_t::search(const char *key_str)
   {
      if(operation_mode != NHPC_RBTREE_STR) 
	 return NULL;
      
      struct rb_node *node = root.rb_node;
      
      while (node) 
      {
	 int result = 0;
	 
	 rbtree_t::node *data = container_of(node, rbtree_t::node, node_next);
	 if(data == NULL)
	    return NULL;
	 
	 int key = strcmp(key_str, data->node_key_str);
	 
	 if(key < 0)
	 {
	    node = node->rb_left;
	 }
	 else if(key > 0)
	 {
	    node = node->rb_right;
	 }
	 else
	    return data->node_data;
      }
      return NULL;
   }
   
   rbtree_t::node *rbtree_t::search_node(const char *key_str)
   {
      if(operation_mode != NHPC_RBTREE_STR)
	 return NULL;
      
      struct rb_node *node = root.rb_node;
      
      while (node) 
      {
	 int result = 0;
	 
	 rbtree_t::node *data = container_of(node, rbtree_t::node, node_next);
	 if(data == NULL)
	    return NULL;
	 
	 int key = strcmp(key_str, data->node_key_str);
	 
	 if(key < 0)
	 {
	    node = node->rb_left;
	 }
	 else if(key > 0)
	 {
	    node = node->rb_right;
	 }
	 else
	    return data;
      }
      return NULL;
   }  
   
   void *rbtree_t::search(int key, int subkey)
   {
      if(operation_mode != NHPC_RBTREE_NUM_HASH)
	 return NULL;
      
      rbtree_t::node *data = search_node(key);
      
      if(data == NULL)
	 return NULL;
      
      hash_elem_t *hash_elem = (hash_elem_t *)data->node_data;
      for(int i = 1; i < subkey && hash_elem != NULL; i++)
      {
	 hash_elem = hash_elem->next;
      }
      
      if(hash_elem == NULL)
	 return NULL;
      
      return (hash_elem->data);
   }
      
   int rbtree_t::erase(const char *key_str)
   {
      if(operation_mode != NHPC_RBTREE_STR)
	 return NHPC_FAIL;
      
      rbtree_t::node *data = rbtree_t::search_node(key_str);
      if(data){
	 rb_erase(&(data->node_next), &root);
	 //nhpc_deallocate_str(data->node_key_str);
	 delete[] (data->node_key_str);
	 if(data->key_pair)
	    delete (data->key_pair);
	 free(data);
	 count--;
	 return NHPC_SUCCESS;
      }
      else
	 return NHPC_FAIL;
   }   
};

static void __rb_rotate_left(struct rb_node *node, struct rb_root *root)
{
   struct rb_node *right = node->rb_right;
   struct rb_node *parent = rb_parent(node);
   
   if ((node->rb_right = right->rb_left))
      rb_set_parent(right->rb_left, node);
   right->rb_left = node;
   
   rb_set_parent(right, parent);
   
   if (parent)
   {
      if (node == parent->rb_left)
	 parent->rb_left = right;
      else
	 parent->rb_right = right;
   }
   else
      root->rb_node = right;
   rb_set_parent(node, right);
}

static void __rb_rotate_right(struct rb_node *node, struct rb_root *root)
{
   struct rb_node *left = node->rb_left;
   struct rb_node *parent = rb_parent(node);
   
   if ((node->rb_left = left->rb_right))
      rb_set_parent(left->rb_right, node);
   left->rb_right = node;
   
   rb_set_parent(left, parent);
   
   if (parent)
   {
      if (node == parent->rb_right)
	 parent->rb_right = left;
      else
	 parent->rb_left = left;
   }
   else
      root->rb_node = left;
   rb_set_parent(node, left);
}

void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
   struct rb_node *parent, *gparent;
   
   while ((parent = rb_parent(node)) && rb_is_red(parent))
   {
      gparent = rb_parent(parent);
      
      if (parent == gparent->rb_left)
      {
	 {
	    register struct rb_node *uncle = gparent->rb_right;
	    if (uncle && rb_is_red(uncle))
	    {
	       rb_set_black(uncle);
	       rb_set_black(parent);
	       rb_set_red(gparent);
	       node = gparent;
	       continue;
	    }
	 }
	 
	 if (parent->rb_right == node)
	 {
	    register struct rb_node *tmp;
	    __rb_rotate_left(parent, root);
	    tmp = parent;
	    parent = node;
	    node = tmp;
	 }
	 
	 rb_set_black(parent);
	 rb_set_red(gparent);
	 __rb_rotate_right(gparent, root);
      } else {
	 {
	    register struct rb_node *uncle = gparent->rb_left;
	    if (uncle && rb_is_red(uncle))
	    {
	       rb_set_black(uncle);
	       rb_set_black(parent);
	       rb_set_red(gparent);
	       node = gparent;
	       continue;
	    }
	 }
	 
	 if (parent->rb_left == node)
	 {
	    register struct rb_node *tmp;
	    __rb_rotate_right(parent, root);
	    tmp = parent;
	    parent = node;
	    node = tmp;
	 }
	 
	 rb_set_black(parent);
	 rb_set_red(gparent);
	 __rb_rotate_left(gparent, root);
      }
   }
   
   rb_set_black(root->rb_node);
}

static void __rb_erase_color(struct rb_node *node, struct rb_node *parent, struct rb_root *root)
{
   struct rb_node *other;
   
   while ((!node || rb_is_black(node)) && node != root->rb_node)
   {
      if (parent->rb_left == node)
      {
	 other = parent->rb_right;
	 if (rb_is_red(other))
	 {
	    rb_set_black(other);
	    rb_set_red(parent);
	    __rb_rotate_left(parent, root);
	    other = parent->rb_right;
	 }
	 if ((!other->rb_left || rb_is_black(other->rb_left)) &&
	     (!other->rb_right || rb_is_black(other->rb_right)))
	 {
	    rb_set_red(other);
	    node = parent;
	    parent = rb_parent(node);
	 }
	 else
	 {
	    if (!other->rb_right || rb_is_black(other->rb_right))
	    {
	       rb_set_black(other->rb_left);
	       rb_set_red(other);
	       __rb_rotate_right(other, root);
	       other = parent->rb_right;
	    }
	    rb_set_color(other, rb_color(parent));
	    rb_set_black(parent);
	    rb_set_black(other->rb_right);
	    __rb_rotate_left(parent, root);
	    node = root->rb_node;
	    break;
	 }
      }
      else
      {
	 other = parent->rb_left;
	 if (rb_is_red(other))
	 {
	    rb_set_black(other);
	    rb_set_red(parent);
	    __rb_rotate_right(parent, root);
	    other = parent->rb_left;
	 }
	 if ((!other->rb_left || rb_is_black(other->rb_left)) &&
	     (!other->rb_right || rb_is_black(other->rb_right)))
	 {
	    rb_set_red(other);
	    node = parent;
	    parent = rb_parent(node);
	 }
	 else
	 {
	    if (!other->rb_left || rb_is_black(other->rb_left))
	    {
	       rb_set_black(other->rb_right);
	       rb_set_red(other);
	       __rb_rotate_left(other, root);
	       other = parent->rb_left;
	    }
	    rb_set_color(other, rb_color(parent));
	    rb_set_black(parent);
	    rb_set_black(other->rb_left);
	    __rb_rotate_right(parent, root);
	    node = root->rb_node;
	    break;
	 }
      }
   }
   if (node)
      rb_set_black(node);
}

void rb_erase(struct rb_node *node, struct rb_root *root)
{
   struct rb_node *child, *parent;
   int color;
   
   if (!node->rb_left)
      child = node->rb_right;
   else if (!node->rb_right)
      child = node->rb_left;
   else
   {
      struct rb_node *old = node, *left;
      
      node = node->rb_right;
      while ((left = node->rb_left) != NULL)
	 node = left;
      
      if (rb_parent(old)) {
	 if (rb_parent(old)->rb_left == old)
	    rb_parent(old)->rb_left = node;
	 else
	    rb_parent(old)->rb_right = node;
      } else
	 root->rb_node = node;
      
      child = node->rb_right;
      parent = rb_parent(node);
      color = rb_color(node);
      
      if (parent == old) {
	 parent = node;
      } else {
	 if (child)
	    rb_set_parent(child, parent);
	 parent->rb_left = child;
	 
	 node->rb_right = old->rb_right;
	 rb_set_parent(old->rb_right, node);
      }
      
      node->rb_parent_color = old->rb_parent_color;
      node->rb_left = old->rb_left;
      rb_set_parent(old->rb_left, node);
      
      goto color;
   }
   
   parent = rb_parent(node);
   color = rb_color(node);
   
   if (child)
      rb_set_parent(child, parent);
   if (parent)
   {
      if (parent->rb_left == node)
	 parent->rb_left = child;
      else
	 parent->rb_right = child;
   }
   else
      root->rb_node = child;
   
color:
   if (color == RB_BLACK)
      __rb_erase_color(child, parent, root);
}

static void rb_augment_path(struct rb_node *node, rb_augment_f func, void *data)
{
   struct rb_node *parent;
   
up:
   func(node, data);
   parent = rb_parent(node);
   if (!parent)
      return;
   
   if (node == parent->rb_left && parent->rb_right)
      func(parent->rb_right, data);
   else if (parent->rb_left)
      func(parent->rb_left, data);
   
   node = parent;
   goto up;
}

/*
 * after inserting @node into the tree, update the tree to account for
 * both the new entry and any damage done by rebalance
 */
void rb_augment_insert(struct rb_node *node, rb_augment_f func, void *data)
{
   if (node->rb_left)
      node = node->rb_left;
   else if (node->rb_right)
      node = node->rb_right;
   
   rb_augment_path(node, func, data);
}

/*
 * before removing the node, find the deepest node on the rebalance path
 * that will still be there after @node gets removed
 */
struct rb_node *rb_augment_erase_begin(struct rb_node *node)
{
   struct rb_node *deepest;
   
   if (!node->rb_right && !node->rb_left)
      deepest = rb_parent(node);
   else if (!node->rb_right)
      deepest = node->rb_left;
   else if (!node->rb_left)
      deepest = node->rb_right;
   else {
      deepest = rb_next(node);
      if (deepest->rb_right)
	 deepest = deepest->rb_right;
      else if (rb_parent(deepest) != node)
	 deepest = rb_parent(deepest);
   }
   
   return deepest;
}

/*
 * after removal, update the tree to account for the removed entry
 * and any rebalance damage.
 */
void rb_augment_erase_end(struct rb_node *node, rb_augment_f func, void *data)
{
   if (node)
      rb_augment_path(node, func, data);
}

/*
 * This function returns the first node (in sort order) of the tree.
 */
struct rb_node *rb_first(const struct rb_root *root)
{
   struct rb_node	*n;
   
   n = root->rb_node;
   if (!n)
      return NULL;
   while (n->rb_left)
      n = n->rb_left;
   return n;
}

struct rb_node *rb_last(const struct rb_root *root)
{
   struct rb_node	*n;
   
   n = root->rb_node;
   if (!n)
      return NULL;
   while (n->rb_right)
      n = n->rb_right;
   return n;
}

struct rb_node *rb_next(const struct rb_node *node)
{
   struct rb_node *parent;
   
   if (rb_parent(node) == node)
      return NULL;
   
   /* If we have a right-hand child, go down and then left as far
    as we can. */
   if (node->rb_right) 
   {
      node = node->rb_right; 
      while (node->rb_left)
	 node=node->rb_left;
      return (struct rb_node *)node;
   }
   
   /* No right-hand children.  Everything down and left is
    smaller than us, so any 'next' node must be in the general
    direction of our parent. Go up the tree; any time the
    ancestor is a right-hand child of its parent, keep going
    up. First time it's a left-hand child of its parent, said
    parent is our 'next' node. */
   while ((parent = rb_parent(node)) && node == parent->rb_right)
      node = parent;
   
   return parent;
}

struct rb_node *rb_prev(const struct rb_node *node)
{
   struct rb_node *parent;
   
   if (rb_parent(node) == node)
      return NULL;
   
   /* If we have a left-hand child, go down and then right as far
    as we can. */
   if (node->rb_left) 
   {
      node = node->rb_left; 
      while (node->rb_right)
	 node=node->rb_right;
      return (struct rb_node *)node;
   }
   
   /* No left-hand children. Go up till we find an ancestor which
    is a right-hand child of its parent */
   while ((parent = rb_parent(node)) && node == parent->rb_left)
      node = parent;
   
   return parent;
}

void rb_replace_node(struct rb_node *victim, struct rb_node *new_node, struct rb_root *root)
{
   struct rb_node *parent = rb_parent(victim);
   
   /* Set the surrounding nodes to point to the replacement */
   if (parent) {
      if (victim == parent->rb_left)
	 parent->rb_left = new_node;
      else
	 parent->rb_right = new_node;
   } 
   else 
   {
      root->rb_node = new_node;
   }
   if (victim->rb_left)
      rb_set_parent(victim->rb_left, new_node);
   if (victim->rb_right)
      rb_set_parent(victim->rb_right, new_node);
   
   /* Copy the pointers/colour from the victim to the replacement */
   *new_node = *victim;
}
