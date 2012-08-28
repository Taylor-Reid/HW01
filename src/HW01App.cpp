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
 *
 * @note This project satisfies goals A.1 (rectangle), A.2 (circle), B.1 (blur), E.2 (transparency),
 * E.5 (animation) and E.6 (mouse interaction)
 */

#include "cinder/app/AppBasic.h" /*copy pastes things*/
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "Resources.h"

using namespace ci;/*cinder namespace for accessing cinder functions*/
using namespace ci::app;/*indicates computer should try namespace if function location is not defined*/
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
	boost::posix_time::ptime app_start_time_;
	
	struct rings_info{
		int x;
		int y;
		int r;
	};
	deque<rings_info> rings_list_;
	deque<rings_info> accident_list_;
	
	uint8_t* my_blur_pattern_;
	
	//Width and height of the screen
	static const int kAppWidth=800;
	static const int kAppHeight=600;
	static const int kTextureSize=1024; //Must be the next power of 2 bigger or equal to app dimensions
	
	/**
	 * Fill a rectange with a checkerboard pattern.
	 *
	 * Fill the rectangle between (x1,y1) and (x2,y2) with a checkerboard pattern. The pattern is made up of
	 * rectangles of width and height specified by rect_width and rect_height. x1, y1, x2, and y2 are allowed to be
	 * negative and/or larger than the extents of the screen. The border is always 1 pixel in width.
	 *
	 * This satisfies the "rectangle" requirement, goal A.1
	 */
	void tileWithRectangles(uint8_t* pixels, int x1, int y1, int x2, int y2, int rect_width, int rect_height, Color8u fill1, Color8u border1, Color8u fill2, Color8u border2);

	/**
	 * Blur one image, using another as a template.
	 *
	 * Blur the image_to_blur using a standard convolution blur, but the strength of the blur depends on the blur_pattern.
	 * Pixels are fulling blurred if they are black not blurred at all if they are white. Both images must be exactly the
	 * same size as the app window.
	 *
	 * This satisfies the "blur" requirement, goal B.1
	 */
	void selectiveBlur(uint8_t* image_to_blur, uint8_t* blur_pattern);
	
	
	/**
	 * Draw concentric circles, each of width 7, until reaching r.
	 *
	 * This satisfies the "circle" requirement, goal A.2 and "transparency" (E.2)
	 */
	void drawRings(uint8_t* pixels, int x, int y, int r, Color8u c);
	
	/**
	 * This was the original version of drawRings, which was buggy, but looked cool,
	 * so I kept it.
	 */
	void drawAccident(uint8_t* pixels, int x, int y, int r, Color8u c);
};

void HW01App::prepareSettings(Settings* settings){
	(*settings).setWindowSize(kAppWidth,kAppHeight);
	(*settings).setResizable(false);
}

//This function takes about 15.265 ms for 800x600
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
	
	//Variable creation can be slow, so moved it outside the loop
	Color8u c = Color8u(255,0,0);
	int y_distance_from_start;
	int rects_tall;
	int rect_row;
	int y;
	bool in_horiz_border;
	
	int x_distance_from_start;
	int rects_along;
	int rect_col;
	int x;
	bool in_vert_border;
	
	//I do the loops with x on the inside because this incurs less cache misses
	for(y=((starty >= 0) ? starty : 0); y<=endy; y++){
		y_distance_from_start = y - starty;
		rects_tall = y_distance_from_start/rect_height; //How many squares down from the top of the board?
		
		rect_row = y_distance_from_start%rect_height;
		in_horiz_border = (rect_row == 0 || rect_row == rect_height-1);
		
		for(x=((startx >= 0) ? startx : 0); x<=endx; x++){
			x_distance_from_start = x - startx;
			rects_along = x_distance_from_start/rect_width; //How many squares along from the left of the board?
			
			rect_col = x_distance_from_start%rect_width;
			in_vert_border = (rect_col == 0 || rect_col == rect_width-1);
			
			//This is what makes the checkerboard pattern.
			if((rects_tall + rects_along)%2 == 0){
				//I originally had c = fill1, but it turns out that is REALLY slow. Probably causes a copy
				// constructor to get called!
				c.r = fill1.r;
				c.b = fill1.b;
				c.g = fill1.g;
				if(in_horiz_border || in_vert_border){
					c.r = border1.r;
					c.b = border1.b;
					c.g = border1.g;
				}
			} else {
				c.r = fill2.r;
				c.b = fill2.b;
				c.g = fill2.g;
				if(in_horiz_border || in_vert_border){
					c.r = border2.r;
					c.b = border2.b;
					c.g = border2.g;
				}
			}
			pixels[3*(x + y*kTextureSize)] = c.r;
			pixels[3*(x + y*kTextureSize)+1] = c.g;
			pixels[3*(x + y*kTextureSize)+2] = c.b;
		}
	}
}

