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

#ifndef _ALLOC_H_
#define _ALLOC_H_

#include <errno.h>
#ifdef linux
#include <malloc.h>
#endif
#include <stdlib.h>

#include "error.h"
#include "strings_pool.h"

using namespace neweraHPC;

template <class T>
T *alloc(size_t _size)
{
   _size = sizeof(T) * _size;
   T *ptr;
   
   do{
      ptr = (T *)malloc(_size);
   }while(ptr == NULL);
   
   return ptr;
}

#ifdef ENABLE_GARBAGE_COLLECTOR
void *operator new(std::size_t size);
void *operator new[](std::size_t size);

void operator delete(void *ptr);
void operator delete[](void *ptr);
#endif

#endif
