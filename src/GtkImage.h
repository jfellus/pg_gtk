/*
 * GtkImage.h
 *
 *  Created on: 10 avr. 2015
 *      Author: jfellus
 */

#ifndef PG_GTKIMAGE_H_
#define PG_GTKIMAGE_H_

#include <pg.h>
#include <image.h>

#include "gtk/gtkimageoverlay.h"



class GtkImageViewer {
public:

	std::string title;

	bool click;

private:
	GtkImageOverlayComponent* viewer;
	ImageRGB tmp;

public:

	GtkImageViewer() {viewer = 0; title="image"; click = false;}

	void init() {
		viewer = new GtkImageOverlayComponent(title);
	}

	void process(const ImageRGB& in) {
		viewer->set_image(in.data, in.w, in.h);
		click = viewer->bClick;
		viewer->swap();
	}

	void process(const Image& in) {
		if(tmp && (tmp.w!=in.w || tmp.h!=in.h)) tmp.free();
		if(!tmp) tmp.init(in.w, in.h);
		for(uint i=in.n; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = (unsigned char) (in[i]*255);
		process(tmp);
	}

	void process(const Matrix& in) {
		if(tmp && (tmp.w!=in.w || tmp.h!=in.h)) tmp.free();
		if(!tmp) tmp.init(in.w, in.h);
		for(uint i=in.n; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = (unsigned char) (in[i]*255);
		process(tmp);
	}

};






#endif /* PG_GTKIMAGE_H_ */
