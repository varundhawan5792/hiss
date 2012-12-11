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

#ifndef _NEWERAHPC_H_
#define _NEWERAHPC_H_

#include "file.h"
#include "rbtree.h"
#include "list.h"
#include "thread.h"
#include "network.h"
#include "strings.h"
#include "general.h"
#include "grid.h"
#include "system.h"
#include "alloc.h"
#include "http.h"
#include "web_ui.h"
#include "json.h"

#include <iostream>

extern neweraHPC::rbtree_t cmdline_arguments;

using namespace std;

namespace neweraHPC
{
   void neweraHPC_init(int argc, char **argv);
};

#endif