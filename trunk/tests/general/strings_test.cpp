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

#include <stdlib.h>
#include <iostream>

#include <include/strings.h>
#include <include/headers.h>

using namespace std;
using namespace neweraHPC;

int main()
{
   nhpc_status_t nrv;
   const char *s1;
   const char *s2;
   
   cout<<"Testing new strfind api"<<endl;
   cout<<"Passing string: /app/neweraHPC?q=test&?t2=test2 testing for ?"<<endl;
   int pos = nhpc_strfind("/app/neweraHPC?q=test&?t2=test2", '?');
   cout << "? found at: " << pos << endl;
   int pos2 = nhpc_strfind("/app/neweraHPC?q=test&?t2=test2", '?', pos + 1);
   cout << "next ? found at: " << pos2 << endl;
   cout << "Testing new substr api between limits: " <<nhpc_substr("/app/neweraHPC?q=test&?t2=test2", pos, pos2)<<endl;
   
   s1 = "GET";
   s2 = "GET";
   cout<<"s1: "<<s1<<" s2: "<<s2<<endl;
   nrv = nhpc_strcmp(s1, s2);
   if(nrv == NHPC_FAIL)
      cout<<"string not found"<<endl;
   else
      cout<<"string found"<<endl;
   cout<<endl;
   
   s1 = "GET abc";
   s2 = "GET";
   cout<<"s1: "<<s1<<" s2: "<<s2<<endl;
   nrv = nhpc_strcmp(s1, s2);
   if(nrv == NHPC_FAIL)
      cout<<"string not found"<<endl;
   else
      cout<<"string found"<<endl;
   cout<<endl;

   s1 = "GET abc";
   s2 = "GET*";
   cout<<"s1: "<<s1<<" s2: "<<s2<<endl;
   nrv = nhpc_strcmp(s1, s2);
   if(nrv == NHPC_FAIL)
      cout<<"string not found"<<endl;
   else
      cout<<"string found"<<endl;
   cout<<endl;

   s1 = "ASB GET abc HTTP/1.1";
   s2 = "*GET";
   cout<<"s1: "<<s1<<" s2: "<<s2<<endl;
   nrv = nhpc_strcmp(s1, s2);
   if(nrv == NHPC_FAIL)
      cout<<"string not found"<<endl;
   else
      cout<<"string found"<<endl;
   cout<<endl;

   s1 = "GET abc";
   s2 = "**GET*a***bd***c**";
   cout<<"s1: "<<s1<<" s2: "<<s2<<endl;
   nrv = nhpc_strcmp(s1, s2);
   if(nrv == NHPC_FAIL)
      cout<<"string not found"<<endl;
   else
      cout<<"string found"<<endl;
   cout<<endl;
   
   s1 = "";
   s2 = "";
   cout<<"s1: "<<s1<<" s2: "<<s2<<endl;
   nrv = nhpc_strcmp(s1, s2);
   if(nrv == NHPC_FAIL)
      cout<<"string not found"<<endl;
   else
      cout<<"string found"<<endl;
   cout<<endl;

   s1 = "Varun Dhawan";
   s2 = "*varun*";
   cout<<"s1: "<<s1<<" s2: "<<s2<<endl;
   nrv = nhpc_strcmpi(s1, s2);
   if(nrv == NHPC_FAIL)
      cout<<"string not found"<<endl;
   else
      cout<<"string found"<<endl;
   cout<<endl;
   
   string_t *string = nhpc_substr("My name is   varun", ' ');
   for(int i = 0; i < string->count; i++)
      cout<<"Substr_1: "<<string->strings[i]<<endl;

   string = nhpc_substr("   My name is   varun  ", '&');

   for(int i = 0; i < string->count; i++)
      cout<<"Substr: "<<string->strings[i]<<endl;

   string = nhpc_substr("My name is   varun   ", ' ');
   string = nhpc_substr("  My name is varun  end ", ' ');
   for(int i = 0; i < string->count; i++)
   {
      cout << string->strings[i] << endl;
   }
   exit(0);
      
   cout<<endl<<"Concatenating strings:"<<endl;
   
   s1 = "Varun";
   s2 = " Mittal";
   const char *s3 = nhpc_strconcat(s1, s2);
   cout<<s1<<" len "<<strlen(s1)<<endl;
   cout<<s2<<" len "<<strlen(s2)<<endl;
   cout<<s3<<" len "<<strlen(s3)<<endl;
   
   cout<<nhpc_itostr(123)<<endl;
   
   cout<<"Generating random strings of length 5"<<endl;
   for(int i = 0; i < 10; i++)
   {
      cout<<nhpc_random_string(5)<<endl;
   }
   
   cout<<"Testing strcpy"<<endl;
   char *test_cpy;
   nhpc_strcpy(&test_cpy, "hi my name is varun");
   cout<<test_cpy<<" len: "<<strlen(test_cpy)<<endl;
   
   cout<<nhpc_strtoi("12390")<<endl;
   
   return 0;
}
