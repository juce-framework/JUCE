/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Typeface.h"
#include "juce_Font.h"
#include "../contexts/juce_EdgeTable.h"


//==============================================================================
Typeface::Typeface (const String& name_) noexcept
    : name (name_)
{
}

Typeface::~Typeface()
{
}

Typeface::Ptr Typeface::getFallbackTypeface()
{
    const Font fallbackFont (Font::getFallbackFontName(), 10, 0);
    return fallbackFont.getTypeface();
}

EdgeTable* Typeface::getEdgeTableForGlyph (int glyphNumber, const AffineTransform& transform)
{
    Path path;

    if (getOutlineForGlyph (glyphNumber, path) && ! path.isEmpty())
        return new EdgeTable (path.getBoundsTransformed (transform).getSmallestIntegerContainer().expanded (1, 0),
                              path, transform);

    return nullptr;
}



END_JUCE_NAMESPACE
