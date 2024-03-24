/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ComponentTransformsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Applies transformations to components.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        ComponentTransformsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "WidgetsDemo.h"

//==============================================================================
class ComponentTransformsDemo final : public Component
{
public:
    ComponentTransformsDemo()
    {
        content.reset (new WidgetsDemo (true));
        addAndMakeVisible (content.get());
        content->setSize (750, 500);

        for (int i = 0; i < 3; ++i)
        {
            auto* d = new CornerDragger();
            addAndMakeVisible (draggers.add (d));
        }

        draggers.getUnchecked (0)->relativePos = { 0.10f, 0.15f };
        draggers.getUnchecked (1)->relativePos = { 0.95f, 0.05f };
        draggers.getUnchecked (2)->relativePos = { 0.05f, 0.85f };

        setSize (800, 600);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));

        g.setColour (Colours::white);
        g.setFont (15.0f);
        g.drawFittedText ("Drag the corner-points around to show how complex components can have affine-transforms applied...",
                          getLocalBounds().removeFromBottom (40).reduced (10, 0), Justification::centred, 3);
    }

    void resized() override
    {
        for (auto* d : draggers)
            d->setCentrePosition (proportionOfWidth  (d->relativePos.x),
                                  proportionOfHeight (d->relativePos.y));
    }

    void childBoundsChanged (Component* child) override
    {
        if (dynamic_cast<CornerDragger*> (child) != nullptr)
            updateTransform();
    }

private:
    std::unique_ptr<Component> content;

    struct CornerDragger final : public Component
    {
        CornerDragger()
        {
            setSize (30, 30);
            setRepaintsOnMouseActivity (true);
        }

        void paint (Graphics& g) override
        {
            g.setColour (Colours::white.withAlpha (isMouseOverOrDragging() ? 0.9f : 0.5f));
            g.fillEllipse (getLocalBounds().reduced (3).toFloat());

            g.setColour (Colours::darkgreen);
            g.drawEllipse (getLocalBounds().reduced (3).toFloat(), 2.0f);
        }

        void resized() override
        {
            constrainer.setMinimumOnscreenAmounts (getHeight(), getWidth(), getHeight(), getWidth());
        }

        void moved() override
        {
            if (isMouseButtonDown())
                relativePos = getBounds().getCentre().toFloat() / Point<int> (getParentWidth(), getParentHeight()).toFloat();
        }

        void mouseDown (const MouseEvent& e) override   { dragger.startDraggingComponent (this, e); }
        void mouseDrag (const MouseEvent& e) override   { dragger.dragComponent (this, e, &constrainer); }

        Point<float> relativePos;

    private:
        ComponentBoundsConstrainer constrainer;
        ComponentDragger dragger;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CornerDragger)
    };

    OwnedArray<CornerDragger> draggers;

    Point<float> getDraggerPos (int index) const
    {
        return draggers.getUnchecked (index)->getBounds().getCentre().toFloat();
    }

    void updateTransform()
    {
        auto p0 = getDraggerPos (0);
        auto p1 = getDraggerPos (1);
        auto p2 = getDraggerPos (2);

        if (p0 != p1 && p1 != p2 && p0 != p2)
            content->setTransform (AffineTransform::fromTargetPoints (0, 0, p0.x, p0.y,
                                                                      (float) content->getWidth(), 0,  p1.x, p1.y,
                                                                      0, (float) content->getHeight(), p2.x, p2.y));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentTransformsDemo)
};
