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

        @see setFillColour, setStrokeType
    */
    void setPath (const Path& newPath) throw();

    /** Returns the current path. */
    const Path& getPath() const throw()                         { return path; }

    /** Sets a fill type for the path.

        This colour is used to fill the path - if you don't want the path to be
        filled (e.g. if you're just drawing an outline), set this to a transparent
        colour.

        @see setPath, setStrokeFill
    */
    void setFill (const FillType& newFill) throw();

    /** Returns the current fill type.
        @see setFill
    */
    const FillType& getFill() const throw()                     { return mainFill; }

    /** Sets the fill type with which the outline will be drawn.
        @see setFill
    */
    void setStrokeFill (const FillType& newStrokeFill) throw();

    /** Returns the current stroke fill.
        @see setStrokeFill
    */
    const FillType& getStrokeFill() const throw()               { return strokeFill; }

    /** Changes the properties of the outline that will be drawn around the path.
        If the stroke has 0 thickness, no stroke will be drawn.
        @see setStrokeThickness, setStrokeColour
    */
    void setStrokeType (const PathStrokeType& newStrokeType) throw();

    /** Changes the stroke thickness.
        This is a shortcut for calling setStrokeType.
    */
    void setStrokeThickness (const float newThickness) throw();

    /** Returns the current outline style. */
    const PathStrokeType& getStrokeType() const throw()         { return strokeType; }


    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    void getBounds (float& x, float& y, float& width, float& height) const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    ValueTree createValueTree() const throw();
    /** @internal */
    static DrawablePath* createFromValueTree (const ValueTree& tree) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Path path, stroke;
    FillType mainFill, strokeFill;
    PathStrokeType strokeType;

    void updateOutline();

    DrawablePath (const DrawablePath&);
    const DrawablePath& operator= (const DrawablePath&);
};


#endif   // __JUCE_DRAWABLEPATH_JUCEHEADER__