void HW01App::selectiveBlur(uint8_t* image_to_blur, uint8_t* blur_pattern){
	//Convolution filters tend to overwrite the data that you need, so
	// we keep a temporary copy of the image_to_blur. There are certainly
	// more efficient ways to deal with this problem, but this is simple to
	// understand. 
	static uint8_t work_buffer[3*kTextureSize*kTextureSize];
	//This memcpy is not much of a performance hit.
	memcpy(work_buffer,image_to_blur,3*kTextureSize*kTextureSize);
	
	//These are used in right shifts.
	//Both of these kernels actually darken as well as blur.
	uint8_t kernelA[9] = 
	   {4,3,4,
		4,3,4,
		4,3,4};
	uint8_t kernelB[9] = 
	   {4,3,4,
		4,2,4,
		4,3,4};
	
	uint8_t total_red  =0;
	uint8_t total_green=0;
	uint8_t total_blue =0;
	int offset;
	int k;
	int y,x,ky,kx;
	
	//Visit every pixel in the image, except the ones on the edge.
	//TODO Special purpose logic to handle the edge cases
	for( y=1;y<kAppHeight-1;y++){
		for( x=1;x<kAppWidth-1;x++){
			
			offset = 3*(x + y*kAppWidth);
			if(blur_pattern[offset] < 256/3){
				//Compute the convolution of the kernel with the region around the current pixel
				//I use ints for the totals and the kernel to avoid overflow
				total_red=0;
				total_green=0;
				total_blue=0;
				for( ky=-1;ky<=1;ky++){
					for( kx=-1;kx<=1;kx++){
						offset = 3*(x + kx + (y+ky)*kTextureSize);
						k = kernelA[kx+1 + (ky+1)*3];
						total_red   += (work_buffer[offset  ] >> k);
						total_green += (work_buffer[offset+1] >> k);
						total_blue  += (work_buffer[offset+2] >> k);
					}
				}
			} else if(blur_pattern[offset] < 2*256/3){
				//Compute the convolution of the kernel with the region around the current pixel
				//I use ints for the totals and the kernel to avoid overflow
				total_red=0;
				total_green=0;
				total_blue=0;
				for( ky=-1;ky<=1;ky++){
					for( kx=-1;kx<=1;kx++){
						offset = 3*(x + kx + (y+ky)*kTextureSize);
						k = kernelB[kx+1 + (ky+1)*3];
						total_red   += (work_buffer[offset  ] >> k);
						total_green += (work_buffer[offset+1] >> k);
						total_blue  += (work_buffer[offset+2] >> k);
					}
				}
			} else {
				offset = 3*(x + y*kTextureSize);
				total_red   = work_buffer[offset];
				total_green = work_buffer[offset+1];
				total_blue  = work_buffer[offset+2];
			}
			
			offset = 3*(x + y*kTextureSize);
			image_to_blur[offset]   = total_red;
			image_to_blur[offset+1] = total_green;
			image_to_blur[offset+2] = total_blue;
		}
	}
}


void HW01App::setup()
{
	frame_number_=0;
	
	//This is the setup that everyone needs to do
	mySurface_ = new Surface(kTextureSize,kTextureSize,false);
	myTexture_ = new gl::Texture(*mySurface_);
	
	//Setup for my blur function
	Surface baby_picture(loadImage( loadResource(RES_BABY) ));
	uint8_t* blur_data = baby_picture.getData();	
	my_blur_pattern_ = new uint8_t[kAppWidth*kAppHeight*3];
	for(int y=0;y<kAppHeight;y++){
		for(int x=0;x<kAppWidth;x++){
			int offset = 3*(x + y*kAppWidth);
			my_blur_pattern_[offset] = blur_data[offset];
		}
	}
	
	//Setup for my rings
	rings_info t;
	t.x = 700;
	t.y = 200;
	t.r = 7*50;
	rings_list_.push_back(t);
	t.x = 600;
	t.y = 100;
	rings_list_.push_back(t);
		
}

