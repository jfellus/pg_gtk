/*
 * GtkImageOverlay.h
 *
 *  Created on: 12 avr. 2015
 *      Author: jfellus
 */

#ifndef GTKIMAGEOVERLAY_H_
#define GTKIMAGEOVERLAY_H_


#include <pg.h>
#include <image.h>

#include "gtk/gtk.h"



class GtkImageOverlay {
public:

	std::string title;

private:
	ImageViewerComponent* viewer;
	ImageRGB tmp;

public:

	GtkImageOverlay() {viewer = 0; title="image";}

	void init() {
		viewer = new ImageViewerComponent(title);
	}


	void process(const ImageRGB& in) {
		viewer->set_image(in.data, in.w, in.h);
		viewer->swap();
	}

	void process(const Image& in) {
		tmp.init(in.w, in.h);
		for(uint i=in.n; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = in[i]*255;
		process(tmp);
	}

	void process(const Matrix& in) {
		tmp.init(in.w, in.h);
		for(uint i=in.w*in.h; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = in[i]*255;
		process(tmp);
	}

	void process(const ImageRGB& in, const Image& overlay_points) {
		viewer->set_image(in.data, in.w, in.h);
		viewer->set_overlay_points_image(overlay_points, overlay_points.w, overlay_points.h);
		viewer->swap();
	}

	void process(const Image& in, const Image& overlay_points) {
		tmp.init(in.w, in.h);
		for(uint i=in.n; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = in[i]*255;
		process(tmp, overlay_points);
	}

	void process(const ImageRGB& in, const Matrix& points) {
		viewer->set_image(in.data, in.w, in.h);
		viewer->clear_overlay_points();
		for(uint i=0; i<points.h; i++) {
			viewer->add_overlay_point((int)points(i,0), (int)points(i,1));
		}
		viewer->swap();
	}

	void process(const Image& in, const Matrix& points) {
		tmp.init(in.w, in.h);
		for(uint i=in.n; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = in[i]*255;
		process(tmp, points);
	}

	void process(const ImageRGB& in, const Matrix& overlays, const Matrix& overlays_points) {
		viewer->set_image(in.data, in.w, in.h);
		int side = sqrt(overlays.w);
		viewer->clear_overlay_images();
		for(uint i=0; i<overlays.h; i++) {
			const float* f = &overlays_points[i*overlays_points.w];
			viewer->add_overlay_image((int)f[0], (int)f[1], &overlays[i*overlays.w], side, side);
		}
		viewer->swap();
	}

	void process(const Image& in, const Matrix& overlays, const Matrix& overlays_points) {
		tmp.init(in.w, in.h);
		for(uint i=in.n; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = in[i]*255;
		process(tmp, overlays, overlays_points);
	}

};






#endif /* GTKIMAGEOVERLAY_H_ */
