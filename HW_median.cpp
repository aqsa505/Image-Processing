#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;

template <typename TwoDimension>
void copyRowToBuffer(TwoDimension& circularBuffer, ImagePtr I1, int row, int sz);
template <typename TwoDimension>
void createHorizontalPadding(TwoDimension& circularBuffer, short borderRow[], int start, int sz, int width);
template <typename TwoDimension>
uchar medianFilter(TwoDimension& circularBuffer, vector<uchar>& kernelArray, int current, int sz);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is in I2.
//
void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{
  IP_copyImageHeader(I1, I2);

  int width = I1->width();
  int height = I1->height();
  int total = width * height;
  int type;

  // make sz the next odd number for odd sz x sz kernel
  if (sz % 2 == 0) {
    sz += 1;
  }

  int bufferPadding = sz / 2;
  int bufferRow = sz;

  // create a vector to hold values of size |sz x sz|
  vector<uchar> kernelArray(sz * sz);

  // create circular buffer
  short** circularBuffer = new short*[bufferRow];
  for (int i = 0; i < bufferRow; ++i) {
    circularBuffer[i] = new short[width + (2 * bufferPadding)];
  }

  // pre-populate the first "sz/2" rows onto buffer
  for (int i = 0; i < bufferPadding; ++i) {
    copyRowToBuffer(circularBuffer, I1, i, sz);
  }

  // populate top padding
  for (int i = 0; i < bufferPadding; ++i) {
    createHorizontalPadding(circularBuffer, circularBuffer[bufferPadding], i, sz, width);
  }
   
  // process image using median filter
  ChannelPtr<uchar> out;
  for (int ch = 0; IP_getChannel(I2, ch, out, type); ch++) {
    for (int y = 0; y < height; ++y) {
      copyRowToBuffer(circularBuffer, I1, y + bufferPadding, sz);
    
      for (int x = 0; x < width; ++x) {
        *out = medianFilter(circularBuffer, kernelArray, x, sz);

        out++;
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
uchar medianFilter(TwoDimension& circularBuffer, vector<uchar>& kernelArray, int current, int sz) {
  // grab all values within the median filter
  for (int i = 0; i < sz; ++i) {
    for (int j = current; j < current + sz; j++) {
      kernelArray.push_back(circularBuffer[i][j]);
    }
  }

  // sort the values in the median filter kernel
  sort(kernelArray.begin(), kernelArray.end());

  // find median
  uchar median = kernelArray[(sz * sz) / 2];

  // clean median filter kernel
  kernelArray.clear();

  return median;
}

template <typename TwoDimension>
void createHorizontalPadding(TwoDimension& circularBuffer, short borderRow[], int start, int sz, int width) {
  int bufferPadding = sz / 2;
  int bufferRow = sz;
  int i;
  int j = 0;

  // fill in left padding for top padding
  for (i = 0; i < bufferPadding; ++i) {
    circularBuffer[start % bufferRow][i] = borderRow[j];
  }

  // fill in the top padding
  for (; i < (width + bufferPadding); ++i, ++j) {
    circularBuffer[start % bufferRow][i] = borderRow[j];
  }

  // fill in right padding for top padding
  for (; i < (width + (2 * bufferPadding)); ++i) {
    circularBuffer[start % bufferRow][i] = borderRow[j];
  }
}

template <typename TwoDimension>
void copyRowToBuffer(TwoDimension& circularBuffer, ImagePtr I1, int row, int sz) {
  ChannelPtr<uchar> p;
  int width = I1->width();
  int type;
  int bufferPadding = sz / 2;
  int bufferRow = sz;

  // advance channel pointer to appropriate row
  for (int ch = 0; IP_getChannel(I1, ch, p, type); ch++) {
    for (int i = 0; i < row; ++i) {
      p += width;
    }
  }

  int i;

  // fill in left padding
  for (i = 0; i < bufferPadding; ++i) {
    circularBuffer[(row + bufferPadding) % bufferRow][i] = *p;
  }

  // fill in the image onto buffer
  for (; i < (width + bufferPadding); ++i) {
    circularBuffer[(row + bufferPadding) % bufferRow][i] = *p++;
  }

  // fill in right padding
  for (; i < (width + (2 * bufferPadding)); ++i) {
    circularBuffer[(row + bufferPadding) % bufferRow][i] = *p;
  }

  return;
}