void HW01App::mouseDown( MouseEvent event )
{
	//Satisfies E.6, though it is debatable whether or not this is an "interesting" interaction
	rings_info t;
	t.x = event.getX();
	t.y = event.getY();
	t.r = 7*50;
	
	if(rand() % 5 != 0){
		rings_list_.push_back(t);
	} else {
		accident_list_.push_back(t);
	}
}

void HW01App::drawRings(uint8_t* pixels, int center_x, int center_y, int r, Color8u c){
	//Bounds test
	if(r <= 0) return;
	
	for(int y=center_y-r; y<=center_y+r; y++){
		for(int x=center_x-r; x<=center_x+r; x++){
			//Bounds test, to make sure we don't access array out of bounds
			if(y < 0 || x < 0 || x >= kAppWidth || y >= kAppHeight) continue;
			
			int dist = (int)sqrt((double)((x-center_x)*(x-center_x) + (y-center_y)*(y-center_y)));
			if(dist <= r){
				if((dist/7)%2 == 1 ){
					int offset = 3*(x + y*kTextureSize);
					//By blending the colors I get a semi-transparent effect
					pixels[offset] = pixels[offset]/2 + c.r/2;
					pixels[offset+1] = pixels[offset+1]/2 + c.g/2;
					pixels[offset+2] = pixels[offset+2]/2 + c.b/2;
				}
			}
		}
	}
}

void HW01App::drawAccident(uint8_t* pixels, int center_x, int center_y, int r, Color8u c){
	//Bounds test
	if(r <= 0) return;
	
	int r2 = r*r;
	for(int y=center_y-r; y<=center_y+r; y++){
		for(int x=center_x-r; x<=center_x+r; x++){
			//Bounds test, to make sure we don't access array out of bounds
			if(y < 0 || x < 0 || x >= kAppWidth || y >= kAppHeight) continue;
			
			int dist = (x-center_x)*(x-center_x) + (y-center_y)*(y-center_y);
			if(dist <= r2){
				if((dist/49)%2 == 0){
					int offset = 3*(x + y*kTextureSize);
					pixels[offset] = c.r;
					pixels[offset+1] = c.g;
					pixels[offset+2] = c.b;
				}
			}
		}
	}
}

void HW01App::update()
{
	//Get our array of pixel information
	uint8_t* dataArray = (*mySurface_).getData();
	
	
	
	//
	// Creative bits go here
	//
	
	Color8u fill1 = Color8u(128,128,192);
	Color8u border1 = Color8u(192,192,255);
	Color8u fill2 = Color8u(192,192,192);
	Color8u border2 = Color8u(255,255,255);
	//With just this method called, frame rate drops from 54 to 53.5.
	tileWithRectangles(dataArray, -(frame_number_%14), -(frame_number_%14), 800, 600, 7, 7, fill1, border1, fill2, border2);
	
	//With just this method called, frame rate drops from 54 to 11.93
	selectiveBlur(dataArray, my_blur_pattern_);
	
	while(rings_list_.size() > 0 && rings_list_[0].r <= 0) rings_list_.pop_front();
	while(accident_list_.size() > 0 && accident_list_[0].r <= 0) accident_list_.pop_front();
	
	for(unsigned int i=0;i<rings_list_.size();i++){
		rings_info t = rings_list_[i];
		drawRings(dataArray, t.x, t.y, t.r, Color8u(249,132,229));
		rings_list_[i].r -= 4;
	}
	for(unsigned int i=0;i<accident_list_.size();i++){
		rings_info t = accident_list_[i];
		drawAccident(dataArray, t.x, t.y, t.r, Color8u(249,132,229));
		accident_list_[i].r -= 4;
	}
	
	//
	// End creative bits
	//
	
	
	
	//Update the Texture we are drawing using the pixels we just wrote
	(*myTexture_).update(*mySurface_,(*mySurface_).getBounds());
	
	//Only save the first frame of drawing as output
	if(frame_number_ == 0){
		writeImage("brinkmwj.png",*mySurface_);
		//We do this here, instead of setup, because we don't want to count the writeImage time in our estimate
		app_start_time_ = boost::posix_time::microsec_clock::local_time();
	}
	//keeps track of how many frames we have shown.
	frame_number_++;
	
	//For debugging: Print the actual frames per second
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration msdiff = now - app_start_time_;
    console() << (1000.0*frame_number_) / msdiff.total_milliseconds() << std::endl;
}

void HW01App::draw()
{
	//Draw our texture to the screen, using graphics library
	gl::draw(*myTexture_);
}

CINDER_APP_BASIC( HW01App, RendererGl )
