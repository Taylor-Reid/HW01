/**
 * @file HW01App.cpp
 * CSE 274 - Fall 2012
 * My solution for HW01.
 *
 * @author Bo Brinkman
 * @date 2012-08-24
 *
 * @note This file is (c) 2012. It is licensed under the 
 * CC BY 3.0 license (http://creativecommons.org/licenses/by/3.0/),
 * which means you are free to use, share, and remix it as long as you
 * give attribution. Commercial uses are allowed.
 *
 * @note The code fore creating Textures and Surfaces comes from another
 * of my projects, https://github.com/brinkmwj/CatPicture/blob/master/src/CatPictureApp.cpp
 */

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class HW01App : public AppBasic {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void prepareSettings(Settings* settings);
	
  private:
	Surface* mySurface_; //The Surface object whose pixel array we will modify
	gl::Texture* myTexture_; //The Texture object that we use to display our Surface in the window
	
	//Track how many frames we have shown, for animatino purposes
	int frame_number_;
	
	//Width and height of the screen
	static const int kAppWidth=800;
	static const int kAppHeight=600;
	
	/**
	 * Fill a rectange with a checkerboard pattern.
	 *
	 * Fill the rectangle between (x1,y1) and (x2,y2) with a checkerboard pattern. The pattern is made up of
	 * rectangles of width and height specified by rect_width and rect_height. x1, y1, x2, and y2 are allowed to be
	 * negative and/or larger than the extents of the screen. The border is always 1 pixel in width.
	 */
	void tileWithRectangles(uint8_t* pixels, int x1, int y1, int x2, int y2, int rect_width, int rect_height, Color8u fill1, Color8u border1, Color8u fill2, Color8u border2);

};

void HW01App::prepareSettings(Settings* settings){
	(*settings).setWindowSize(kAppWidth,kAppHeight);
	(*settings).setResizable(false);
}

void HW01App::tileWithRectangles(uint8_t* pixels, int x1, int y1, int x2, int y2, int rect_width, int rect_height, Color8u fill1, Color8u border1, Color8u fill2, Color8u border2){
	//Figure out the starting and ending coordinates of the rectangle to fill
	int startx = (x1 < x2) ? x1 : x2;
	int endx = (x1 < x2) ? x2 : x1;
	int starty = (y1 < y2) ? y1 : y2;
	int endy = (y1 < y2) ? y2 : y1;
	
	//Do some bounds checking
	if(endx < 0) return; //Visible part of rectangle is off screen
	if(endy < 0) return; //Visible part of rectangle is off screen
	if(startx >= kAppWidth) return; //Visible part of rectangle is off screen
	if(starty >= kAppHeight) return; //Visible part of rectangle is off screen
	if(endx >= kAppWidth) endx = kAppWidth-1;
	if(endy >= kAppHeight) endy = kAppHeight-1;
	
	//I do the loops with x on the inside because this incurs less cache misses
	for(int y=((starty >= 0) ? starty : 0); y<=endy; y++){
		int y_distance_from_start = y - starty;
		int rects_tall = y_distance_from_start/rect_height;
		int rect_row = y_distance_from_start%rect_height;
		bool in_horiz_border = (rect_row == 0 || rect_row == rect_height-1);
		
		for(int x=((startx >= 0) ? startx : 0); x<=endx; x++){
			int x_distance_from_start = x - startx;
			int rects_along = x_distance_from_start/rect_width;
			int rect_col = x_distance_from_start%rect_width;
			bool in_vert_border = (rect_col == 0 || rect_col == rect_width-1);
			
			Color8u c = fill1;
			if((rects_tall + rects_along)%2 == 0){
				c = fill1;
				if(in_horiz_border || in_vert_border){
					c = border1;
				}
			} else {
				c = fill2;
				if(in_horiz_border || in_vert_border){
					c = border2;
				}
			}
			pixels[3*(x + y*kAppWidth)] = c.r;
			pixels[3*(x + y*kAppWidth)+1] = c.g;
			pixels[3*(x + y*kAppWidth)+2] = c.b;
		}
	}
}

void HW01App::setup()
{
	frame_number_=0;
	
	mySurface_ = new Surface(kAppWidth,kAppHeight,false);
	myTexture_ = new gl::Texture(*mySurface_);
}

void HW01App::mouseDown( MouseEvent event )
{
}

void HW01App::update()
{
	//Get our array of pixel information
	uint8_t* dataArray = (*mySurface_).getData();
	
	
	
	//
	// Creative bits go here
	//
	
	Color8u fill1 = Color8u(0,0,192);
	Color8u border1 = Color8u(64,64,255);
	Color8u fill2 = Color8u(0,64,192);
	Color8u border2 = Color8u(64,128,255);
	tileWithRectangles(dataArray, -1, -(frame_number_%10), 801, 600, 802, 5, fill1, border1, fill2, border2);	
	//
	// End creative bits
	//
	
	
	
	//Update the Texture we are drawing using the pixels we just wrote
	(*myTexture_).update(*mySurface_,(*mySurface_).getBounds());
	
	//Only save the first frame of drawing as output
	if(frame_number_ == 0){
		writeImage("brinkmwj.png",*mySurface_);
	}
	//keeps track of how many frames we have shown.
	frame_number_++;
	
}

void HW01App::draw()
{
	//Draw our texture to the screen, using graphics library
	gl::draw(*myTexture_);
}

CINDER_APP_BASIC( HW01App, RendererGl )
