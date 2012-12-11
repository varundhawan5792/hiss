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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/constants.h>
#include <include/error.h>

bool log_on[3];

using namespace std;

namespace neweraHPC
{
   void nhpc_perror(nhpc_status_t status, const char *message)
   {
      if(status == NHPC_TIMEUP)
	 cout<<message<<": Operation timedout"<<endl;
   }
   
   
   ostream& operator<<(ostream& ostr, const MyObject& obj) {
      ostr << "(a=" << obj.a << ", b=" << obj.b << ")";
      return ostr;
   }   
};