/*
 * gtk.cpp
 *
 *  Created on: 10 avr. 2015
 *      Author: jfellus
 */


#include "gtk.h"
#include <pthread.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>
#include <semaphore.h>
#include <iostream>
#include <pg.h>

static pthread_t th_gtk;
static bool inited = false;
pthread_mutex_t GTK_MUT = PTHREAD_MUTEX_INITIALIZER;
static GtkWidget* window = NULL;
static GtkWidget* tabs = NULL;


static void* _gtk_thread(void*) {
	gtk_init (NULL, NULL);
	for(;;) g_main_context_iteration(NULL, TRUE);
	return 0;
}


void gtk_add_tab(void* c, const std::string& title) {
	if(tabs) gtk_notebook_append_page(GTK_NOTEBOOK(tabs), (GtkWidget*)c, gtk_label_new(title.c_str()));
	else gtk_container_add (GTK_CONTAINER (window), (GtkWidget*)c);
}


void gtk_init_gtk() {
	if(!inited) {
		pthread_mutex_lock(&GTK_MUT);
		if(!inited) {
			inited = true;
			pthread_create(&th_gtk, NULL, _gtk_thread, NULL);
		}
		pthread_mutex_unlock(&GTK_MUT);
	}
}



void gtk_create_window(const std::string& title) {
	if(window) return;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), title.c_str());
	g_signal_connect (window, "destroy", G_CALLBACK (exit), NULL);
	gtk_widget_set_size_request (window, 640,480);

	tabs = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(window), tabs);
	gtk_widget_show_all(window);
	gtk_window_maximize(GTK_WINDOW(window));
}







