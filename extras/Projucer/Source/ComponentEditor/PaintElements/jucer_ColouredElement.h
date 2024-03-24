/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

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
                     bool showOutline_,
                     bool showJointAndEnd_);

    ~ColouredElement() override;

    //==============================================================================
    void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected) override;
    void getColourSpecificProperties (Array<PropertyComponent*>& props);

    //==============================================================================
    const JucerFillType& getFillType() noexcept;
    void setFillType (const JucerFillType& newType, bool undoable);

    bool isStrokeEnabled() const noexcept;
    void enableStroke (bool enable, bool undoable);

    const StrokeType& getStrokeType() noexcept;
    void setStrokeType (const PathStrokeType& newType, bool undoable);
    void setStrokeFill (const JucerFillType& newType, bool undoable);

    //==============================================================================
    Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const override;
    void setCurrentBounds (const Rectangle<int>& newBounds, const Rectangle<int>& parentArea, bool undoable) override;

    void createSiblingComponents() override;

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
