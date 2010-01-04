/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__
#define __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__

#include "juce_Image.h"


//==============================================================================
/**
    Represents a filter kernel to use in convoluting an image.

    @see Image::applyConvolution
*/
class JUCE_API  ImageConvolutionKernel
{
public:
    //==============================================================================
    /** Creates an empty convulution kernel.

        @param size     the length of each dimension of the kernel, so e.g. if the size
                        is 5, it will create a 5x5 kernel
    */
    ImageConvolutionKernel (const int size);

    /** Destructor. */
    ~ImageConvolutionKernel();

    //==============================================================================
    /** Resets all values in the kernel to zero.
    */
    void clear();

    /** Sets the value of a specific cell in the kernel.

        The x and y parameters must be in the range 0 < x < getKernelSize().

        @see setOverallSum
    */
    void setKernelValue (const int x,
                         const int y,
                         const float value);

    /** Rescales all values in the kernel to make the total add up to a fixed value.

        This will multiply all values in the kernel by (desiredTotalSum / currentTotalSum).
    */
    void setOverallSum (const float desiredTotalSum);

    /** Multiplies all values in the kernel by a value. */
    void rescaleAllValues (const float multiplier);

    /** Intialises the kernel for a gaussian blur.

        @param blurRadius   this may be larger or smaller than the kernel's actual
                            size but this will obviously be wasteful or clip at the
                            edges. Ideally the kernel should be just larger than
                            (blurRadius * 2).
    */
    void createGaussianBlur (const float blurRadius);

    //==============================================================================
    /** Returns the size of the kernel.

        E.g. if it's a 3x3 kernel, this returns 3.
    */
    int getKernelSize() const               { return size; }

    /** Returns a 2-dimensional array of the kernel's values.

        The size of each dimension of the array will be getKernelSize().
    */
    float** getValues() const               { return values; }

    //==============================================================================
    /** Applies the kernel to an image.

        @param destImage        the image that will receive the resultant convoluted pixels.
        @param sourceImage      an optional source image to read from - if this is 0, then the
                                destination image will be used as the source. If an image is
                                specified, it must be exactly the same size and type as the destination
                                image.
        @param x                the region of the image to apply the filter to
        @param y                the region of the image to apply the filter to
        @param width            the region of the image to apply the filter to
        @param height           the region of the image to apply the filter to
    */
    void applyToImage (Image& destImage,
                       const Image* sourceImage,
                       int x,
                       int y,
                       int width,
                       int height) const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    HeapBlock <float> values;
    const int size;

    // no reason not to implement these one day..
    ImageConvolutionKernel (const ImageConvolutionKernel&);
    const ImageConvolutionKernel& operator= (const ImageConvolutionKernel&);
};


#endif   // __JUCE_IMAGECONVOLUTIONKERNEL_JUCEHEADER__
