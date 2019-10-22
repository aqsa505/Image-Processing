#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void blur1D(ChannelPtr<uchar>, int, int, int, ChannelPtr<uchar>);

void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	ImagePtr I3;
	IP_copyImageHeader(I1, I3);

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, src, dest, endd;
	int type;

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, src, type);
		IP_getChannel(I3, ch, dest, type);

		//check if filterW is greater then 1 then blur horizontal
		if (filterW == 1) {
			for (endd = p1 + total; p1 < endd;)
				*dest++ = *p1++;
		}
		else if (filterW > 1) {
			for (int i = 0; i < h; i++) {
				blur1D(p1, w, 1, filterW, dest);
				p1 += w;
				dest += w;
			}
		}
	}
	dest = dest - total;

	//check if filterW is greater then 1 then blur horizontal
	if (filterH == 1) {
		for (endd = dest + total; dest < endd;)
			*src++ = *dest++;
	}

	else if (filterH > 1) {
		for (int j = 0; j < w; j++) {
			blur1D(dest, h, w, filterH, src);
			src += 1;
			dest += 1;
		}
	}
}


void blur1D(ChannelPtr<uchar>src, int len, int stride, int ww, ChannelPtr<uchar>dst)
{
	int i, j;
	int k = 0;
	if (ww % 2 == 0) ww++;
	int padding = ww  / 2;
	int bufferWidth = len + ww - 1;
	short* buffer = new short[bufferWidth];

	for (i = 0; i < padding; i++) {
		buffer[i] = *src;
	}

	for (i = padding; i < padding + len; i++) {
		buffer[i] = src[k];
		k += stride;
	}

	for (i = (padding + len); i < padding + len; i++) {
		int index = i - stride;
		buffer[i] = src[index];
	}

	long sum = 0;
	for (i = 0; i < ww; i++) {

	}
	for (i = 0; i < ww; i++) {
		sum += buffer[i];
	}
	for (int i = 0; i < len; i++) {

	}
	for (int i = 0; i < len; i++) {
		dst[i*stride] = sum / ww;
		sum += (buffer[i + ww] - buffer[i]);
	}

}

