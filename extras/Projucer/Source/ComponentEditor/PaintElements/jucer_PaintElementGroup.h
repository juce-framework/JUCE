/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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

    void ungroup (const bool);

    static void groupSelected (PaintRoutine* const);

    int getNumElements() const noexcept;

    PaintElement* getElement (const int index) const noexcept;
    int indexOfElement (const PaintElement* element) const noexcept;

    bool containsElement (const PaintElement* element) const;

    //==============================================================================
    void setInitialBounds (int, int) override;
    Rectangle<int> getCurrentBounds (const Rectangle<int>&) const override;
    void setCurrentBounds (const Rectangle<int>&, const Rectangle<int>&, const bool) override;

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

        void buttonClicked();
        String getButtonText() const;

        PaintElementGroup* element;
    };
};
