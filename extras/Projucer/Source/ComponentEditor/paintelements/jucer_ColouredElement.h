/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCER_COLOUREDELEMENT_H_INCLUDED
#define JUCER_COLOUREDELEMENT_H_INCLUDED

#include "../jucer_PaintRoutine.h"
#include "../jucer_JucerDocument.h"
#include "jucer_StrokeType.h"


//==============================================================================
/**
    Base class for paint elements that have a fill colour and stroke.

*/
class ColouredElement   : public PaintElement
{
public:
    ColouredElement (PaintRoutine* owner,
                     const String& name,
                     const bool showOutline_,
                     const bool showJointAndEnd_);

    ~ColouredElement();

    //==============================================================================
    void getEditableProperties (Array<PropertyComponent*>& props);
    void getColourSpecificProperties (Array<PropertyComponent*>& props);

    //==============================================================================
    const JucerFillType& getFillType() noexcept;
    void setFillType (const JucerFillType& newType, const bool undoable);

    bool isStrokeEnabled() const noexcept;
    void enableStroke (bool enable, const bool undoable);

    const StrokeType& getStrokeType() noexcept;
    void setStrokeType (const PathStrokeType& newType, const bool undoable);
    void setStrokeFill (const JucerFillType& newType, const bool undoable);

    //==============================================================================
    Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const;
    void setCurrentBounds (const Rectangle<int>& newBounds, const Rectangle<int>& parentArea, const bool undoable);

    void createSiblingComponents();

    //==============================================================================
    void addColourAttributes (XmlElement* const e) const;
    bool loadColourAttributes (const XmlElement& xml);

protected:
    JucerFillType fillType;

    bool isStrokePresent;
    const bool showOutline, showJointAndEnd;
    StrokeType strokeType;

    void convertToNewPathElement (const Path& path);
};


#endif   // JUCER_COLOUREDELEMENT_H_INCLUDED
