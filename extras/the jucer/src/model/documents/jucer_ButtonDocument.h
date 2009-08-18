/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_BUTTONDOCUMENT_JUCEHEADER__
#define __JUCER_BUTTONDOCUMENT_JUCEHEADER__

#include "../jucer_JucerDocument.h"


//==============================================================================
/**
*/
class ButtonDocument   : public JucerDocument
{
public:
    //==============================================================================
    ButtonDocument();
    ~ButtonDocument();

    //==============================================================================
    const String getTypeName() const;

    JucerDocument* createCopy();
    Component* createTestComponent (const bool alwaysFillBackground);

    int getNumPaintRoutines() const;
    const StringArray getPaintRoutineNames() const;
    PaintRoutine* getPaintRoutine (const int index) const;

    void setStatePaintRoutineEnabled (const int index, bool b);
    bool isStatePaintRoutineEnabled (const int index) const;

    int chooseBestEnabledPaintRoutine (int paintRoutineWanted) const;

    ComponentLayout* getComponentLayout() const                 { return 0; }

    void addExtraClassProperties (PropertyPanel* panel);

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
    PaintRoutine* paintRoutines[7];
    bool paintStatesEnabled [7];

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
};


#endif   // __JUCER_BUTTONDOCUMENT_JUCEHEADER__
