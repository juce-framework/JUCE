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

#ifndef __JUCER_FILLTYPE_JUCEHEADER__
#define __JUCER_FILLTYPE_JUCEHEADER__

#include "../jucer_JucerDocument.h"


//==============================================================================
/**
    Defines a brush to be used to fill a shape.
*/
class JucerFillType
{
public:
    //==============================================================================
    JucerFillType();
    JucerFillType (const JucerFillType& other);
    const JucerFillType& operator= (const JucerFillType& other);
    ~JucerFillType();

    bool operator== (const JucerFillType& other) const throw();
    bool operator!= (const JucerFillType& other) const throw();

    //==============================================================================
    void setFillType (Graphics& g, JucerDocument* const document, const Rectangle& parentArea);

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) const;

    const String toString() const;
    void restoreFromString (const String& s);

    bool isOpaque() const;
    bool isInvisible() const;

    //==============================================================================
    enum FillMode
    {
        solidColour,
        linearGradient,
        radialGradient,
        imageBrush
    };

    FillMode mode;

    Colour colour, gradCol1, gradCol2;

    // just the x, y, of these are used
    RelativePositionedRectangle gradPos1, gradPos2;

    String imageResourceName;
    double imageOpacity;
    RelativePositionedRectangle imageAnchor;

    //==============================================================================
private:
    Image* image;

    void reset();
    void loadImage (JucerDocument* const document);
};


#endif   // __JUCER_FILLTYPE_JUCEHEADER__
