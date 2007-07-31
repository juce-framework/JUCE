/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
