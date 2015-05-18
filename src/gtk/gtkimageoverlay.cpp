/*
 * gtkimageoverlay.cpp
 *
 *  Created on: 29 avr. 2015
 *      Author: jfellus
 */


#include "gtkimageoverlay.h"
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo/cairo.h>





GtkImageOverlayComponent::GtkImageOverlayComponent(const std::string& title) {
	data = new DisplayData();
	data2 = new DisplayData();
	bClick = false;

	comp = new GtkViewer(title, this);
}


void GtkImageOverlayComponent::set_image(const unsigned char* data, uint w, uint h) {
	this->data->set_image(data, w, h);
	comp->w = w;
	comp->h = h;
}

void GtkImageOverlayComponent::set_overlay_points_image(const float* data, uint w, uint h) {
	this->data->overlay_points.clear();
	for(uint y=0; y<h; y++) {
		for(uint x=0; x<w; x++) {
			if(data[y*w+x]>0.0001) {
				this->data->add_overlay_point(OverlayPoint(x,y));
			}
		}
	}
}

void GtkImageOverlayComponent::add_overlay_image(int x, int y, const float* data, uint w, uint h) {
	this->data->add_overlay_image(OverlayImage(x,y,w,h,data));
}

void GtkImageOverlayComponent::clear_overlay_images() {
	this->data->overlay_images.clear();
}

void GtkImageOverlayComponent::add_overlay_text(int x, int y, const char* s) {
	this->data->add_overlay_text(OverlayText(x,y,s));
}

void GtkImageOverlayComponent::clear_overlay_texts() {
	this->data->overlay_texts.clear();
}


void GtkImageOverlayComponent::set_overlay_point(int x, int y) {
	this->data->overlay_points.clear();
	this->data->add_overlay_point(OverlayPoint(x,y));
}

void GtkImageOverlayComponent::clear_overlay_points() {
	this->data->overlay_points.clear();
}

void GtkImageOverlayComponent::add_overlay_point(int x, int y) {
	this->data->add_overlay_point(OverlayPoint(x,y));
}





void GtkImageOverlayComponent::swap() {
	comp->LOCK();
	DisplayData* tmp = data;
	data = data2;
	data2 = tmp;
	bClick = false;
	comp->repaint();
	comp->UNLOCK();
}

void GtkImageOverlayComponent::draw(void* _cr, GtkViewer* v) {
	if(data2->img) v->draw_image(data2->img, data2->w, data2->h);

	v->set_color(1,0,0);
	for(uint i=0; i<data2->overlay_points.size(); i++)
		v->draw_point(data2->overlay_points[i].x, data2->overlay_points[i].y);

	for(uint i=0; i<data2->overlay_images.size(); i++)
		v->draw_image_rect(data2->overlay_images[i].img, data2->overlay_images[i].x -data2->overlay_images[i].w/2, data2->overlay_images[i].y-data2->overlay_images[i].h/2, data2->overlay_images[i].w, data2->overlay_images[i].h);

	for(uint i=0; i<data2->overlay_texts.size(); i++)
		v->draw_text_circle(data2->overlay_texts[i].x, data2->overlay_texts[i].y, data2->overlay_texts[i].text);
}

void GtkImageOverlayComponent::on_click(double x, double y) {
	bClick = true;
}
