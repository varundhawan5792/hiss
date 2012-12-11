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

#include <sys/stat.h>
#include <iostream>
#include <dirent.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/file.h>
#include <include/strings.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t nhpc_file_size(const char *file_path, nhpc_size_t *size)
   {
      nhpc_status_t nrv = nhpc_fileordirectory(file_path);
      
      if(nrv != NHPC_FILE)
	 *size = 0;
      else 
      {
	 struct stat *buffer = new struct stat;
	 nhpc_status_t nrv = stat(file_path, buffer);
	 
	 *size = buffer->st_size;
	 
	 delete buffer;
      }
      
      return nrv;
   }
   
   nhpc_status_t nhpc_fileordirectory(const char *file_path)
   {
      struct stat *buffer = new struct stat;
      nhpc_status_t nrv = stat(file_path, buffer);
      
      if(nrv != 0)
      {
	 nrv = NHPC_FILE_NOT_FOUND;
      }
      else if(!S_ISDIR(buffer->st_mode))
	 nrv = NHPC_FILE;
      else 
	 nrv = NHPC_DIRECTORY;
      
      delete buffer;
      return nrv;
   }
   
   nhpc_status_t nhpc_filecopy(const char *dst, const char *src)
   {
      if(dst == NULL || src == NULL)
	 return NHPC_FAIL;
      
      if(nhpc_fileordirectory(dst) != NHPC_FILE_NOT_FOUND || nhpc_fileordirectory(src) != NHPC_FILE)
	 return NHPC_FAIL;
      
      FILE *fp_src = fopen(src, "r");
      if(!fp_src)
	 return NHPC_FAIL;
      
      FILE *fp_dst = fopen(dst, "w+");
      if(!fp_dst)
      {
	 fclose(fp_src);
	 return NHPC_FAIL;
      }
      
      nhpc_size_t size;
      char buffer[1000];
      while(!feof(fp_src))
      {
	 size = fread(buffer, 1, sizeof(buffer), fp_src); 
	 fwrite(buffer, 1, size, fp_dst);
      }
      
      fclose(fp_src);
      fclose(fp_dst);
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t nhpc_delete_file_dir(const char *target_path)
   {
      nhpc_status_t nrv = nhpc_fileordirectory(target_path);
      
      if(nrv == NHPC_FILE)
      {
	 
      }
      else if(nrv == NHPC_DIRECTORY)
      {
	 
      }
      
      return NHPC_FAIL;
   }
   
   nhpc_status_t nhpc_create_tmp_file_or_dir(const char **new_file_dir, const char *target_dir, 
					     int target_type, const char *file_name)
   {
      *new_file_dir = NULL;
      
      if(target_type != NHPC_FILE && target_type != NHPC_DIRECTORY)
      {
	 return NHPC_FAIL;
      }
      
      const char *final_target;
      if(target_dir != NULL)
      {
	 nhpc_strcpy((char **)&final_target, target_dir);
	 string_t *string = nhpc_substr(final_target, '/');
	 nhpc_string_delete((char *)final_target);
	 
	 final_target = nhpc_strconcat("/", string->strings[0]);
	 
	 const char *tmp_str;
	 
	 for(int i = 1; i < string->count; i++)
	 {
	    tmp_str = nhpc_strconcat(final_target, "/");
	    nhpc_string_delete((char *)final_target);
	    
	    final_target = nhpc_strconcat(tmp_str, string->strings[i]);
	    nhpc_string_delete((char *)tmp_str);
	 }
	 
	 nhpc_string_delete(string);
      }
      else 
	 final_target = "/tmp";
      
      nhpc_status_t nrv = nhpc_fileordirectory(final_target);
      if(nrv != NHPC_DIRECTORY)
      {
	 if(target_dir != NULL)
	    nhpc_string_delete((char *)final_target);
	 return NHPC_FAIL;
      }
      
      const char *tmp_target = nhpc_strconcat(final_target, "/");
      if(target_dir != NULL   )
	 nhpc_string_delete((char *)final_target);
      final_target = tmp_target;
      
      bool unique_found = false;
      const char *unique_name;
      const char *target_name;
      
      if(file_name != NULL)
      {
	 target_name = nhpc_strconcat(final_target, file_name);
	 
	 int check_type = nhpc_fileordirectory(target_name);
	 
	 if(check_type == target_type)
	 {
	    *new_file_dir = target_name;
	    return NHPC_SUCCESS;
	 }
      }
      else 
      {
	 while(unique_found == false)
	 {
	    unique_name = nhpc_random_string(7);
	    target_name = nhpc_strconcat(final_target, unique_name);
	    if(nhpc_fileordirectory(target_name) != NHPC_FILE_NOT_FOUND)
	    {
	       nhpc_string_delete((char *)target_name);
	    }
	    else 
	    {
	       unique_found = true;
	    }
	    
	    nhpc_string_delete((char *)unique_name);
	 }
      }
      
      if(target_type == NHPC_DIRECTORY)
      {
	 int rv = mkdir(target_name, 0777);	 
	 if(rv == -1)
	 {
	    nhpc_string_delete((char *)final_target);
	    return NHPC_FAIL;
	 }
	 
	 const char *tmp_str = nhpc_strconcat(target_name, "/");
	 nhpc_string_delete((char *)final_target);
	 target_name = tmp_str;
      }
      else if(target_type == NHPC_FILE)
      {
	 FILE *fp = fopen(target_name, "w+");
	 fclose(fp);
      }
      
      *new_file_dir = target_name;
      return NHPC_SUCCESS;
   }
   
   string_t *nhpc_get_file_list(const char *dir, int mode)
   {
      DIR *_dir;
      struct dirent *ent;
      
      _dir = opendir(dir);
      
      string_t *string = NULL;
      
      if(_dir != NULL)
      {
         string = new string_t;
	 
         while((ent = readdir(_dir)) != NULL)
         {
            if(mode == NHPC_VISIBLE_DIR_CHILD)
            {
               if(nhpc_strcmp(ent->d_name, ".*"))
                  continue;
            }
	    
            string->count++;
         }
	 
         string->strings = new char* [string->count + 1];
         string->strings[string->count + 1] = NULL;
	 
         closedir(_dir);
         _dir = opendir(dir);
	 
         int i = 0;
         while((ent = readdir(_dir)) != NULL)
         {
            if(mode == NHPC_VISIBLE_DIR_CHILD)
            {
               if(nhpc_strcmp(ent->d_name, ".*"))
                  continue;
            }
	    
            nhpc_strcpy(&(string->strings[i]), ent->d_name);
	    
            i++;
         }
      }
      
      return string;
   }
};
