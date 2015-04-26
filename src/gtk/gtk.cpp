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
	double dy = event->delta_y;
	((ImageViewerComponent*)user_data)->zoom(-0.1 * dy, event->x, event->y);
	return TRUE;
}

static gboolean _on_draw (GtkWidget *widget, cairo_t *cr, void* p) {
	((ImageViewerComponent*)p)->draw(cr);
	return FALSE;
}
static gboolean _on_key (GtkWidget *widget, GdkEventKey  *event, gpointer   user_data) {
	return TRUE;
}


static void* _gtk_thread(void*) {
	gtk_init (NULL, NULL);
	for(;;) {
		g_main_context_iteration(NULL, FALSE);
	}
	return 0;
}

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static void init() {
	pthread_mutex_lock(&mut);
	if(!inited) {
		inited = true;
		pthread_create(&th_gtk, NULL, _gtk_thread, NULL);
	}
	pthread_mutex_unlock(&mut);
}


static GtkWidget* window = NULL;
static GtkWidget* tabs = NULL;
static GtkWidget* create_window(const std::string& title) {
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), title.c_str());
	g_signal_connect (window, "destroy", G_CALLBACK (exit), NULL);
	gtk_widget_set_size_request (window, 640,480);

	tabs = gtk_notebook_new();
	gtk_container_add(GTK_CONTAINER(window), tabs);
	gtk_widget_show_all(window);
	gtk_window_maximize(GTK_WINDOW(window));

	return window;
}


gboolean __init_instance(void* p) {((ImageViewerComponent*)p)->_init_instance(); return FALSE;}

ImageViewerComponent::ImageViewerComponent(const std::string& title) {
	bRealloc = true;
	this->title = title;
	bNeedRepaint = true;
	displayed_overlay_points = new std::vector<OverlayPoint*>();
	buf_overlay_points = new std::vector<OverlayPoint*>();
	displayed_overlay_images = new std::vector<OverlayImage*>();
	buf_overlay_images = new std::vector<OverlayImage*>();
	displayed_overlay_texts = new std::vector<OverlayText*>();
	buf_overlay_texts = new std::vector<OverlayText*>();

	bFirstDraw = true;
	_zoom = 1;
	offsetx = offsety = 0;
	w1 = h1 = w2 = h2 = 0;
	buf_img_data = displayed_img_data = 0;

	if(!inited) init();

	g_idle_add(__init_instance, this);
}

void ImageViewerComponent::_init_instance() {
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
	g_signal_connect(G_OBJECT((GtkWidget*)image_viewer), "draw", G_CALLBACK(_on_draw), this);
}

void ImageViewerComponent::set_image(const unsigned char* data, uint w, uint h) {
	pthread_mutex_lock(&mut);
	this->w2 = w;
	this->h2 = h;
	this->buf_img_data = data;
	pthread_mutex_unlock(&mut);
//	repaint();
}

void ImageViewerComponent::set_overlay_points_image(const float* data, uint w, uint h) {
	std::vector<OverlayPoint*>* overlay_points = buf_overlay_points;
	for(uint i=0; i<overlay_points->size(); i++) delete (*buf_overlay_points)[i];
	overlay_points->clear();
	for(uint y=0; y<h; y++) {
		for(uint x=0; x<w; x++) {
			if(data[y*w+x]>0.0001) {
				overlay_points->push_back(new OverlayPoint(x,y));
			}
		}
	}
}

void ImageViewerComponent::add_overlay_image(int x, int y, const float* data, uint w, uint h) {
	buf_overlay_images->push_back(new OverlayImage(x,y,w,h,data));
}

void ImageViewerComponent::clear_overlay_images() {
	for(uint i=0; i<buf_overlay_images->size(); i++) delete (*buf_overlay_images)[i];
	buf_overlay_images->clear();
}

void ImageViewerComponent::add_overlay_text(int x, int y, const char* s) {
	buf_overlay_texts->push_back(new OverlayText(x,y,s));
}

void ImageViewerComponent::clear_overlay_texts() {
//	for(uint i=0; i<buf_overlay_texts->size(); i++) delete (*buf_overlay_texts)[i];
	buf_overlay_texts->clear();
}


void ImageViewerComponent::set_overlay_point(int x, int y) {
//	for(uint i=0; i<buf_overlay_points->size(); i++) delete (*buf_overlay_points)[i];
	buf_overlay_points->clear();
	buf_overlay_points->push_back(new OverlayPoint(x,y));
}

void ImageViewerComponent::zoom(float fzoom, double cx, double cy) {
	cx =  (cx - offsetx)/ _zoom ;
	cy =  (cy - offsety)/ _zoom ;
	double oldzoom = _zoom;
	_zoom *= 1 + fzoom;
	offsetx -= cx*(_zoom - oldzoom);
	offsety -= cy*(_zoom - oldzoom);
	do_repaint();
}

void ImageViewerComponent::zoom(double x, double y, double w, double h) {
	double www = get_width()-20;
	double hhh = get_height()-20;
	if(!www || !hhh) return;
	_zoom = MIN(hhh/h, www/w);

	offsetx= -x*_zoom + 10 + MAX(0,(www - w*_zoom)/2);
	offsety= -y*_zoom + 10 + MAX(0,(hhh - h*_zoom)/2);

	do_repaint();
}

