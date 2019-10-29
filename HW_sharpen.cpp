#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//

// linkage with Blur 
extern void HW_blur(ImagePtr, int, int, ImagePtr);
extern void HW_blur1D(ChannelPtr<uchar>, int, int, int, ChannelPtr<uchar>);

void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2,endd;
	int type, value;
	
	//check size and factor is 1 then show the orignal image 
	if (size == 1) {
		for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
			IP_getChannel(I2, ch, p2, type);
			for (endd = p1 + total; p1 < endd;) *p2++ = *p1++;
		}

	}
	else{
		// blur the orignal image 
		HW_blur(I1, size, size, I2);

		for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
			IP_getChannel(I2, ch, p2, type);
			for (endd = p1 + total; p1 < endd;) {

             // subtracting blurred image from orignal image and
             // multiply by factor
             // add to orignal image
				value = factor * ((int)*p1 - *p2) + *p1;
			
			// clipping is used so that image is within range[0,255] 	
				*p2 = CLIP(value, 0, MaxGray);
				*p1++;
				*p2++;
			}
		}
	}
}
