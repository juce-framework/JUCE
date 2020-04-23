/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

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

    ~EditingPanelBase() override;

    //==============================================================================
    void resized() override;
    void paint (Graphics& g) override;
    void visibilityChanged() override;

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

    Viewport* viewport;
    MagnifierComponent* magnifier;
    Component* editor;
    Component* propsPanel;
};