void ImageViewerComponent::move(double dx, double dy) {offsetx += dx; offsety += dy; do_repaint();}

void ImageViewerComponent::repaint() {
	pthread_mutex_lock(&mut);
	do_repaint();
	pthread_mutex_unlock(&mut);
}

void ImageViewerComponent::do_repaint() {
	if(bNeedRepaint && image_viewer) {
		gtk_widget_queue_draw((GtkWidget*)image_viewer); bNeedRepaint = false;
	}
}

int ImageViewerComponent::get_width() {return gtk_widget_get_allocated_width((GtkWidget*)image_viewer);}
int ImageViewerComponent::get_height() {return gtk_widget_get_allocated_height((GtkWidget*)image_viewer);}

gboolean _redraw(void* p) {
	gtk_widget_queue_draw((GtkWidget*)p); return FALSE;
}
void ImageViewerComponent::swap() {
	pthread_mutex_lock(&mut);
	std::vector<OverlayPoint*>* tmp_points = buf_overlay_points;
	std::vector<OverlayImage*>* tmp_images = buf_overlay_images;
	std::vector<OverlayText*>* tmp_texts = buf_overlay_texts;
	const unsigned char* tmp_img_data = buf_img_data;
	buf_overlay_images = displayed_overlay_images;
	buf_overlay_points = displayed_overlay_points;
	buf_overlay_texts = displayed_overlay_texts;
	buf_img_data = displayed_img_data;
	displayed_overlay_images = tmp_images;
	displayed_overlay_points = tmp_points;
	displayed_overlay_texts = tmp_texts;
	displayed_img_data = tmp_img_data;
	uint tmph = h1; h1 = h2; h2 = tmph;
	uint tmpw = w1; w1 = w2; w2 = tmpw;
	g_timeout_add(10, _redraw,image_viewer);
	pthread_mutex_unlock(&mut);
}

void ImageViewerComponent::draw(void* _cr) {
	cairo_t* cr = (cairo_t*)_cr;
	if(!image_viewer || !gtk_widget_get_realized(GTK_WIDGET(image_viewer)) || !w1 || !h1) return;
	pthread_mutex_lock(&mut);
	do_repaint();
	if(!displayed_img_data) return;
	if(bFirstDraw){
		zoom(0,0,w1,h1);
		bFirstDraw = false;
	}
	cairo_set_source_rgb(cr, .5,.5,.5);
	cairo_paint(cr);

	cairo_translate(cr, offsetx, offsety);
	cairo_scale(cr, _zoom, _zoom);

	{
		cairo_surface_t* pixbuf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w1, h1);
		uint* data = (uint*)cairo_image_surface_get_data((cairo_surface_t*)pixbuf);
		for(uint i=0; i<w1*h1; i++) data[i] = ((uint)displayed_img_data[i*3])<<16 |((uint)displayed_img_data[i*3+1])<<8 | ((uint)displayed_img_data[i*3+2]);
		cairo_set_source_surface(cr, (cairo_surface_t*)pixbuf, 0, 0);
		cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
		cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
		cairo_rectangle(cr, 0,0,w1,h1);
		cairo_fill(cr);
		cairo_surface_destroy(pixbuf);
	}

	std::vector<OverlayPoint*>* overlay_points = displayed_overlay_points;
	std::vector<OverlayImage*>* overlay_images = displayed_overlay_images;
	std::vector<OverlayText*>* overlay_texts = displayed_overlay_texts;

	if(!overlay_points->empty()) {
		cairo_set_source_rgb(cr, 1,0,0);
		for(uint i=0; i<overlay_points->size(); i++) {
			cairo_rectangle(cr, (*overlay_points)[i]->x-2, (*overlay_points)[i]->y-2, 4,4);
			cairo_fill(cr);
		}
	}

	if(!overlay_images->empty()) {
		for(uint i=0; i<overlay_images->size(); i++) {
			OverlayImage* o = (*overlay_images)[i];
			cairo_surface_t* pixbuf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, o->w, o->h);
			uint* d = (uint*)cairo_image_surface_get_data((cairo_surface_t*)pixbuf);
			const float* data = o->img;
			for(uint j=o->w*o->h;j--;) d[j] = ((uint)(data[j]*255) << 16 | (uint)(data[j]*255) << 8 | (uint)(data[j]*255));
			cairo_set_source_surface(cr, pixbuf, o->x - o->w/2, o->y - o->h/2);
			cairo_rectangle(cr, o->x - o->w/2, o->y - o->h/2, o->w, o->h);
			cairo_fill(cr);
			cairo_surface_destroy(pixbuf);
		}
	}


	if(!overlay_texts->empty()) {
		for(uint i=0; i<overlay_texts->size(); i++) {
			OverlayText* o = (*overlay_texts)[i];
			cairo_translate(cr, o->x , o->y);

			cairo_set_source_rgba(cr, 1,0,0,0.5);
			cairo_arc(cr, 0,0,7, 0, 6.5);
			cairo_fill(cr);

			cairo_translate(cr, -3, 3);
			cairo_set_source_rgb(cr, 1,1,1);
			cairo_show_text(cr, o->text);
			cairo_fill(cr);

			cairo_translate(cr, -o->x + 3, -o->y -3);
		}
	}
	bNeedRepaint = true;
	pthread_mutex_unlock(&mut);
}
