/*
 * GtkPlot.h
 *
 *  Created on: 20 avr. 2015
 *      Author: jfellus
 */

#ifndef GTKPLOT_H_
#define GTKPLOT_H_

#include <pg.h>
#include <matrix.h>
#include "gtk/gtk.h"
#include <string.h>

class GtkPlot {
public:
	uint w,h;
	float min,max;
	std::string title;

private:
	uint cur_x;
	ImageRGB out;
	ImageViewerComponent* viewer;

private:
	public:
	GtkPlot() {title = "plot"; w = 640; h = 480; viewer = NULL; min = 0; max = 1;cur_x = 0;}

	void init() {viewer = new ImageViewerComponent(title); out.init(w,h);}

	void process(Matrix& in) {
		for(uint y=0;y<h;y++) memset(out(cur_x, y), 255, 3);
		for(uint i=0; i<in.n; i++) {
			float v = in[i];
			plot(v);
		}
		next();
	}

private:

	void plot(float v) {
		unsigned char color[] = {255,0,0};
		int y = (1-(v-min)/(max-min))*h;
		if(y>=h) y = h; if(y<0) y = 0;
		memcpy(out(cur_x,y), color, 3);
	}

	void next() {cur_x++; viewer->set_image(out, w, h); viewer->swap(); if(cur_x>=w) cur_x = 0;}
};




#endif /* GTKPLOT_H_ */
