/*
 * GtkImageOverlayText.h
 *
 *  Created on: 12 avr. 2015
 *      Author: jfellus
 */

#ifndef GTKIMAGEOVERLAYTEXT_H_
#define GTKIMAGEOVERLAYTEXT_H_


#include <pg.h>
#include <image.h>

#include "gtk/gtk.h"



class GtkImageOverlayText {
public:

	std::string title;

private:
	ImageViewerComponent* viewer;
	ImageRGB tmp;

public:

	GtkImageOverlayText() {viewer = 0; title="image";}

	void init() {
		viewer = new ImageViewerComponent(title);
	}

	void process(const ImageRGB& in, const Matrix& overlay_values, const Matrix& overlays_points) {
		viewer->set_image(in.data, in.w, in.h);
		viewer->clear_overlay_texts();
		for(uint i=0; i<overlay_values.h; i++) {
			const float* f = &overlays_points[i*overlays_points.w];
			std::string s = TOSTRING(overlay_values(i,0));
			viewer->add_overlay_text((int)f[0], (int)f[1], s.c_str());
		}
		viewer->swap();
	}

	void process(const Image& in, const Matrix& overlay_values, const Matrix& overlays_points) {
		tmp.init(in.w, in.h);
		for(uint i=in.n; i--;) tmp[i*3] = tmp[i*3+1] = tmp[i*3+2] = in[i]*255;
		process(tmp, overlay_values, overlays_points);
	}

};






#endif /* GTKIMAGEOVERLAYTEXT_H_ */
