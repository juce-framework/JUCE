/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
