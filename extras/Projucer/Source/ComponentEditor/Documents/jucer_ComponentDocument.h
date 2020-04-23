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

#include "../jucer_JucerDocument.h"

//==============================================================================
class ComponentDocument   : public JucerDocument
{
public:
    ComponentDocument (SourceCodeDocument* cpp);
    ~ComponentDocument();

    //==============================================================================
    String getTypeName() const;

    JucerDocument* createCopy();
    Component* createTestComponent (const bool alwaysFillBackground);

    int getNumPaintRoutines() const                             { return 1; }
    StringArray getPaintRoutineNames() const                    { return StringArray ("Graphics"); }
    PaintRoutine* getPaintRoutine (const int index) const       { return index == 0 ? backgroundGraphics.get() : nullptr; }

    ComponentLayout* getComponentLayout() const                 { return components.get(); }

    //==============================================================================
    std::unique_ptr<XmlElement> createXml() const;
    bool loadFromXml (const XmlElement& xml);

    void fillInGeneratedCode (GeneratedCode& code) const;
    void applyCustomPaintSnippets (StringArray&);

private:
    std::unique_ptr<ComponentLayout> components;
    std::unique_ptr<PaintRoutine> backgroundGraphics;
};
