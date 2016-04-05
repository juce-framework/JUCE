/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_SlidingPanelComponent.h"


struct SlidingPanelComponent::DotButton  : public Button
{
    DotButton (SlidingPanelComponent& sp, int pageIndex)
        : Button (String()), owner (sp), index (pageIndex) {}

    void paintButton (Graphics& g, bool /*isMouseOverButton*/, bool /*isButtonDown*/) override
    {
        g.setColour (Colours::white);
        const Rectangle<float> r (getLocalBounds().reduced (getWidth() / 4).toFloat());

        if (index == owner.getCurrentTabIndex())
            g.fillEllipse (r);
        else
            g.drawEllipse (r, 1.0f);
    }

    void clicked() override
    {
        owner.goToTab (index);
    }

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
    addAndMakeVisible (page->dotButton = new DotButton (*this, pages.indexOf (page)));
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
        pages.getUnchecked(i)->dotButton->setBounds (dotHolder.removeFromLeft (dotSize));

    for (int i = pages.size(); --i >= 0;)
        if (Component* c = pages.getUnchecked(i)->content)
            c->setBounds (content.translated (i * content.getWidth(), 0));
}
