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

#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>

extern bool log_on[3];

using namespace std;

namespace neweraHPC
{
   //void nhpc_perror(nhpc_status_t status, const char *message);
      
   class Log
   {
   public:
      enum Level { Debug, Warning, Info, Error };
      
      static ostream& GetStream() { return cout; }
      static bool IsLevelActive(Level l) { return true; }
   };
   
   struct MyObject 
   {
      int a, b;
   };
   
#define LOG_ERROR(M)   do { if (Log::IsLevelActive(Log::Error))   (Log::GetStream() << "ERR: " << M << "\n"); } while (false)
#define LOG_INFO(M)    do { if (Log::IsLevelActive(Log::Info) && log_on[Log::Info])    (Log::GetStream() << "INF: " << M << "\n"); } while (false)
#define LOG_WARNING(M) do { if (Log::IsLevelActive(Log::Warning) && log_on[Log::Warning]) (Log::GetStream() << "WRN: " << M << "\n"); } while (false)
#define LOG_DEBUG(M)   do { if (Log::IsLevelActive(Log::Debug) && log_on[Log::Debug])   (Log::GetStream() << "DBG: " << M << "\n"); } while (false)
   
   ostream& operator<<(ostream& ostr, const MyObject& obj);
};

#endif
