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

#ifndef __JUCE_DRAWABLEPATH_JUCEHEADER__
#define __JUCE_DRAWABLEPATH_JUCEHEADER__

#include "juce_Drawable.h"
#include "../colour/juce_ColourGradient.h"


//==============================================================================
/**
    A drawable object which renders a filled or outlined shape.

    @see Drawable
*/
class JUCE_API  DrawablePath  : public Drawable
{
public:
    //==============================================================================
    /** Creates a DrawablePath.
    */
    DrawablePath();

    /** Destructor. */
    virtual ~DrawablePath();

    //==============================================================================
    /** Changes the path that will be drawn.

        @see setSolidFill, setOutline
    */
    void setPath (const Path& newPath);

    /** Returns the current path. */
    const Path& getPath() const throw()                         { return path; }

    /** Sets a colour to fill the path with.

        This colour is used to fill the path - if you don't want the path to be
        filled (e.g. if you're just drawing an outline), set this colour to be
        transparent.

        @see setPath, setOutline
    */
    void setSolidFill (const Colour& newColour);

    /** Sets a custom brush to use to fill the path.

        @see setSolidFill
    */
    void setFillBrush (const Brush& newBrush);

    /** Returns the brush currently being used to fill the shape. */
    Brush* getCurrentBrush() const throw()                      { return fillBrush; }

    /** Changes the properties of the outline that will be drawn around the path.

        If the thickness value is 0, no outline will be drawn. If one is drawn, the
        colour passed-in here will be used for it.

        @see setPath, setSolidFill
    */
    void setOutline (const float thickness,
                     const Colour& outlineColour);

    /** Changes the properties of the outline that will be drawn around the path.

        If the stroke type has 0 thickness, no outline will be drawn.

        @see setPath, setSolidFill
    */
    void setOutline (const PathStrokeType& strokeType,
                     const Brush& strokeBrush);

    /** Returns the current outline style. */
    const PathStrokeType& getOutlineStroke() const throw()      { return strokeType; }

    /** Returns the brush currently being used to draw the outline. */
    Brush* getOutlineBrush() const throw()                      { return strokeBrush; }


    //==============================================================================
    /** @internal */
    void draw (Graphics& g, const AffineTransform& transform) const;
    /** @internal */
    void getBounds (float& x, float& y, float& width, float& height) const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Path path, outline;
    Brush* fillBrush;
    Brush* strokeBrush;
    PathStrokeType strokeType;

    void updateOutline();

    DrawablePath (const DrawablePath&);
    const DrawablePath& operator= (const DrawablePath&);
};


#endif   // __JUCE_DRAWABLEPATH_JUCEHEADER__
