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

#ifndef __JUCER_COMPONENTLAYOUTPANEL_JUCEHEADER__
#define __JUCER_COMPONENTLAYOUTPANEL_JUCEHEADER__

#include "jucer_ComponentLayoutEditor.h"
#include "jucer_EditingPanelBase.h"


//==============================================================================
/**
*/
class ComponentLayoutPanel  : public EditingPanelBase
{
public:
    //==============================================================================
    ComponentLayoutPanel (JucerDocument& document, ComponentLayout& layout);
    ~ComponentLayoutPanel();

    ComponentLayout& getLayout() const throw()          { return layout;}

    void updatePropertiesList();
    const Rectangle getComponentArea() const;

    Image* createComponentSnapshot() const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ComponentLayout& layout;
};


#endif   // __JUCER_COMPONENTLAYOUTPANEL_JUCEHEADER__
