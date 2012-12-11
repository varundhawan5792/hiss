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

#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <unistd.h>

typedef int    nhpc_status_t;
typedef size_t nhpc_size_t;

#define NHPC_SUCCESS 1
#define NHPC_FAIL 0

#define NHPC_ON  1
#define NHPC_OFF 0

#define NHPC_MODE_NORMAL 0
#define NHPC_MODE_SERVER 1

/* Network specific constants */
#define NHPC_BUFFER_SIZE 1000

#define NHPC_POLLING_READ  1
#define NHPC_POLLING_WRITE 1

#define NHPC_BINDING_SOCKET    1
#define NHPC_CONNECTING_SOCKET 2
#define NHPC_INCOMPLETE 4
#define NHPC_EOF 8

#define NHPC_KEEP_ALIVE SO_KEEPALIVE

#define NHPC_NONBLOCK  1
#define NHPC_REUSEADDR 2

/* Polling and timeout specific constants */
#define NHPC_TIMEUP  2
#define NHPC_TIMEOUT 30

/* File and directory specific constants */
#define NHPC_FILE_NOT_FOUND 0
#define NHPC_FILE 1
#define NHPC_DIRECTORY 2
#define NHPC_VISIBLE_DIR_CHILD 0
#define NHPC_ALL_DIR_CHILD 1

/* Grid specific constants */
#define PLUGIN_REQUEST_PENDING 0
#define PLUGIN_REQUEST_SENT 1
#define PLUGIN_UNDER_PROCESSING 0
#define PLUGIN_INSTALLED 1
#define PLUGIN_INSTALL_FAILED

typedef void *(*fnc_ptr_t)(void *);
typedef void *(*fnc_ptr_two_t)(void *, void *);
typedef nhpc_status_t (*fnc_ptr_nhpc_t)(void *);
typedef nhpc_status_t (*fnc_ptr_nhpc_two_t)(void *, void *);
typedef nhpc_status_t (*fnc_ptr_nhpc_plugin_t)(void *, void *, void *, void **);

typedef void *(*fnc_ptr_int_t)(int *);

#endif
