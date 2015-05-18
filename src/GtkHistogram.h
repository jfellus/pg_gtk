/*
 * GtkHistogram.h
 *
 *  Created on: 15 avr. 2015
 *      Author: jfellus
 */

#ifndef GTKHISTOGRAM_H_
#define GTKHISTOGRAM_H_

#include <pg.h>
#include <matrix.h>
#include "gtk/gtkimageoverlay.h"

class GtkHistogram {
public:
	std::string title;
	float max;
	bool click;

private:
	GtkImageOverlayComponent* viewer;
	ImageRGB tmp;

public:
	GtkHistogram() {
		title = "Histogram";
		viewer = 0;
		max = 200;
		click = false;
	}

	void init() {
		viewer = new GtkImageOverlayComponent(title);
	}

	void process(Matrix& m) {
		click = viewer->bClick;
		if(!tmp) { tmp.init(m.w,max);}
		for(uint x=0; x<tmp.w; x++) {
			for(uint y=tmp.h;y--;) tmp(x,y)[0] = tmp(x,y)[1] = tmp(x,y)[2] = y>tmp.h-m[x] ? 255 : 0;
		}
		viewer->set_image(tmp, tmp.w, tmp.h);
		viewer->swap();
	}
};



#endif /* GTKHISTOGRAM_H_ */
