/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
