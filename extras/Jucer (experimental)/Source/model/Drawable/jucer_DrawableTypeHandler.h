/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_DRAWABLETYPEHANDLER_H_7FB02E2F__
#define __JUCER_DRAWABLETYPEHANDLER_H_7FB02E2F__

#include "jucer_DrawableDocument.h"


//==============================================================================
class DrawableTypeHandler
{
public:
    DrawableTypeHandler (const String& displayName_, const Identifier& valueTreeType_)
        : displayName (displayName_), valueTreeType (valueTreeType_)
    {
    }

    virtual ~DrawableTypeHandler() {}

    virtual const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition) = 0;

    const String& getDisplayName() const        { return displayName; }
    const Identifier& getValueTreeType() const  { return valueTreeType; }

private:
    const String displayName;
    const Identifier valueTreeType;
};

//==============================================================================
class DrawablePathHandler : public DrawableTypeHandler
{
public:
    DrawablePathHandler()  : DrawableTypeHandler ("Polygon", DrawablePath::valueTreeType) {}
    ~DrawablePathHandler() {}

    const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition)
    {
        Path p;
        p.addTriangle (approxPosition.getX(), approxPosition.getY() - 50.0f,
                       approxPosition.getX() + 50.0f, approxPosition.getY() + 20.0f,
                       approxPosition.getX() - 50.0f, approxPosition.getY() + 20.0f);

        DrawablePath dp;
        dp.setPath (p);
        dp.setFill (Colours::lightblue.withHue (Random::getSystemRandom().nextFloat()));
        return dp.createValueTree (0);
    }
};

//==============================================================================
class DrawableImageHandler : public DrawableTypeHandler
{
public:
    DrawableImageHandler()  : DrawableTypeHandler ("Image", DrawableImage::valueTreeType) {}
    ~DrawableImageHandler() {}

    const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition)
    {
        Image tempImage (Image::ARGB, 100, 100, true);

        {
            Graphics g (tempImage);
            g.fillAll (Colours::grey.withAlpha (0.3f));
            g.setColour (Colours::red);
            g.setFont (40.0f);
            g.drawText ("?", 0, 0, 100, 100, Justification::centred, false);
        }

        DrawableImage di;
        di.setTransform (RelativePoint (approxPosition),
                         RelativePoint (approxPosition + Point<float> (100.0f, 0.0f)),
                         RelativePoint (approxPosition + Point<float> (0.0f, 100.0f)));
        return di.createValueTree (&document);
    }
};



#endif  // __JUCER_DRAWABLETYPEHANDLER_H_7FB02E2F__
