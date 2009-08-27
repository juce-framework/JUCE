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

#ifndef __JUCE_IMAGEBRUSH_JUCEHEADER__
#define __JUCE_IMAGEBRUSH_JUCEHEADER__

#include "juce_Brush.h"
#include "../imaging/juce_Image.h"


//==============================================================================
/**
    A Brush that fills areas with tiled repetitions of an image.

    @see Brush, Graphics::setBrush, SolidColourBrush, GradientBrush
*/
class JUCE_API  ImageBrush  : public Brush
{
public:
    //==============================================================================
    /* Creates an image brush, ready for use in Graphics::setBrush().

       (x, y) is an anchor point for the top-left of the image
       A reference to the image passed in will be kept, so don't delete
       it within the lifetime of this object
    */
    ImageBrush (Image* const image,
                const int anchorX,
                const int anchorY,
                const float opacity) throw();

    /** Destructor. */
    ~ImageBrush() throw();

    //==============================================================================
    /** Returns the image currently being used. */
    Image* getImage() const throw()             { return image; }

    /** Returns the current anchor X position. */
    int getAnchorX() const throw()              { return anchorX; }

    /** Returns the current anchor Y position. */
    int getAnchorY() const throw()              { return anchorY; }

    /** Returns the current opacity. */
    float getOpacity() const throw()            { return opacity; }

    //==============================================================================
    Brush* createCopy() const throw();

    void applyTransform (const AffineTransform& transform) throw();

    void multiplyOpacity (const float multiple) throw();

    bool isInvisible() const throw();

    void paintPath (LowLevelGraphicsContext& context,
                    const Path& path, const AffineTransform& transform) throw();

    void paintRectangle (LowLevelGraphicsContext& context,
                         int x, int y, int w, int h) throw();

    void paintAlphaChannel (LowLevelGraphicsContext& context,
                            const Image& alphaChannelImage, int imageX, int imageY,
                            int x, int y, int w, int h) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    Image* image;
    int anchorX, anchorY;
    float opacity;

private:
    ImageBrush (const ImageBrush&);
    const ImageBrush& operator= (const ImageBrush&);

    void getStartXY (int& x, int& y) const throw();
};

#endif   // __JUCE_IMAGEBRUSH_JUCEHEADER__
