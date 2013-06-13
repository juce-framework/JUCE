/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    JucerFillType& operator= (const JucerFillType& other);
    ~JucerFillType();

    bool operator== (const JucerFillType& other) const throw();
    bool operator!= (const JucerFillType& other) const throw();

    //==============================================================================
    void setFillType (Graphics& g, JucerDocument* const document, const Rectangle<int>& parentArea);

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
    Image image;

    void reset();
    void loadImage (JucerDocument* const document);
};


#endif   // __JUCER_FILLTYPE_JUCEHEADER__
