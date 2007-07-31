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

#ifndef __JUCE_SOLIDCOLOURBRUSH_JUCEHEADER__
#define __JUCE_SOLIDCOLOURBRUSH_JUCEHEADER__

#include "juce_Brush.h"
#include "../colour/juce_Colour.h"


//==============================================================================
/**
    A Brush that fills its area with a solid (or semi-transparent) colour.

    An application won't normally need to use this class directly, as drawing
    with solid colours is taken care of automatically by the Graphics class
    (it actually uses one of these brushes internally when you set the colour
    with the Graphics::setColour() method).

    @see Brush, Graphics::setBrush, GradientBrush, ImageBrush
*/
class JUCE_API  SolidColourBrush  : public Brush
{
public:
    //==============================================================================
    /** Creates a SolidColourBrush to draw with the given colour.

        The colour can be changed later with the setColour() method.
    */
    SolidColourBrush (const Colour& colour) throw();

    /** Creates a SolidColourBrush set to black.

        The colour can be changed later with the setColour() method.
    */
    SolidColourBrush() throw();

    /** Destructor. */
    ~SolidColourBrush() throw();

    //==============================================================================
    /** Returns the colour currently being used. */
    const Colour& getColour() const throw()             { return colour; }

    /** Sets the colour to use for drawing. */
    void setColour (const Colour& newColour) throw()    { colour = newColour; }

    //==============================================================================
    Brush* createCopy() const throw();

    void applyTransform (const AffineTransform& transform) throw();

    bool isInvisible() const throw();

    void multiplyOpacity (const float multiple) throw();

    void paintPath (LowLevelGraphicsContext& context,
                    const Path& path, const AffineTransform& transform) throw();

    void paintRectangle (LowLevelGraphicsContext& context,
                         int x, int y, int w, int h) throw();

    void paintAlphaChannel (LowLevelGraphicsContext& context,
                            const Image& alphaChannelImage, int imageX, int imageY,
                            int x, int y, int w, int h) throw();

    void paintVerticalLine (LowLevelGraphicsContext& context,
                            int x, float y1, float y2) throw();

    void paintHorizontalLine (LowLevelGraphicsContext& context,
                              int y, float x1, float x2) throw();

    void paintLine (LowLevelGraphicsContext& context,
                    float x1, float y1, float x2, float y2) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Colour colour;

    SolidColourBrush (const SolidColourBrush&);
    const SolidColourBrush& operator= (const SolidColourBrush&);
};

#endif   // __JUCE_SOLIDCOLOURBRUSH_JUCEHEADER__
