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

#ifndef _WEB_UI_H_
#define _WEB_UI_H_

#include "network_data.h"
#include "json.h"

namespace neweraHPC
{
   extern rbtree_t *app_handlers;
   extern const char *ui_temp_dir;
   
   struct app_details_t
   {
      fnc_ptr_nhpc_two_t func_trigger_local;
      rbtree_t *instances;
      char *app_name;
   };
   
   struct app_instance_t
   {
      int instance_id;
   };
   
   struct ui_xml_data
   {
      char *file_path;
   };
   
   class web_ui_elements_t
   {
   public:
      nhpc_json_t *elements;
      rbtree_t *elements_tree;
      web_ui_elements_t(const char *_working_dir, const char *_app_xml);
      ~web_ui_elements_t();
      
      char *app_name;
      char *working_dir;
      char *app_xml;
      char *app_xml_public;
      int instance_id;
      
      nhpc_status_t add_element(const char *element, const char *property);
      nhpc_status_t add_public_data();
   };
   
   nhpc_status_t web_ui_init();
   nhpc_status_t web_ui_handler(http_data_t *http_data);
   nhpc_status_t web_ui_init_request(http_data_t *http_data);
   nhpc_status_t web_ui_register(const char *app_name, fnc_ptr_nhpc_two_t func_trigger);
   nhpc_status_t web_ui_generate(web_ui_elements_t *web_ui_elements, char *file_path);
};

#endif