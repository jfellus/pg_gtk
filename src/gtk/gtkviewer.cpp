/*
 * gtkimageviewer.cpp
 *
 *  Created on: 29 avr. 2015
 *      Author: jfellus
 */


#include "gtkviewer.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>


extern pthread_mutex_t GTK_MUT;


gboolean __init_instance(void* p) {((GtkViewer*)p)->_init_instance(); return FALSE;}




static gboolean _on_mouse_move (GtkWidget *widget, GdkEventMotion  *event, gpointer   user_data) {
	static gdouble oldx_mouse, oldy_mouse;
	if((event->state & GDK_BUTTON2_MASK) || (event->state & GDK_BUTTON1_MASK))
		((GtkViewer*)user_data)->move(event->x - oldx_mouse, event->y - oldy_mouse);
	oldx_mouse = event->x; oldy_mouse = event->y;
	return TRUE;
}
static gboolean _on_click (GtkWidget *widget, GdkEventButton  *event, gpointer   user_data) {
	if(((GtkViewer*)user_data)->drawable && event->button == 1) ((GtkViewer*)user_data)->drawable->on_click(event->x, event->y);
	return TRUE;
}
static gboolean _on_unclick (GtkWidget *widget, GdkEventButton  *event, gpointer   user_data) {
	return TRUE;
}
static gboolean _on_scroll (GtkWidget *widget, GdkEventScroll  *event, gpointer   user_data) {
	double dy = event->delta_y;
	((GtkViewer*)user_data)->zoom(-0.1 * dy, event->x, event->y);
	return TRUE;
}

static gboolean _on_draw (GtkWidget *widget, cairo_t *cr, void* p) {
	((GtkViewer*)p)->draw(cr);
	return FALSE;
}
static gboolean _on_key (GtkWidget *widget, GdkEventKey  *event, gpointer   user_data) {
	return TRUE;
}






GtkViewer::GtkViewer(const std::string& title, Drawable* drawable) : title(title), drawable(drawable) {
	c = NULL;
	bRealloc = true;
	bNeedRepaint = true;
	bFirstDraw = true;
	_zoom = 1;
	offsetx = offsety = 0;

	gtk_init_gtk();

	g_idle_add(__init_instance, this);
}




void GtkViewer::_init_instance() {
	gtk_create_window(title);

	c = gtk_drawing_area_new();
	gtk_widget_set_can_focus((GtkWidget*)c, TRUE);

	gtk_widget_set_events((GtkWidget*)c, GDK_ALL_EVENTS_MASK);
	g_signal_connect(G_OBJECT((GtkWidget*)c), "scroll-event", G_CALLBACK(_on_scroll), this);
	g_signal_connect(G_OBJECT((GtkWidget*)c), "motion-notify-event", G_CALLBACK(_on_mouse_move), this);
	g_signal_connect(G_OBJECT((GtkWidget*)c), "button-press-event", G_CALLBACK(_on_click), this);
	g_signal_connect(G_OBJECT((GtkWidget*)c), "button-release-event", G_CALLBACK(_on_unclick), this);
	g_signal_connect(G_OBJECT((GtkWidget*)c), "key-press-event", G_CALLBACK(_on_key), this);

	gtk_add_tab(c, title);

	gtk_widget_show ((GtkWidget*)c);
	gtk_widget_set_size_request(GTK_WIDGET(c), 800,600);
	g_signal_connect(G_OBJECT((GtkWidget*)c), "draw", G_CALLBACK(_on_draw), this);
}


void GtkViewer::zoom(float fzoom, double cx, double cy) {
	cx =  (cx - offsetx)/ _zoom ;
	cy =  (cy - offsety)/ _zoom ;
	double oldzoom = _zoom;
	_zoom *= 1 + fzoom;
	offsetx -= cx*(_zoom - oldzoom);
	offsety -= cy*(_zoom - oldzoom);
	do_repaint();
}

void GtkViewer::zoom(double x, double y, double w, double h) {
	double www = get_width()-20;
	double hhh = get_height()-20;
	if(!www || !hhh) return;
	_zoom = MIN(hhh/h, www/w);

	offsetx= -x*_zoom + 10 + MAX(0,(www - w*_zoom)/2);
	offsety= -y*_zoom + 10 + MAX(0,(hhh - h*_zoom)/2);

	do_repaint();
}

void GtkViewer::move(double dx, double dy) {offsetx += dx; offsety += dy; do_repaint();}

gboolean _redraw(void* p) {
	gtk_widget_queue_draw((GtkWidget*)p); return FALSE;
}
void GtkViewer::repaint() {
	g_idle_add(_redraw,c);
}

