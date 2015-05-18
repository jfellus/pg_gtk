/*
 * gtkimagerecords.h
 *
 *  Created on: 29 avr. 2015
 *      Author: jfellus
 */

#ifndef gtk_GTKIMAGERECORDS_H_
#define gtk_GTKIMAGERECORDS_H_

#include "gtkviewer.h"
#include <image.h>
#include <matrix.h>



class GtkImageRecordsComponent : public Drawable {
	class DisplayData;
public:
	GtkViewer* comp;

	DisplayData* data;
	DisplayData* data2;

	bool bClick;

public:

	GtkImageRecordsComponent(const std::string& title);

	void set_data(const ImageRGB& img, const Matrix& records_activities, const std::vector<ImageRGB>& records) {
		data->set(img, records_activities, records);
		comp->w = img.w;
		comp->h = img.h;
	}

	void swap();

	virtual void draw(void* _cr, GtkViewer* v);
	virtual void on_click(double x, double y);


private:
	class DisplayData {
	public:
		ImageRGB img;
		Matrix records_activities;
		std::vector<ImageRGB> records;
		void set(const ImageRGB& img, const Matrix& records_activities, const std::vector<ImageRGB>& records);
	};
};



#endif /* GTKIMAGERECORDS_H_ */
