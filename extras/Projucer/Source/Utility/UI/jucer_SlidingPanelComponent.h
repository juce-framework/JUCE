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

#pragma once

#include "../../Application/jucer_Application.h"

//==============================================================================
class SlidingPanelComponent final : public Component
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
