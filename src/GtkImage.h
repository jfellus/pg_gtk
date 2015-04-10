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

#include "gtk/gtk.h"



class GtkImageViewer {
public:

	std::string title;

private:
	ImageViewerComponent* viewer;
	ImageRGB tmp;

public:

	GtkImageViewer() {viewer = 0; title="image";}

	void init() {
		viewer = new ImageViewerComponent(title);
	}

	void process(const ImageRGB& in) {
		viewer->set_image(in.data, in.w, in.h);
	}

	void process(const Image& in) {
		tmp.init(in.w, in.h);
		for(uint i=in.n-1; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = in[i]*255;
		process(tmp);
	}


};






#endif /* PG_GTKIMAGE_H_ */
