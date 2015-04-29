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
	OverlayText(int x, int y, const char* text) : x(x), y(y) {strncpy(this->text, text, 100);}
	OverlayText(const OverlayText& o) : x(o.x), y(o.y) {strncpy(text, o.text, 100);}
};

class DisplayData {
public:
	unsigned char *img;
	uint w,h;
	std::vector<OverlayPoint> overlay_points;
	std::vector<OverlayImage> overlay_images;
	std::vector<OverlayText> overlay_texts;

	DisplayData() {
		img = NULL;
		w = h = 0;
	}

	void set_image(const unsigned char* img, uint w, uint h) {
		if(w!=this->w || h!=this->h) {
			if(this->img) delete this->img;
			this->w = w; this->h = h; this->img = 0;
			if(img && w && h) this->img = new unsigned char[this->w*this->h*3];
		}
		if(this->img) memcpy(this->img, img, w*h*3);
	}

	void add_overlay_point(const OverlayPoint& p) { overlay_points.push_back(p); }
	void add_overlay_image(const OverlayImage& i) { overlay_images.push_back(i); }
	void add_overlay_text(const OverlayText& t) { overlay_texts.push_back(t); }
};

class ImageViewerComponent {
public:
	void* image_viewer;

	std::string title;
	bool bClick;

private:
	DisplayData* data;
	DisplayData* data2;

	bool bFirstDraw, bRealloc;

	double offsetx, offsety, _zoom;

	bool bNeedRepaint;


public:
	ImageViewerComponent(const std::string& title);

	void set_image(const unsigned char* data, uint w, uint h);
	void set_overlay_points_image(const float* data, uint w, uint h);
	void set_overlay_point(int x, int y);

	void add_overlay_point(int x, int y);
	void clear_overlay_points();

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


	void on_click(double x, double y);
	void _init_instance();
};


#endif /* GTK_H_ */
