#include "IP.h"
using namespace IP;

template <typename TwoDimension>
void copyRowToCircBuffer(TwoDimension& circularBuffer, ImagePtr I1, int row, int method);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//
void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{
  ImagePtr gammaI;

  IP_copyImageHeader(I1, I2);
  IP_copyImageHeader(I1, gammaI);

  int width = I1->width();
  int height = I1->height();
  int total = width * height;
  int threshold = MXGRAY / 2;
  int bufferRow;
  int bufferPadding;
  int lut[MXGRAY];
  ChannelPtr<uchar> in, gp, out;
  int type;

  if (method == 0) {
    bufferRow = 2;
    bufferPadding = 1;
  }
  if (method == 1) {
    bufferRow = 3;
    bufferPadding = 2;
  }

  // construct LUT for gamma correction
  for (int i = 0; i < MXGRAY; ++i) {
    lut[i] = MaxGray * (pow((double)i / MaxGray, 1 / gamma));
  }
  
  // apply gamma correction to I1 prior to error diffusion
  for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {
    IP_getChannel(gammaI, ch, gp, type);
    for (int i = 0; i < total; ++i) {
      *gp++ = lut[*in++];
    }
  }

  // create circular buffer
  short** circularBuffer = new short*[bufferRow];
  for (int i = 0; i < bufferRow; ++i) {
    circularBuffer[i] = new short[width + (2 * bufferPadding)];
  }
   
  // process image using Floyd-Steinberg
  if (method == 0) {
    copyRowToCircBuffer(circularBuffer, gammaI, 0, method);
    for (int ch = 0; IP_getChannel(I2, ch, out, type); ch++) {
      for (int y = 0; y < height; ++y) {
        copyRowToCircBuffer(circularBuffer, gammaI, y + 1, method);

        if (serpentine && (y % 2 == 1)) {
          // assign pointers for right-to-left order
          short* in1 = circularBuffer[y % bufferRow] + (width + bufferPadding - 1);
          short* in2 = circularBuffer[(y + 1) % bufferRow] + (width + bufferPadding - 1);
          out += width;

          for (int x = (width + bufferPadding - 1); x >= bufferPadding; --x) {
            *out = (*in1 < threshold) ? 0 : 255;

            short error = *in1 - *out;
            in1[-1] += (error * 7 / 16.0);
            in2[1] += (error * 3 / 16.0);
            in2[0] += (error * 5 / 16.0);
            in2[-1] += (error * 1 / 16.0);

            in1--;
            in2--;
            out--;
          }
          // move output pointer to next row
          out += width;
        } else {
          // assign pointers for left-to-right order
          short* in1 = circularBuffer[y % bufferRow] + bufferPadding;
          short* in2 = circularBuffer[(y + 1) % bufferRow] + bufferPadding;

          for (int x = bufferPadding; x < (width + bufferPadding); ++x) {
            *out = (*in1 < threshold) ? 0 : 255;

            short error = *in1 - *out;
            in1[1] += (error * 7 / 16.0);
            in2[-1] += (error * 3 / 16.0);
            in2[0] += (error * 5 / 16.0);
            in2[1] += (error * 1 / 16.0);

            in1++;
            in2++;
            out++;
          }
        }
      }
    }
  } else {
    // process image using Jarvis-Judice-Ninke
    copyRowToCircBuffer(circularBuffer, gammaI, 0, method);
    copyRowToCircBuffer(circularBuffer, gammaI, 1, method);
    for (int ch = 0; IP_getChannel(I2, ch, out, type); ch++) {
      for (int y = 0; y < height; ++y) {
        copyRowToCircBuffer(circularBuffer, gammaI, y + 2, method);

        // switch between raster order and serpentine order
        if (serpentine && (y % 2 == 1)) {
          // assign pointers for right-to-left order
          short* in1 = circularBuffer[y % bufferRow] + (width + bufferPadding - 1);
          short* in2 = circularBuffer[(y + 1) % bufferRow] + (width + bufferPadding - 1);
          short* in3 = circularBuffer[(y + 2) % bufferRow] + (width + bufferPadding - 1);
          out += width;

          for (int x = (width + bufferPadding - 1); x >= bufferPadding; --x) {
            *out = (*in1 < threshold) ? 0 : 255;

            short error = *in1 - *out;
            in1[-1] += (error * 7 / 48.0);
            in1[-2] += (error * 5 / 48.0);
            in2[2] += (error * 3 / 48.0);
            in2[1] += (error * 5 / 48.0);
            in2[0] += (error * 7 / 48.0);
            in2[-1] += (error * 5 / 48.0);
            in2[-2] += (error * 3 / 48.0);
            in3[2] += (error * 1 / 48.0);
            in3[1] += (error * 3 / 48.0);
            in3[0] += (error * 5 / 48.0);
            in3[-1] += (error * 3 / 48.0);
            in3[-2] += (error * 1 / 48.0);

            in1--;
            in2--;
            in3--;
            out--;
          }
          out += width;
        } else {
          // assign pointers for left-to-right order
          short* in1 = circularBuffer[y % bufferRow] + bufferPadding;
          short* in2 = circularBuffer[(y + 1) % bufferRow] + bufferPadding;
          short* in3 = circularBuffer[(y + 2) % bufferRow] + bufferPadding;

          for (int x = bufferPadding; x < (width + bufferPadding); ++x) {
            *out = (*in1 < threshold) ? 0 : 255;

            short error = *in1 - *out;
            in1[1] += (error * 7 / 48.0);
            in1[2] += (error * 5 / 48.0);
            in2[-2] += (error * 3 / 48.0);
            in2[-1] += (error * 5 / 48.0);
            in2[0] += (error * 7 / 48.0);
            in2[1] += (error * 5 / 48.0);
            in2[2] += (error * 3 / 48.0);
            in3[-2] += (error * 1 / 48.0);
            in3[-1] += (error * 3 / 48.0);
            in3[0] += (error * 5 / 48.0);
            in3[1] += (error * 3 / 48.0);
            in3[2] += (error * 1 / 48.0);

            in1++;
            in2++;
            in3++;
            out++;
          }
        }
      }
    }
  }

  // deallocate circular buffer
  for (int i = 0; i < bufferRow; ++i) {
    delete[] circularBuffer[i];
  }
  delete[] circularBuffer;
  
  return;
}

template <typename TwoDimension>
void copyRowToCircBuffer(TwoDimension& circularBuffer, ImagePtr I1, int row, int method) {
  ChannelPtr<uchar> p;
  int width = I1->width();
  int type;
  int bufferRow;
  int bufferPadding;

  if (method == 0) {
    bufferRow = 2;
    bufferPadding = 1;
  }
  if (method == 1) {
    bufferRow = 3;
    bufferPadding = 2;
  }

  // advance channel pointer to appropriate row
  for (int ch = 0; IP_getChannel(I1, ch, p, type); ch++) {
    for (int i = 0; i < row; ++i) {
      p += width;
    }
  }

  int i;

  // fill in left padding
  for (i = 0; i < bufferPadding; ++i) {
    circularBuffer[row % bufferRow][i] = *p;
  }

  // fill in the image onto buffer
  for (; i < (width + bufferPadding); ++i) {
    circularBuffer[row % bufferRow][i] = *p++;
  }

  // fill in right padding
  for (; i < (width + (2 * bufferPadding)); ++i) {
    circularBuffer[row % bufferRow][i] = *p;
  }

  return;
}



