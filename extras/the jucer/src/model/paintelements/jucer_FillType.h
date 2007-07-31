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

#ifndef __JUCER_FILLTYPE_JUCEHEADER__
#define __JUCER_FILLTYPE_JUCEHEADER__

#include "../jucer_JucerDocument.h"


//==============================================================================
/**
    Defines a brush to be used to fill a shape.
*/
class FillType
{
public:
    //==============================================================================
    FillType();
    FillType (const FillType& other);
    const FillType& operator= (const FillType& other);
    ~FillType();

    bool operator== (const FillType& other) const throw();
    bool operator!= (const FillType& other) const throw();

    //==============================================================================
    Brush* createBrush (JucerDocument* const document, const Rectangle& parentArea);

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
