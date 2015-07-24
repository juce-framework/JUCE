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

#ifndef JUCER_COMPONENTDOCUMENT_H_INCLUDED
#define JUCER_COMPONENTDOCUMENT_H_INCLUDED

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

    ComponentLayout* getComponentLayout() const                 { return components; }

    //==============================================================================
    XmlElement* createXml() const;
    bool loadFromXml (const XmlElement& xml);

    void fillInGeneratedCode (GeneratedCode& code) const;

private:
    ScopedPointer<ComponentLayout> components;
    ScopedPointer<PaintRoutine> backgroundGraphics;
};


#endif   // JUCER_COMPONENTDOCUMENT_H_INCLUDED
