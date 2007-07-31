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
