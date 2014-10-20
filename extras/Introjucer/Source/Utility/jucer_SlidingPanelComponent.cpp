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


#include "jucer_SlidingPanelComponent.h"

//==============================================================================
// TEMPORARY COPY OF TAB HELPERS FOR SLIDER PROTOTYPE, REMOVE BEFORE RELEASE ///
//==============================================================================

namespace TabbedComponentHelpers
{
    const Identifier deleteComponentId ("deleteByTabComp_");

    static void deleteIfNecessary (Component* const comp)
    {
        if (comp != nullptr && (bool) comp->getProperties() [deleteComponentId])
            delete comp;
    }
}


//==============================================================================
SlidingPanelComponent::SlidingPanelComponent()
    : currentTabIndex (0), dotSize (20)
{
    addAndMakeVisible (slide);
}

SlidingPanelComponent::~SlidingPanelComponent()
{
    for (int i = contentComponents.size(); --i >= 0;)
        if (Component* c = contentComponents.getReference(i))
            TabbedComponentHelpers::deleteIfNecessary (c);
}

void SlidingPanelComponent::addTab (const String& tabName,
                                    Component* const contentComponent,
                                    const bool deleteComponentWhenNotNeeded,
                                    const int insertIndex)
{
    contentComponents.insert (insertIndex, WeakReference<Component> (contentComponent));
    tabNames.insert (insertIndex, tabName);

    if (deleteComponentWhenNotNeeded && contentComponent != nullptr)
        contentComponent->getProperties().set (TabbedComponentHelpers::deleteComponentId, true);

    slide.addAndMakeVisible (contentComponent);

    resized();
}

void SlidingPanelComponent::goToTab (int targetTabIndex)
{
    const int xTranslation = (currentTabIndex - targetTabIndex) * getWidth();

    currentTabIndex = targetTabIndex;

    Desktop::getInstance().getAnimator()
        .animateComponent (&slide, slide.getBounds().translated (xTranslation, 0),
                           1.0f, 600, false, 0.0, 0.0);

    repaint();
}

void SlidingPanelComponent::paint (Graphics& g)
{
    Rectangle<int> dotHolder = getLocalBounds();

    dotHolder.reduce ((getWidth() - dotSize * getNumTabs()) / 2, 20);
    dotHolder = dotHolder.removeFromBottom (dotSize);

    g.setColour (Colours::white);

    for (int i = 0; i < getNumTabs(); ++i)
    {
        const Rectangle<float> r (dotHolder.removeFromLeft (dotSize).reduced (5, 5).toFloat());

        if (i == currentTabIndex)
            g.fillEllipse (r);
        else
            g.drawEllipse (r, 1.0f);
    }
}

void SlidingPanelComponent::resized()
{
    slide.setBounds (-currentTabIndex * getWidth(), slide.getPosition().y,
                     getNumTabs() * getWidth(), getHeight());

    Rectangle<int> content (getLocalBounds());

    content.removeFromBottom (20 + 2 * dotSize);

    for (int i = contentComponents.size(); --i >= 0;)
        if (Component* c = contentComponents.getReference(i))
            c->setBounds (content.translated (i * content.getWidth(), 0));
}
