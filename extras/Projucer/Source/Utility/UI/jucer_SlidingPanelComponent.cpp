/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_SlidingPanelComponent.h"

//==============================================================================
struct SlidingPanelComponent::DotButton final : public Button
{
    DotButton (SlidingPanelComponent& sp, int pageIndex)
        : Button (String()), owner (sp), index (pageIndex) {}

    void paintButton (Graphics& g, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        g.setColour (findColour (defaultButtonBackgroundColourId));
        const auto r = getLocalBounds().reduced (getWidth() / 4).toFloat();

        if (index == owner.getCurrentTabIndex())
            g.fillEllipse (r);
        else
            g.drawEllipse (r, 1.0f);
    }

    void clicked() override
    {
        owner.goToTab (index);
    }

    using Button::clicked;

    SlidingPanelComponent& owner;
    int index;
};

//==============================================================================
SlidingPanelComponent::SlidingPanelComponent()
    : currentIndex (0), dotSize (20)
{
    addAndMakeVisible (pageHolder);
}

SlidingPanelComponent::~SlidingPanelComponent()
{
}

SlidingPanelComponent::PageInfo::~PageInfo()
{
    if (shouldDelete)
        content.deleteAndZero();
}

void SlidingPanelComponent::addTab (const String& tabName,
                                    Component* const contentComponent,
                                    const bool deleteComponentWhenNotNeeded,
                                    const int insertIndex)
{
    PageInfo* page = new PageInfo();
    pages.insert (insertIndex, page);
    page->content = contentComponent;
    page->dotButton.reset (new DotButton (*this, pages.indexOf (page)));
    addAndMakeVisible (page->dotButton.get());
    page->name = tabName;
    page->shouldDelete = deleteComponentWhenNotNeeded;

    pageHolder.addAndMakeVisible (contentComponent);
    resized();
}

void SlidingPanelComponent::goToTab (int targetTabIndex)
{
    currentIndex = targetTabIndex;

    Desktop::getInstance().getAnimator()
        .animateComponent (&pageHolder, pageHolder.getBounds().withX (-targetTabIndex * getWidth()),
                           1.0f, 600, false, 0.0, 0.0);

    repaint();
}

void SlidingPanelComponent::resized()
{
    pageHolder.setBounds (-currentIndex * getWidth(), pageHolder.getPosition().y,
                          getNumTabs() * getWidth(), getHeight());

    Rectangle<int> content (getLocalBounds());

    Rectangle<int> dotHolder = content.removeFromBottom (20 + dotSize)
                                 .reduced ((content.getWidth() - dotSize * getNumTabs()) / 2, 10);

    for (int i = 0; i < getNumTabs(); ++i)
        pages.getUnchecked (i)->dotButton->setBounds (dotHolder.removeFromLeft (dotSize));

    for (int i = pages.size(); --i >= 0;)
        if (Component* c = pages.getUnchecked (i)->content)
            c->setBounds (content.translated (i * content.getWidth(), 0));
}
