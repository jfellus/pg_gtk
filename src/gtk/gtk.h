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

class ImageViewerComponent {
public:
	void* pixbuf;
	void* image_viewer;

private:
	bool bFirstDraw;
	double offsetx, offsety, _zoom;

public:
	ImageViewerComponent(const std::string& title);

	void set_image(const unsigned char* data, uint w, uint h);

	void repaint();

	void zoom(float fzoom, double cx, double cy);
	void zoom(double x, double y, double w, double h);
	void move(double dx, double dy);

	int get_width();
	int get_height();

	void draw();
};


#endif /* GTK_H_ */
