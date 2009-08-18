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

#ifndef __JUCER_PAINTROUTINEPANEL_JUCEHEADER__
#define __JUCER_PAINTROUTINEPANEL_JUCEHEADER__

#include "jucer_PaintRoutineEditor.h"
#include "jucer_EditingPanelBase.h"


//==============================================================================
/**
*/
class PaintRoutinePanel  : public EditingPanelBase
{
public:
    //==============================================================================
    PaintRoutinePanel (JucerDocument& design,
                       PaintRoutine& routine,
                       JucerDocumentHolder* documentHolder);

    ~PaintRoutinePanel();

    PaintRoutine& getPaintRoutine() const throw()           { return routine; }

    void updatePropertiesList();
    const Rectangle getComponentArea() const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PaintRoutine& routine;
};


#endif   // __JUCER_PAINTROUTINEPANEL_JUCEHEADER__
