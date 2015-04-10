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

static pthread_t th_gtk;
static bool inited = false;



static gboolean _on_mouse_move (GtkWidget *widget, GdkEventMotion  *event, gpointer   user_data) {
	static gdouble oldx_mouse, oldy_mouse;
	if((event->state & GDK_BUTTON2_MASK) || (event->state & GDK_BUTTON1_MASK))
		((ImageViewerComponent*)user_data)->move(event->x - oldx_mouse, event->y - oldy_mouse);
	oldx_mouse = event->x; oldy_mouse = event->y;
	return TRUE;
}
static gboolean _on_click (GtkWidget *widget, GdkEventButton  *event, gpointer   user_data) {
	return TRUE;
}
static gboolean _on_unclick (GtkWidget *widget, GdkEventButton  *event, gpointer   user_data) {
	return TRUE;
}
static gboolean _on_scroll (GtkWidget *widget, GdkEventScroll  *event, gpointer   user_data) {
	double dy = event->direction==GDK_SCROLL_UP ? -1 : 1;
	((ImageViewerComponent*)user_data)->zoom(-0.1 * dy, event->x, event->y);
	return TRUE;
}

static gboolean _on_draw (GtkWidget    *widget,  GdkEventExpose *event,  gpointer      user_data) {
	((ImageViewerComponent*)user_data)->draw();
	return FALSE;
}
static gboolean _on_key (GtkWidget *widget, GdkEventKey  *event, gpointer   user_data) {
	return TRUE;
}


static void* _gtk_thread(void*) {
	gdk_threads_init ();
	gdk_threads_enter ();
	gtk_init (NULL, NULL);
	gtk_main ();
	gdk_threads_leave();
	return 0;
}

static void init() {
	if(inited) return;
	pthread_create(&th_gtk, NULL, _gtk_thread, NULL);
	usleep(300000);
}


static GtkWidget* window = NULL;
static GtkWidget* tabs = NULL;
static GtkWidget* create_window(const std::string& title) {
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, FALSE);
	gtk_window_set_title (GTK_WINDOW (window), title.c_str());
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_exit), NULL);
	gtk_widget_set_size_request (window, 640,480);

	tabs = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(window), tabs);
	gtk_widget_show_all(window);
	gtk_window_maximize(GTK_WINDOW(window));

	return window;
}


ImageViewerComponent::ImageViewerComponent(const std::string& title) {
	bFirstDraw = true;
	_zoom = 1;
	offsetx = offsety = 0;

	if(!inited) init();

	gdk_threads_enter ();
	pixbuf = 0;

	if(!window) create_window(title);

	image_viewer = gtk_drawing_area_new();
	gtk_widget_set_can_focus((GtkWidget*)image_viewer, TRUE);

	gtk_widget_set_events((GtkWidget*)image_viewer, GDK_ALL_EVENTS_MASK);
	g_signal_connect(G_OBJECT((GtkWidget*)image_viewer), "scroll-event", G_CALLBACK(_on_scroll), this);
	g_signal_connect(G_OBJECT((GtkWidget*)image_viewer), "motion-notify-event", G_CALLBACK(_on_mouse_move), this);
	g_signal_connect(G_OBJECT((GtkWidget*)image_viewer), "button-press-event", G_CALLBACK(_on_click), this);
	g_signal_connect(G_OBJECT((GtkWidget*)image_viewer), "button-release-event", G_CALLBACK(_on_unclick), this);
	g_signal_connect(G_OBJECT((GtkWidget*)image_viewer), "key-press-event", G_CALLBACK(_on_key), this);

	if(tabs) gtk_notebook_append_page(GTK_NOTEBOOK(tabs), (GtkWidget*)image_viewer, gtk_label_new(title.c_str()));
	else gtk_container_add (GTK_CONTAINER (window), (GtkWidget*)image_viewer);

	gtk_widget_show ((GtkWidget*)image_viewer);
	gtk_widget_set_size_request(GTK_WIDGET(image_viewer), 800,600);
	g_signal_connect(G_OBJECT((GtkWidget*)image_viewer), "expose-event", G_CALLBACK(_on_draw), this);

	gdk_threads_leave();
}

void ImageViewerComponent::set_image(const unsigned char* data, uint w, uint h) {
	if(!pixbuf) pixbuf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w,h);

	if(image_viewer && pixbuf) {
		gdk_threads_enter();
		uint* d = (uint*)cairo_image_surface_get_data((cairo_surface_t*)pixbuf);
		for(uint i=w*h-1;i--;) d[i] = ((uint)data[i*3] << 16 | (uint)data[i*3+1] << 8 | (uint)data[i*3+2]);
		gtk_widget_queue_draw( GTK_WIDGET(image_viewer));
		gdk_threads_leave();
	}
}

void ImageViewerComponent::zoom(float fzoom, double cx, double cy) {
	cx =  (cx - offsetx)/ _zoom ;
	cy =  (cy - offsety)/ _zoom ;
	double oldzoom = _zoom;
	_zoom *= 1 + fzoom;
	offsetx -= cx*(_zoom - oldzoom);
	offsety -= cy*(_zoom - oldzoom);
	repaint();
}

void ImageViewerComponent::zoom(double x, double y, double w, double h) {
	double www = get_width()-20;
	double hhh = get_height()-20;
	if(!www || !hhh) return;
	_zoom = MIN(hhh/h, www/w);

	offsetx= -x*_zoom + 10 + MAX(0,(www - w*_zoom)/2);
	offsety= -y*_zoom + 10 + MAX(0,(hhh - h*_zoom)/2);

	repaint();
}

void ImageViewerComponent::move(double dx, double dy) {offsetx += dx; offsety += dy; repaint();}

void ImageViewerComponent::repaint() {
	gtk_widget_queue_draw((GtkWidget*)image_viewer);
}

int ImageViewerComponent::get_width() {return ((GtkWidget*)image_viewer)->allocation.width;}
int ImageViewerComponent::get_height() {return ((GtkWidget*)image_viewer)->allocation.height;}



void ImageViewerComponent::draw() {
	if(!pixbuf || !image_viewer || !gtk_widget_get_realized(GTK_WIDGET(image_viewer))) return;
	if(bFirstDraw){
		zoom(0,0,cairo_image_surface_get_width((cairo_surface_t*)pixbuf),cairo_image_surface_get_height((cairo_surface_t*)pixbuf));
		bFirstDraw = false;
	}
	cairo_t* cr = gdk_cairo_create (((GtkWidget*)image_viewer)->window);
	cairo_set_source_rgb(cr, 0,0,0);
	cairo_paint(cr);

	cairo_translate(cr, offsetx, offsety);
	cairo_scale(cr, _zoom, _zoom);

	cairo_set_source_surface(cr, (cairo_surface_t*)pixbuf, 0, 0);
	cairo_rectangle(cr, 0,0,cairo_image_surface_get_width((cairo_surface_t*)pixbuf),cairo_image_surface_get_height((cairo_surface_t*)pixbuf));
	cairo_fill(cr);
	cairo_destroy(cr);
}
