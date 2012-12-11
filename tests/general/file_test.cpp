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

#include <include/neweraHPC.h>

using namespace std;
using namespace neweraHPC;

int main()
{
   const char *tmp_file;
   nhpc_status_t nrv;
   
   nrv = nhpc_create_tmp_file_or_dir(&tmp_file, "/tmp/neweraHPC", NHPC_FILE);
   if(nrv == NHPC_SUCCESS)
   {
      cout<<tmp_file<<endl;
      nhpc_string_delete((char *)tmp_file);
   }
   
   nrv = nhpc_create_tmp_file_or_dir(&tmp_file, "/tmp/neweraHPC", NHPC_DIRECTORY);
   if(nrv == NHPC_SUCCESS)
   {
      cout<<tmp_file<<endl;
      nhpc_string_delete((char *)tmp_file);
   }

   return 0;
}
