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

#include "jucer_PaintElement.h"
#include "../jucer_ObjectTypes.h"

//==============================================================================
class PaintElementGroup   : public PaintElement
{
public:
    PaintElementGroup (PaintRoutine*);
    ~PaintElementGroup() override;

    void ungroup (bool);

    static void groupSelected (PaintRoutine* const);

    int getNumElements() const noexcept;

    PaintElement* getElement (int index) const noexcept;
    int indexOfElement (const PaintElement* element) const noexcept;

    bool containsElement (const PaintElement* element) const;

    //==============================================================================
    void setInitialBounds (int, int) override;
    Rectangle<int> getCurrentBounds (const Rectangle<int>&) const override;
    void setCurrentBounds (const Rectangle<int>&, const Rectangle<int>&, bool) override;

    //==============================================================================
    void draw (Graphics&, const ComponentLayout*, const Rectangle<int>&) override;

    void getEditableProperties (Array<PropertyComponent*>&, bool) override;

    void fillInGeneratedCode (GeneratedCode&, String&) override;

    static const char* getTagName() noexcept;

    XmlElement* createXml() const override;

    bool loadFromXml (const XmlElement&) override;

    void applyCustomPaintSnippets (StringArray&) override;

private:
    OwnedArray<PaintElement> subElements;

    struct UngroupProperty   : public ButtonPropertyComponent
    {
        UngroupProperty (PaintElementGroup* const);

        void buttonClicked() override;
        String getButtonText() const override;

        PaintElementGroup* element;
    };
};
