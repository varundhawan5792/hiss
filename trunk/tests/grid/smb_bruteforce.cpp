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

#include <stdlib.h>
#include <iostream>

#include <include/neweraHPC.h>

using namespace std;
using namespace neweraHPC;

int main(int argc, char **argv)
{
   if(argc < 5)
   {
      cout<<"Usage: ./simple_client server_host server_port file.blend start_frame max_frame\n";
      return 1;
   }
   
   const char *grid_uid;
   
   nhpc_status_t nrv = nhpc_register_to_server(&grid_uid, argv[1], argv[2]);
   
   if(nrv != NHPC_SUCCESS)
   {
      cout<<"Registration failed with server\n";
      return 1;
   }
   else 
   {
      cout<<"Grid registration uid: "<<grid_uid<<endl;
      cout<<"Registration done\n";
   }
   
   nrv = nhpc_send_file(grid_uid, argv[1], argv[2], argv[3]);
   cout<<nrv<<endl;
   if(nrv != NHPC_SUCCESS)
   {
      cout<<"File Upload Failed"<<endl;
      cout<<argv[3]<<endl;
      exit(1);
   }
   
   nhpc_instruction_set_t *instruction_set;
   nhpc_create_instruction(&instruction_set, GRID_RANGE_PLUGIN);
   
   int start = nhpc_strtoi(argv[4]), end = nhpc_strtoi(argv[5]);
   
   string_t *string = nhpc_substr((const char *)argv[3], '/');
   
   nhpc_add_argument(instruction_set, COMMAND, "/newerahpc/smblogin");
   nhpc_add_argument(instruction_set, GRID_FILE, string->strings[string->count - 1]);
   nhpc_add_argument(instruction_set, LITERAL, "-b");
   nhpc_add_argument(instruction_set, LITERAL, "-f");
   nhpc_add_argument(instruction_set, RANGE, &start, &end);
   
   time_t *tm;
   time(tm);
   cout<<"Starting Job at "<<ctime(tm)<<endl;
   nrv = nhpc_send_instruction(grid_uid, argv[1], argv[2], instruction_set);
   if(nrv != NHPC_SUCCESS)
   {
      cout<<"Instruction failed"<<endl;
   }
   time(tm);
   cout<<"Finishing Job at "<<ctime(tm)<<endl;
   
   cout<<"Plugin Uploaded"<<endl;
   
   delete[] grid_uid;
   
   return 0;
}
