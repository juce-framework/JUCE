/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Application/jucer_Application.h"

//==============================================================================
class SlidingPanelComponent   : public Component
{
public:
    SlidingPanelComponent();
    ~SlidingPanelComponent() override;

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
        std::unique_ptr<DotButton> dotButton;
        String name;
        bool shouldDelete;
    };

    OwnedArray<PageInfo> pages;

    Component pageHolder;
    int currentIndex, dotSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlidingPanelComponent)
};
