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
#include <fstream>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/network.h>
#include <include/general.h>
#include <include/web_ui.h>
#include <include/constants.h>

using namespace std;

namespace neweraHPC
{
   rbtree_t *app_handlers;
   const char *ui_temp_dir;
   
   nhpc_status_t web_ui_init()
   {
      app_handlers = new rbtree_t(NHPC_RBTREE_STR);
      
      http_handler_register("app", (fnc_ptr_nhpc_t)web_ui_handler);
      
      nhpc_status_t nrv = nhpc_create_tmp_file_or_dir(&ui_temp_dir, HTTP_ROOT, NHPC_DIRECTORY, "ui_temp");
      
      return nrv;
   }
   
   nhpc_status_t web_ui_register(const char *app_name, fnc_ptr_nhpc_two_t func_trigger)
   {
      app_details_t *app_details = new app_details_t;
      app_details->func_trigger_local = func_trigger;
      app_details->instances = new rbtree_t(NHPC_RBTREE_NUM);
      nhpc_strcpy(&(app_details->app_name), app_name);
      cout << "Registered: " << app_details->app_name << endl;
      
      nhpc_status_t rv = app_handlers->insert(app_details, app_name);
      
      return rv;
   }
   
   nhpc_status_t web_ui_handler(http_data_t *http_data)
   {
      web_ui_init_request(http_data);
   }
   
   nhpc_status_t web_ui_init_request(http_data_t *http_data)
   {
      nhpc_socket_t *sock = http_data->sock;
      rbtree_t *ui_details;
      char *file_path;
      
      string_t *app_details_str = nhpc_substr(http_data->request_page, '/');
      if(app_details_str->count < 2)
      {
	 return NHPC_FAIL;
      }
      
      app_details_t *app_details = (app_details_t *)app_handlers->search(app_details_str->strings[1]);
      //fnc_ptr_nhpc_two_t *func_trigger_local = (fnc_ptr_nhpc_two_t *)app_handlers->search(app_details->strings[1]);
      
      if(!app_details)
      {
	 cout<<"failed\n\n";
	 return NHPC_FAIL;
      }
      
      int instance_count = app_details->instances->ret_count();
      app_instance_t *app_instance = new app_instance_t;
      app_instance->instance_id = instance_count + 1;
      app_details->instances->insert(app_instance);
      cout << "Current:Instance_id:" << instance_count << endl;
      
      char *app_name = app_details_str->strings[1];
      app_details_str->strings[1] = nhpc_strconcat(app_details_str->strings[1], ".json");
      nhpc_create_tmp_file_or_dir((const char **)&file_path, ui_temp_dir, NHPC_FILE, app_details_str->strings[1]);
      
      web_ui_elements_t *web_ui_elements = new web_ui_elements_t(ui_temp_dir, file_path);
      web_ui_elements->instance_id = instance_count;
      web_ui_elements->add_element("app_title", app_details_str->strings[1]);
      nhpc_strcpy(&(web_ui_elements->app_name), app_name);
      
      web_ui_elements->elements->add_element(JSON_STRING, "appname", app_name);
      web_ui_elements->elements->add_element(JSON_OBJECT, "app_attributes");
	 web_ui_elements->elements->add_element(JSON_STRING, "instance_id", nhpc_itostr(instance_count));
	 web_ui_elements->elements->add_element(JSON_STRING, "type", "window");
      web_ui_elements->elements->close_element();      
      
      (*(app_details->func_trigger_local))(sock, web_ui_elements);
      
      web_ui_generate(web_ui_elements, file_path);
      
      delete[] file_path;
      
      delete[] (http_data->request_page);
      http_data->request_page = nhpc_strconcat("/ui_temp", "/standard.html");
      cout<<http_data->request_page<<endl;
      
      return NHPC_SUCCESS;
   }
   
   nhpc_status_t web_ui_generate(web_ui_elements_t *web_ui_elements, char *file_path)
   {
      ofstream xml_file(file_path);
      
      xml_file << web_ui_elements->elements->get_stream() << endl;
      
      xml_file.close();
   }
   
   web_ui_elements_t::web_ui_elements_t(const char *_working_dir, const char *_app_xml)
   {
      working_dir = nhpc_strconcat(_working_dir, "/");
      
      string_t *temp = nhpc_substr(_app_xml, '/');
      elements = new nhpc_json_t;
      elements_tree = new rbtree_t(NHPC_RBTREE_STR);
      
      nhpc_strcpy(&app_xml, temp->strings[temp->count - 1]);
   }
   
   nhpc_status_t web_ui_elements_t::add_element(const char *element, const char *property)
   {
      if(nhpc_strcmp(element, "connect") == NHPC_SUCCESS)
      {
	 char *remote_app_xml_path = nhpc_strconcat(working_dir, property, ".public");
      }
      else 
      {
	 elements_tree->insert((void *)property, element);
      }
   }
   
   nhpc_status_t web_ui_elements_t::add_public_data()
   {
      char *file_name = nhpc_strconcat(app_xml, ".public");
      nhpc_status_t nrv = nhpc_create_tmp_file_or_dir((const char **)&(app_xml_public), working_dir, NHPC_FILE, file_name);
   }
};