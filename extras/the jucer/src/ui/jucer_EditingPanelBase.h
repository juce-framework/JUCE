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

#ifndef __JUCER_EDITINGPANELBASE_JUCEHEADER__
#define __JUCER_EDITINGPANELBASE_JUCEHEADER__

#include "../model/jucer_JucerDocument.h"
#include "jucer_ComponentLayoutEditor.h"
class LayoutPropsPanel;


//==============================================================================
/**
    Base class for the layout and graphics panels - this takes care of arranging
    the properties panel and managing the viewport for the content.

*/
class EditingPanelBase  : public Component
{
public:
    //==============================================================================
    EditingPanelBase (JucerDocument& document,
                      Component* propsPanel,
                      Component* editorComp);

    ~EditingPanelBase();

    //==============================================================================
    void resized();
    void visibilityChanged();

    virtual void updatePropertiesList() = 0;

    virtual const Rectangle getComponentArea() const = 0;

    double getZoom() const;
    void setZoom (double newScale);
    void setZoom (double newScale, int anchorX, int anchorY);

    // convert a pos relative to this component into a pos on the editor
    void xyToTargetXY (int& x, int& y) const;

    void dragKeyHeldDown (bool isKeyDown);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    JucerDocument& document;

    Viewport* viewport;
    MagnifierComponent* magnifier;
    Component* editor;
    Component* propsPanel;
};


#endif   // __JUCER_EDITINGPANELBASE_JUCEHEADER__
