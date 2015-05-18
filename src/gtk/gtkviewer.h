/*
 * gtkviewer.h
 *
 *  Created on: 29 avr. 2015
 *      Author: jfellus
 */

#ifndef GTKVIEWER_H_
#define GTKVIEWER_H_


#include "gtk.h"

class GtkViewer;


class Drawable {
public:
	Drawable() {}
	virtual ~Drawable() {}
	virtual void draw(void* _cr, GtkViewer* v) {}
	virtual void on_click(double x, double y) {}
};


class GtkViewer {
public:
	std::string title;
	void* c;
	Drawable* drawable;

	bool bFirstDraw, bRealloc;

	double offsetx, offsety, _zoom;

	bool bNeedRepaint;

	uint w,h;

private:
	void* _cr;

public:
	GtkViewer(const std::string& title, Drawable* drawable);

	void repaint();
	void do_repaint();

	void zoom(float fzoom, double cx, double cy);
	void zoom(double x, double y, double w, double h);
	void move(double dx, double dy);


	int get_width();
	int get_height();

	void draw(void* _cr);


	void on_click(double x, double y);
	void _init_instance();

	void LOCK();
	void UNLOCK();

	operator bool();


	/////////////
	// DRAWING //
	/////////////

	void draw_text(double x, double y, const char* text);
	void draw_text_circle(double x, double y, const char* text);
	void draw_image_rect(const float* img, double x, double y, uint w, uint h);
	void draw_image_rect(const unsigned char* img, uint imgw, uint imgh, double x, double y, double w, double h);
	void draw_point(double x, double y);
	void set_color(double r, double g, double b);
	void draw_image(const unsigned char* img, uint w, uint h);
	void scale(double d);
	void fill_rect(double x, double y, double w, double h);
};


#endif /* GTKVIEWER_H_ */
