/*
 * GtkImageRecords.h
 *
 *  Created on: 29 avr. 2015
 *      Author: jfellus
 */

#ifndef GTKIMAGERECORDS_H_
#define GTKIMAGERECORDS_H_

#include <pg.h>
#include <image.h>
#include <matrix.h>
#include "gtk/gtkimagerecords.h"

class GtkImageRecords {
public:
	std::string title;
	bool click;
private:
		GtkImageRecordsComponent* comp;

public:
	GtkImageRecords() {
		title = "ImageRecords";
		comp = NULL;
		click = false;
	}

	void init() {
		comp = new GtkImageRecordsComponent(title);
	}

	void process(const ImageRGB& img, const Matrix& records_activities, const std::vector<ImageRGB>& records) {
		click = comp->bClick;
		comp->set_data(img, records_activities, records);
		comp->swap();
	}
};



#endif /* GTKIMAGERECORDS_H_ */
