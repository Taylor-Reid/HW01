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
	
	int frame_number_;
	
	static const int kAppWidth=800;
	static const int kAppHeight=600;

};

void HW01App::prepareSettings(Settings* settings){
	(*settings).setWindowSize(kAppWidth,kAppHeight);
	(*settings).setResizable(false);
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
	
	//Set all the pixels to red
	for(int i=0; i< (*mySurface_).getWidth()*(*mySurface_).getHeight(); i++){
		dataArray[3*i] = 255;
		dataArray[3*i+1] = 0;
		dataArray[3*i+2] = 0;
	}
	
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
