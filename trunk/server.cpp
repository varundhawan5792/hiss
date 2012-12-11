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
#include <iomanip>
#include <fstream>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <include/neweraHPC.h>

using namespace std;
using namespace neweraHPC;

nhpc_status_t func_trigger(nhpc_socket_t *sock, web_ui_elements_t *web_ui_elements)
{
   cout<<"test app trigger invocated"<<endl;
   
   /*
   web_ui_elements->elements->add_element(JSON_STRING, "appname", "Desktop");

   web_ui_elements->elements->add_element(JSON_OBJECT, "app_attributes");
      web_ui_elements->elements->add_element(JSON_STRING, "type", "window");
   web_ui_elements->elements->close_element();
    */
   
   web_ui_elements->elements->add_element(JSON_OBJECT, "NuiWindow");
      web_ui_elements->elements->add_element(JSON_STRING, "height", "100%");
      web_ui_elements->elements->add_element(JSON_STRING, "width", "100%");
   web_ui_elements->elements->close_element();
   
   web_ui_elements->elements->add_element(JSON_OBJECT, "icons");
      web_ui_elements->elements->add_element(JSON_ARRAY, "data");
   
   for(int i = 1; i <= app_handlers->ret_count(); i++)
   {
      app_details_t *app_details = (app_details_t *)(*app_handlers)[i];
      
      cout << i << " " << app_details->app_name << endl;
      int instance_id = app_details->instances->ret_count();
      
      web_ui_elements->elements->add_element(JSON_OBJECT);
      web_ui_elements->elements->add_element(JSON_STRING, "id", (app_details->app_name));
      web_ui_elements->elements->add_element(JSON_STRING, "dblclk", nhpc_strconcat("launch_application('", app_details->app_name, "')"));
      web_ui_elements->elements->add_element(JSON_STRING, "text", app_details->app_name);
      web_ui_elements->elements->add_element(JSON_STRING, "img", "/ui_temp/img/bg/1.jpg");
      web_ui_elements->elements->close_element();
   }
   
      web_ui_elements->elements->close_element();
   web_ui_elements->elements->close_element();

   web_ui_elements->elements->close_element();
   web_ui_elements->add_public_data();
   
   return NHPC_SUCCESS;
}

nhpc_status_t func_trigger_calculator(nhpc_socket_t *sock, web_ui_elements_t *web_ui_elements)
{
   cout<<"test app trigger invocated"<<endl;
   cout << "app_details for: " << web_ui_elements->app_name;
   app_details_t *app_details = (app_details_t *)(*app_handlers).search(web_ui_elements->app_name);
   cout << " " << app_details << endl;
   int instance_id = app_details->instances->ret_count();
  
   web_ui_elements->elements->add_element(JSON_OBJECT, "NuiWindow");
      web_ui_elements->elements->add_element(JSON_STRING, "title", "Calculator");
      web_ui_elements->elements->add_element(JSON_STRING, "height", "80%");
      web_ui_elements->elements->add_element(JSON_STRING, "width", "80%");
      web_ui_elements->elements->add_element(JSON_TRUE, "draggable");
   
   
      web_ui_elements->elements->add_element(JSON_OBJECT, "NuiTable");
	 web_ui_elements->elements->add_element(JSON_STRING, "id", "button_panel");
	 web_ui_elements->elements->add_element(JSON_TRUE, "visible");
	 web_ui_elements->elements->add_element(JSON_NUMBER, "n_rows", "4");
	 web_ui_elements->elements->add_element(JSON_NUMBER, "n_columns", "6");

	    web_ui_elements->elements->add_element(JSON_ARRAY, "NuiElements");
	       web_ui_elements->elements->add_element(JSON_OBJECT);
		  web_ui_elements->elements->add_element(JSON_OBJECT, "NuiButton");
		     web_ui_elements->elements->add_element(JSON_STRING, "id", "calc_4_button");
		  web_ui_elements->elements->close_element();
	       web_ui_elements->elements->close_element();
	    web_ui_elements->elements->close_element();
      web_ui_elements->elements->close_element();
   
   web_ui_elements->elements->close_element();
   
   
   web_ui_elements->elements->close_element();
   
   web_ui_elements->add_public_data();
   
   return NHPC_SUCCESS;
}

nhpc_status_t func_trigger_dock(nhpc_socket_t *sock, web_ui_elements_t *web_ui_elements)
{
   cout << "Dock application trigger initiated" << endl;
   
   web_ui_elements->elements->add_element(JSON_STRING, "appname", "Dock");
   web_ui_elements->elements->close_element();
}

int main(int argc, char **argv)
{
   neweraHPC_init(argc, argv);

   nhpc_status_t nrv;
   
   http_init();
   web_ui_init();
   web_ui_register("Desktop", (fnc_ptr_nhpc_two_t)func_trigger);
   
   web_ui_register("Calculator", (fnc_ptr_nhpc_two_t)func_trigger_calculator);   
   
   web_ui_register("Dock", (fnc_ptr_nhpc_two_t)func_trigger_dock);

   nhpc_grid_server_t grid_server;
   nrv = grid_server.grid_server_init();
   
   if(nrv != NHPC_SUCCESS)
      LOG_ERROR("Grid initialization failed");
   
   return 0;
}
