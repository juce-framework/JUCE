/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_IMAGECONVOLUTIONKERNEL_H_INCLUDED
#define JUCE_IMAGECONVOLUTIONKERNEL_H_INCLUDED


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
    ImageConvolutionKernel (int size);

    /** Destructor. */
    ~ImageConvolutionKernel();

    //==============================================================================
    /** Resets all values in the kernel to zero. */
    void clear();

    /** Returns one of the kernel values. */
    float getKernelValue (int x, int y) const noexcept;

    /** Sets the value of a specific cell in the kernel.

        The x and y parameters must be in the range 0 < x < getKernelSize().

        @see setOverallSum
    */
    void setKernelValue (int x, int y, float value) noexcept;

    /** Rescales all values in the kernel to make the total add up to a fixed value.

        This will multiply all values in the kernel by (desiredTotalSum / currentTotalSum).
    */
    void setOverallSum (float desiredTotalSum);

    /** Multiplies all values in the kernel by a value. */
    void rescaleAllValues (float multiplier);

    /** Intialises the kernel for a gaussian blur.

        @param blurRadius   this may be larger or smaller than the kernel's actual
                            size but this will obviously be wasteful or clip at the
                            edges. Ideally the kernel should be just larger than
                            (blurRadius * 2).
    */
    void createGaussianBlur (float blurRadius);

    //==============================================================================
    /** Returns the size of the kernel.

        E.g. if it's a 3x3 kernel, this returns 3.
    */
    int getKernelSize() const               { return size; }

    //==============================================================================
    /** Applies the kernel to an image.

        @param destImage        the image that will receive the resultant convoluted pixels.
        @param sourceImage      the source image to read from - this can be the same image as
                                the destination, but if different, it must be exactly the same
                                size and format.
        @param destinationArea  the region of the image to apply the filter to
    */
    void applyToImage (Image& destImage,
                       const Image& sourceImage,
                       const Rectangle<int>& destinationArea) const;

private:
    //==============================================================================
    HeapBlock<float> values;
    const int size;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageConvolutionKernel)
};


#endif   // JUCE_IMAGECONVOLUTIONKERNEL_H_INCLUDED
