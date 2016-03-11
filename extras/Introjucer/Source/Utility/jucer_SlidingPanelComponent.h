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

#ifndef JUCER_SLIDINGPANELCOMPONENT_H_INCLUDED
#define JUCER_SLIDINGPANELCOMPONENT_H_INCLUDED

#include "../Application/jucer_Application.h"


//==============================================================================
class SlidingPanelComponent   : public Component
{
public:
    SlidingPanelComponent();
    ~SlidingPanelComponent();

    /** Adds a new tab to the panel slider. */
    void addTab (const String& tabName,
                 Component* contentComponent,
                 bool deleteComponentWhenNotNeeded,
                 int insertIndex = -1);

    /** Gets rid of one of the tabs. */
    void removeTab (int tabIndex);

    /** Gets index of current tab. */
    int getCurrentTabIndex() const noexcept         { return currentIndex; }

    /** Returns the number of tabs. */
    int getNumTabs() const noexcept                 { return pages.size(); }

    /** Animates the window to the desired tab. */
    void goToTab (int targetTabIndex);


    //==============================================================================
    /** @internal */
    void resized() override;

private:
    struct DotButton;
    friend struct DotButton;

    struct PageInfo
    {
        ~PageInfo();

        Component::SafePointer<Component> content;
        ScopedPointer<DotButton> dotButton;
        String name;
        bool shouldDelete;
    };

    OwnedArray<PageInfo> pages;

    Component pageHolder;
    int currentIndex, dotSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidingPanelComponent)
};


#endif   // JUCER_SLIDINGPANELCOMPONENT_H_INCLUDED