void GtkViewer::do_repaint() {
	if(bNeedRepaint && c) {
		gtk_widget_queue_draw((GtkWidget*)c); bNeedRepaint = false;
	}
}

int GtkViewer::get_width() {return gtk_widget_get_allocated_width((GtkWidget*)c);}
int GtkViewer::get_height() {return gtk_widget_get_allocated_height((GtkWidget*)c);}

void GtkViewer::LOCK() {pthread_mutex_lock(&GTK_MUT);}
void GtkViewer::UNLOCK() {pthread_mutex_unlock(&GTK_MUT);}


GtkViewer::operator bool() { return gtk_widget_get_realized(GTK_WIDGET(c)); }


void GtkViewer::draw(void* _cr) {
	this->_cr = _cr;
	cairo_t* cr = (cairo_t*)_cr;
	if(!c) return;

	LOCK();
	do_repaint();
	if(bFirstDraw && w && h){
		zoom(0,0,w,h);
		bFirstDraw = false;
	}
	cairo_set_source_rgb(cr, .5,.5,.5);
	cairo_paint(cr);

	cairo_translate(cr, offsetx, offsety);
	cairo_scale(cr, _zoom, _zoom);

	drawable->draw(_cr, this);

	bNeedRepaint = true;
	UNLOCK();
}



/////////////
// DRAWING //
/////////////

void GtkViewer::draw_text_circle(double x, double y, const char* text) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_translate(cr, x , y);

	cairo_set_source_rgba(cr, 1,0,0,0.5);
	cairo_arc(cr, 0,0,7, 0, 6.5);
	cairo_fill(cr);

	cairo_translate(cr, -3, 3);
	cairo_set_source_rgb(cr, 1,1,1);
	cairo_show_text(cr, text);
	cairo_fill(cr);

	cairo_translate(cr, -x + 3, -y -3);
}

void GtkViewer::draw_text(double x, double y, const char* text) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_translate(cr, x , y);
	cairo_show_text(cr, text);
	cairo_translate(cr, -x, -y);
}

void GtkViewer::draw_image_rect(const float* img, double x, double y, uint w, uint h) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_surface_t* pixbuf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
	uint* d = (uint*)cairo_image_surface_get_data((cairo_surface_t*)pixbuf);
	for(uint j=w*h;j--;) d[j] = ((uint)(img[j]*255) << 16 | (uint)(img[j]*255) << 8 | (uint)(img[j]*255));
	cairo_set_source_surface(cr, pixbuf, x , y );
	cairo_rectangle(cr, x, y, w, h);
	cairo_fill(cr);
	cairo_surface_destroy(pixbuf);
}


void GtkViewer::draw_image_rect(const unsigned char* img, uint imgw, uint imgh, double x, double y, double w, double h) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_surface_t* pixbuf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, imgw, imgh);
	uint* d = (uint*)cairo_image_surface_get_data((cairo_surface_t*)pixbuf);
	for(uint i=0; i<imgw*imgh; i++) d[i] = ((uint)img[i*3])<<16 |((uint)img[i*3+1])<<8 | ((uint)img[i*3+2]);
	cairo_translate(cr, x, y);
	cairo_scale(cr, w/imgw, h/imgh);
	cairo_set_source_surface(cr, pixbuf, 0, 0);
	cairo_rectangle(cr, 0, 0, imgw, imgh);
	cairo_fill(cr);
	cairo_scale(cr, imgw/w, imgh/h);
	cairo_translate(cr, -x, -y);
	cairo_surface_destroy(pixbuf);
}

void GtkViewer::draw_point(double x, double y) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_rectangle(cr,x-2, y-2, 4,4);
	cairo_fill(cr);
}

void GtkViewer::set_color(double r, double g, double b) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_set_source_rgb(cr, r,g,b);
}

void GtkViewer::draw_image(const unsigned char* img, uint w, uint h) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_surface_t* pixbuf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
	uint* data = (uint*)cairo_image_surface_get_data((cairo_surface_t*)pixbuf);
	for(uint i=0; i<w*h; i++) data[i] = ((uint)img[i*3])<<16 |((uint)img[i*3+1])<<8 | ((uint)img[i*3+2]);
	cairo_set_source_surface(cr, (cairo_surface_t*)pixbuf, 0, 0);
	cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
	cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
	cairo_rectangle(cr, 0,0,w,h);
	cairo_fill(cr);
	cairo_surface_destroy(pixbuf);
}

void GtkViewer::scale(double d) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_scale(cr, d,d);
}

void GtkViewer::fill_rect(double x, double y, double w, double h) {
	cairo_t* cr = (cairo_t*)_cr;
	cairo_rectangle(cr, x,y,w,h);
	cairo_fill(cr);
}
