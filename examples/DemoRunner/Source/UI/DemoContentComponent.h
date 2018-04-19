/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "../Demos/JUCEDemos.h"

struct DemoContent;
struct CodeContent;

//==============================================================================
class DemoContentComponent  : public TabbedComponent
{
public:
    DemoContentComponent (Component& mainComponent, std::function<void(bool)> demoChangedCallback);
    ~DemoContentComponent();

    void resized() override;

    void setDemo (const String& category, int selectedDemoIndex);
    void clearCurrentDemo();
    int getCurrentDemoIndex() const noexcept      { return currentDemoIndex; }

    bool isShowingHomeScreen() const noexcept;
    void showHomeScreen();

    void setTabBarIndent (int indent) noexcept    { tabBarIndent = indent; }

private:
    std::function<void(bool)> demoChangedCallback;

    std::unique_ptr<DemoContent> demoContent;

   #if ! (JUCE_ANDROID || JUCE_IOS)
    std::unique_ptr<CodeContent> codeContent;
   #endif

    String currentDemoCategory;
    int currentDemoIndex = -1;
    int tabBarIndent = 0;

    //==============================================================================
    void lookAndFeelChanged() override;

    String trimPIP (const String& fileContents);
    void ensureDemoIsShowing();
};
