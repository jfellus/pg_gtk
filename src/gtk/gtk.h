/*
 * gtk.h
 *
 *  Created on: 10 avr. 2015
 *      Author: jfellus
 */

#ifndef PG_GTK_H_
#define PG_GTK_H_

#include <string>
#include <stdlib.h>
#include <vector>
#include "string.h"
#include <unistd.h>


void gtk_init_gtk();
void gtk_create_window(const std::string& title);
void gtk_add_tab(void* component, const std::string& title);






#endif /* GTK_H_ */
