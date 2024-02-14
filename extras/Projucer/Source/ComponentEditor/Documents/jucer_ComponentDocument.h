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

#include "../jucer_JucerDocument.h"

//==============================================================================
class ComponentDocument   : public JucerDocument
{
public:
    ComponentDocument (SourceCodeDocument* cpp);
    ~ComponentDocument() override;

    //==============================================================================
    String getTypeName() const override;

    JucerDocument* createCopy() override;
    Component* createTestComponent (bool alwaysFillBackground) override;

    int getNumPaintRoutines() const override                             { return 1; }
    StringArray getPaintRoutineNames() const override                    { return StringArray ("Graphics"); }
    PaintRoutine* getPaintRoutine (int index) const override             { return index == 0 ? backgroundGraphics.get() : nullptr; }

    ComponentLayout* getComponentLayout() const override                 { return components.get(); }

    //==============================================================================
    std::unique_ptr<XmlElement> createXml() const override;
    bool loadFromXml (const XmlElement& xml) override;

    void fillInGeneratedCode (GeneratedCode& code) const override;
    void applyCustomPaintSnippets (StringArray&) override;

private:
    std::unique_ptr<ComponentLayout> components;
    std::unique_ptr<PaintRoutine> backgroundGraphics;
};
