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
#include <fstream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/grid.h>
#include <include/network.h>
#include <include/file.h>

using namespace std;

namespace neweraHPC
{
   nhpc_status_t nhpc_check_nxi(const char *file_path)
   {
      nhpc_status_t nrv = nhpc_strcmp(file_path, "*.nxi");
      
      if(nrv == NHPC_SUCCESS)
      {
	 nrv = nhpc_fileordirectory(file_path);
	 if(nrv == NHPC_FILE)
	    nrv = NHPC_SUCCESS;
	 else 
	    nrv = NHPC_FAIL;
      }
      
      return nrv;
   }
   
   nhpc_status_t nhpc_nxitodll(const char **dll_path, const char *file_path)
   {
      nhpc_status_t nrv;
      
      const char *directory;
      nrv = nhpc_create_tmp_file_or_dir(&directory, "/tmp/neweraHPC", NHPC_DIRECTORY);
      if(nrv != NHPC_SUCCESS)
	 return nrv;
      
      const char *exec_file = nhpc_strconcat(directory, "exec");
      FILE *fp_exec = fopen(exec_file, "w+");
      chmod(exec_file, S_IRWXU | S_IRWXG | S_IRWXO);
      
      const char *script_type = "#!/bin/bash\n\n";
      fwrite(script_type, 1, strlen(script_type), fp_exec);
      
      fwrite("cd ", 1, 3, fp_exec);
      fwrite(directory, 1, strlen(directory), fp_exec);
      fwrite("\n", 1, 1, fp_exec);
      
      ifstream fp_nxi(file_path, ios::in);
      string line;
      
      const char *bash_substitutes = " &> /dev/null\n if [ \"$?\" != \"0\" ]; then\n exit 1\n fi\n";
      
      while(getline(fp_nxi, line))
      {
	 const char *c_line = line.c_str();
	 if(nhpc_strcmp(c_line, "build-command:*") == NHPC_FAIL)
	    break;
	 
	 const char *command = c_line + 14;
	 
	 fwrite(command, 1, strlen(command), fp_exec);
	 fwrite(bash_substitutes, 1, strlen(bash_substitutes), fp_exec);
      }
      
      const char *bin_dir = nhpc_strconcat(directory, "bin");
      mkdir(bin_dir, 0777);
      
      fclose(fp_exec);
      
      FILE *fp_out = NULL;
      
      while(getline(fp_nxi, line))
      {
	 const char *c_line = line.c_str();
   
	 if(nhpc_strcmp(c_line, "<<file:over>>") == NHPC_SUCCESS)
	 {
	    if(fp_out != NULL)
	    {
	       fclose(fp_out);
	       fp_out = NULL;
	    }
	 }
	 else if(nhpc_strcmp(c_line, "<<file:*>>") == NHPC_SUCCESS)
	 {
	    nhpc_size_t len = strlen(c_line) - 2 - 6;
	    
	    if(len == 1)continue;
	    
	    char *file_name = new char [len];
	    memcpy(file_name, c_line + 7, len - 1);
	    file_name[len - 1] = '\0';
	    
	    char *tmp_name = nhpc_strconcat(directory, file_name);
	    
	    string_t *string = nhpc_substr(tmp_name, '/');
	    
	    const char *final_target = nhpc_strconcat("/", string->strings[0]);
	    const char *tmp_str;
	    
	    for(int i = 1; i < (string->count - 1); i++)
	    {
	       tmp_str = nhpc_strconcat(final_target, "/");
	       nhpc_string_delete((char *)final_target);
	       
	       final_target = nhpc_strconcat(tmp_str, string->strings[i]);
	       nhpc_string_delete((char *)tmp_str);
	       
	       if(nhpc_fileordirectory(final_target) != NHPC_FILE_NOT_FOUND);
		  mkdir(final_target, 0777);
	    }	    
	    	    
	    fp_out = fopen(tmp_name, "w+");
	 }
	 else 
	 {
	    if(fp_out != NULL)
	    {
	       fwrite(c_line, 1, strlen(c_line), fp_out);
	       fwrite("\n", 1, 1, fp_out);
	    }
	 }	 
      }
      
      const char *tmp = nhpc_strconcat("cd ", directory);
      const char *exec_cmd = tmp;
      exec_cmd = nhpc_strconcat(tmp, " && ./exec");
      nhpc_string_delete((char *)tmp);
      int rv = system(exec_cmd);
      nhpc_string_delete((char *)exec_cmd);
      
      if(rv == 0)
      {
	 const char *dll_file = nhpc_strconcat(directory, "bin/libplugin.so");
	 if(nhpc_fileordirectory(dll_file) == NHPC_FILE)
	 {
	    *dll_path = dll_file;
	    return NHPC_SUCCESS;
	 }
	 
	 nhpc_string_delete((char *)dll_file);
      }
      
      return NHPC_FAIL;
   }
};