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

#ifndef JUCER_BUTTONDOCUMENT_H_INCLUDED
#define JUCER_BUTTONDOCUMENT_H_INCLUDED

#include "../jucer_JucerDocument.h"


//==============================================================================
class ButtonDocument   : public JucerDocument
{
public:
    ButtonDocument (SourceCodeDocument* cpp);
    ~ButtonDocument();

    //==============================================================================
    String getTypeName() const;

    JucerDocument* createCopy();
    Component* createTestComponent (const bool alwaysFillBackground);

    int getNumPaintRoutines() const;
    StringArray getPaintRoutineNames() const;
    PaintRoutine* getPaintRoutine (const int index) const;

    void setStatePaintRoutineEnabled (const int index, bool b);
    bool isStatePaintRoutineEnabled (const int index) const;

    int chooseBestEnabledPaintRoutine (int paintRoutineWanted) const;

    ComponentLayout* getComponentLayout() const                 { return 0; }

    void addExtraClassProperties (PropertyPanel&);

    //==============================================================================
    XmlElement* createXml() const;
    bool loadFromXml (const XmlElement& xml);

    void fillInGeneratedCode (GeneratedCode& code) const;
    void fillInPaintCode (GeneratedCode& code) const;

    void getOptionalMethods (StringArray& baseClasses,
                             StringArray& returnValues,
                             StringArray& methods,
                             StringArray& initialContents) const;

    //==============================================================================
    ScopedPointer<PaintRoutine> paintRoutines[7];
    bool paintStatesEnabled [7];
};


#endif   // JUCER_BUTTONDOCUMENT_H_INCLUDED
