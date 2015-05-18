/*
 * gtkimagerecords.cpp
 *
 *  Created on: 29 avr. 2015
 *      Author: jfellus
 */


#include "gtkimagerecords.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>



GtkImageRecordsComponent::GtkImageRecordsComponent(const std::string& title)  {
	data = new DisplayData();
	data2 = new DisplayData();
	comp = new GtkViewer(title, this);
	bClick = false;
}

void GtkImageRecordsComponent::swap() {
	comp->LOCK();
	DisplayData* tmp = data;
	data = data2;
	data2 = tmp;
	bClick = false;
	comp->repaint();
	comp->UNLOCK();
}



void GtkImageRecordsComponent::draw(void* _cr, GtkViewer* v) {
	v->draw_image(data2->img.data, data2->img.w, data2->img.h);
	for(uint i=0; i<data2->records.size(); i++) {
		double w = data2->img.w / data2->records.size();
		double y = data2->img.h+30;
		double h = w/data2->records[i].w * data2->records[i].h;
		float f = data2->records_activities[i];
		double x =  (float)i*w;
		v->draw_image_rect(data2->records[i].data, data2->records[i].w, data2->records[i].h, x, y, w, h);
		v->set_color(1,0,0);
		v->fill_rect(x,y+h*(1-f),10,h*f);
	}
}

void GtkImageRecordsComponent::on_click(double x, double y) {
	bClick = true;
}


void GtkImageRecordsComponent::DisplayData::set(const ImageRGB& img, const Matrix& records_activities, const std::vector<ImageRGB>& records) {
	if(this->img && (img.w!=this->img.w || img.h!=this->img.h)) this->img.free();
	if(!this->img) this->img.init(img.w, img.h);
	memcpy(this->img.data, img.data, img.w*img.h*3);

	if(this->records_activities && (records_activities.w!=this->records_activities.w || records_activities.h!=this->records_activities.h)) this->records_activities.free();
	if(!this->records_activities) this->records_activities.init(records_activities.h, records_activities.w);
	memcpy(this->records_activities.data, records_activities.data, records_activities.n*sizeof(float));

	for(uint i=this->records.size(); i<records.size(); i++) this->records.push_back(records[i]);
}
