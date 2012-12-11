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

#ifndef _FILE_H_
#define _FILE_H_

#include <fstream>
#include <iostream>
#include <sys/stat.h>

#include "constants.h"
#include "strings.h"

namespace neweraHPC
{
   nhpc_status_t nhpc_file_size(const char *file_path, nhpc_size_t *size);

   nhpc_status_t nhpc_fileordirectory(const char *file_path);
   
   nhpc_status_t nhpc_filecopy(const char *dst, const char *src);
   
   nhpc_status_t nhpc_create_tmp_file_or_dir(const char **new_file_dir, const char *target_dir, 
					     int target_type, const char *file_name = NULL);

   string_t *nhpc_get_file_list(const char *dir, int mode);
   
   nhpc_status_t nhpc_delete_file_dir(const char *target_path);
};

#endif
