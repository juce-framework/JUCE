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

#ifndef __JUCE_BRUSH_JUCEHEADER__
#define __JUCE_BRUSH_JUCEHEADER__

class Path;
class AffineTransform;
class LowLevelGraphicsContext;
class Image;
class Graphics;


//==============================================================================
/**
    A brush is used to fill areas with colours, patterns, or images.

    The Graphics class has an idea of a current brush which it uses to render
    shapes, rectangles, lines, text, etc.

    This is the base class - there are subclasses for useful types of fill pattern,
    and applications can define their own brushes too.

    @see Graphics::setBrush, SolidColourBrush, GradientBrush, ImageBrush
*/
class JUCE_API  Brush
{
protected:
    //==============================================================================
    /** Creates a Brush.

        (Nothing much happens in the base class).
    */
    Brush() throw();

public:
    /** Destructor. */
    virtual ~Brush() throw();

    /** Creates a copy of whatever class of Brush this is. */
    virtual Brush* createCopy() const throw() = 0;

    /** Does whatever is relevent to transform the geometry of this brush. */
    virtual void applyTransform (const AffineTransform& transform) throw() = 0;

    /** Does whatever is relevent to change the opacity of this brush. */
    virtual void multiplyOpacity (const float multiple) throw() = 0;

    /** Must return true if this brush won't draw any pixels. */
    virtual bool isInvisible() const throw() = 0;

    //==============================================================================
    virtual void paintPath (LowLevelGraphicsContext& context,
                            const Path& path, const AffineTransform& transform) throw() = 0;

    virtual void paintRectangle (LowLevelGraphicsContext& context,
                                 int x, int y, int w, int h) throw() = 0;

    virtual void paintAlphaChannel (LowLevelGraphicsContext& context,
                                    const Image& alphaChannelImage, int imageX, int imageY,
                                    int x, int y, int w, int h) throw() = 0;

    virtual void paintVerticalLine (LowLevelGraphicsContext& context,
                                    int x, float y1, float y2) throw();

    virtual void paintHorizontalLine (LowLevelGraphicsContext& context,
                                      int y, float x1, float x2) throw();

    virtual void paintLine (LowLevelGraphicsContext& context,
                            float x1, float y1, float x2, float y2) throw();

private:
    //==============================================================================
    Brush (const Brush&);
    const Brush& operator= (const Brush&);
};

#endif   // __JUCE_BRUSH_JUCEHEADER__
