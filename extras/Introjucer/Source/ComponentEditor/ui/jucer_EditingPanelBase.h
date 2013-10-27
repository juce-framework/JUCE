/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef __JUCER_EDITINGPANELBASE_JUCEHEADER__
#define __JUCER_EDITINGPANELBASE_JUCEHEADER__

#include "../jucer_JucerDocument.h"
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

    virtual Rectangle<int> getComponentArea() const = 0;

    double getZoom() const;
    void setZoom (double newScale);
    void setZoom (double newScale, int anchorX, int anchorY);

    // convert a pos relative to this component into a pos on the editor
    void xyToTargetXY (int& x, int& y) const;

    void dragKeyHeldDown (bool isKeyDown);

    class MagnifierComponent;

protected:
    JucerDocument& document;
    LookAndFeel_V2 lookAndFeel;

    Viewport* viewport;
    MagnifierComponent* magnifier;
    Component* editor;
    Component* propsPanel;
};


#endif   // __JUCER_EDITINGPANELBASE_JUCEHEADER__
