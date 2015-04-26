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

class OverlayPoint {
public:
	int x,y;
	OverlayPoint(int x, int y):x(x),y(y) {}
};

class OverlayImage {
public:
	int x,y;
	uint w,h;
	const float* img;
	OverlayImage(int x, int y, uint w, uint h, const float* img) : x(x), y(y), w(w), h(h), img(img) {}
};

class OverlayText {
public:
	int x,y;
	char text[100];
	OverlayText(int x, int y, const char* text) : x(x), y(y) {strcpy(this->text, text);}
	OverlayText(const OverlayText& o) : x(o.x), y(o.y) {strcpy(text, o.text);}
};

class ImageViewerComponent {
public:
	void* image_viewer;

	std::string title;

private:
	const unsigned char *buf_img_data, *displayed_img_data;
	bool bFirstDraw, bRealloc;
	double offsetx, offsety, _zoom;

	std::vector<OverlayPoint*> *displayed_overlay_points, *buf_overlay_points;
	std::vector<OverlayImage*> *displayed_overlay_images, *buf_overlay_images;
	std::vector<OverlayText*> *displayed_overlay_texts, *buf_overlay_texts;

	bool bNeedRepaint;

	uint w1,h1;
	uint w2,h2;

public:
	ImageViewerComponent(const std::string& title);

	void set_image(const unsigned char* data, uint w, uint h);
	void set_overlay_points_image(const float* data, uint w, uint h);
	void set_overlay_point(int x, int y);

	void add_overlay_image(int x, int y, const float* data, uint w, uint h);
	void clear_overlay_images();

	void add_overlay_text(int x, int y, const char* s);
	void clear_overlay_texts();

	void repaint();
	void do_repaint();

	void zoom(float fzoom, double cx, double cy);
	void zoom(double x, double y, double w, double h);
	void move(double dx, double dy);

	void swap();

	int get_width();
	int get_height();

	void draw(void* _cr);



	void _init_instance();
};


#endif /* GTK_H_ */
