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

#ifndef __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
#define __JUCER_COMPONENTDOCUMENT_JUCEHEADER__

#include "../jucer_JucerDocument.h"


//==============================================================================
/**
*/
class ComponentDocument   : public JucerDocument
{
public:
    //==============================================================================
    ComponentDocument();
    ~ComponentDocument();

    //==============================================================================
    const String getTypeName() const;

    JucerDocument* createCopy();
    Component* createTestComponent (const bool alwaysFillBackground);

    int getNumPaintRoutines() const                             { return 1; }
    const StringArray getPaintRoutineNames() const              { StringArray s; s.add (T("Graphics")); return s; }
    PaintRoutine* getPaintRoutine (const int index) const       { return index == 0 ? backgroundGraphics : 0; }

    ComponentLayout* getComponentLayout() const                 { return components; }

    //==============================================================================
    XmlElement* createXml() const;
    bool loadFromXml (const XmlElement& xml);

    void fillInGeneratedCode (GeneratedCode& code) const;

    //==============================================================================
    juce_UseDebuggingNewOperator


private:
    //==============================================================================
    ComponentLayout* components;
    PaintRoutine* backgroundGraphics;
};


#endif   // __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
