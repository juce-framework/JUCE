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

#include "DemoContentComponent.h"
#include "SettingsContent.h"
#include "MainComponent.h"

//==============================================================================
struct DemoContent    : public Component
{
    DemoContent() noexcept    {}

    void resized() override
    {
        if (comp != nullptr)
            comp->setBounds (getLocalBounds());
    }

    void setComponent (Component* newComponent)
    {
        comp.reset (newComponent);

        if (comp != nullptr)
        {
            addAndMakeVisible (comp.get());
            resized();
        }
    }

    Component* getComponent() const noexcept    { return comp.get(); }
    void showHomeScreen()                       { setComponent (createIntroDemo()); }

private:
    std::unique_ptr<Component> comp;
};

//==============================================================================
#if ! (JUCE_ANDROID || JUCE_IOS)
struct CodeContent    : public Component
{
    CodeContent()
    {
        addAndMakeVisible (codeEditor);

        codeEditor.setReadOnly (true);
        codeEditor.setScrollbarThickness (8);

        lookAndFeelChanged();
    }

    void resized() override
    {
        codeEditor.setBounds (getLocalBounds());
    }

    void setDefaultCodeContent()
    {
        document.replaceAllContent ("\n/*******************************************************************************\n"
                                    "          Select one of the demos from the side panel on the left to see\n"
                                    "            its code here and an instance running in the \"Demo\" tab!\n"
                                    "*******************************************************************************/\n");
    }

    void lookAndFeelChanged() override
    {
        auto* v4 = dynamic_cast <LookAndFeel_V4*> (&Desktop::getInstance().getDefaultLookAndFeel());

        if (v4 != nullptr && (v4->getCurrentColourScheme() != LookAndFeel_V4::getLightColourScheme()))
            codeEditor.setColourScheme (getDarkColourScheme());
        else
            codeEditor.setColourScheme (getLightColourScheme());
    }

    CodeDocument document;
    CPlusPlusCodeTokeniser cppTokensier;
    CodeEditorComponent codeEditor  { document, &cppTokensier };
};
#endif

//==============================================================================
DemoContentComponent::DemoContentComponent (Component& mainComponent, std::function<void(bool)> callback)
    : TabbedComponent (TabbedButtonBar::Orientation::TabsAtTop),
      demoChangedCallback (callback)
{
    demoContent.reset (new DemoContent());
    addTab ("Demo",     Colours::transparentBlack, demoContent.get(), false);

   #if ! (JUCE_ANDROID || JUCE_IOS)
    codeContent.reset (new CodeContent());
    addTab ("Code",     Colours::transparentBlack, codeContent.get(), false);
   #endif

    addTab ("Settings", Colours::transparentBlack, new SettingsContent (dynamic_cast<MainComponent&> (mainComponent)), true);

    setTabBarDepth (40);
    lookAndFeelChanged();
}

DemoContentComponent::~DemoContentComponent()
{
}

void DemoContentComponent::resized()
{
    TabbedComponent::resized();

    if (tabBarIndent > 0)
        getTabbedButtonBar().setBounds (getTabbedButtonBar().getBounds().withTrimmedLeft (tabBarIndent));
}

void DemoContentComponent::setDemo (const String& category, int selectedDemoIndex)
{
    if ((currentDemoCategory == category)
        && (currentDemoIndex == selectedDemoIndex))
        return;

    auto demo = JUCEDemos::getCategory (category).demos[(size_t) selectedDemoIndex];

   #if ! (JUCE_ANDROID || JUCE_IOS)
    codeContent->document.replaceAllContent (trimPIP (demo.demoFile.loadFileAsString()));
    codeContent->codeEditor.scrollToLine (0);
   #endif

    auto* content = demo.callback();
    demoContent->setComponent (content);
    demoChangedCallback (demo.isHeavyweight);

    ensureDemoIsShowing();

    currentDemoCategory = category;
    currentDemoIndex = selectedDemoIndex;
}

bool DemoContentComponent::isShowingHomeScreen() const noexcept
{
    return isComponentIntroDemo (demoContent->getComponent()) && getCurrentTabIndex() == 0;
}

void DemoContentComponent::showHomeScreen()
{
    demoContent->showHomeScreen();

   #if ! (JUCE_ANDROID || JUCE_IOS)
    codeContent->setDefaultCodeContent();
   #endif

    demoChangedCallback (false);

    ensureDemoIsShowing();

    resized();

    currentDemoCategory = {};
    currentDemoIndex = -1;
}

void DemoContentComponent::clearCurrentDemo()
{
    demoContent->setComponent (nullptr);
    demoChangedCallback (false);
}

void DemoContentComponent::lookAndFeelChanged()
{
    auto backgroundColour = findColour (ResizableWindow::backgroundColourId);

    for (int i = 0; i < getNumTabs(); ++i)
        setTabBackgroundColour (i, backgroundColour);
}

String DemoContentComponent::trimPIP (const String& fileContents)
{
    auto lines = StringArray::fromLines (fileContents);

    auto metadataEndIndex = lines.indexOf (" END_JUCE_PIP_METADATA");

    if (metadataEndIndex == -1)
        return fileContents;

    lines.removeRange (0, metadataEndIndex + 3); // account for newline and comment block end

    return lines.joinIntoString ("\n");
}

void DemoContentComponent::ensureDemoIsShowing()
{
    if (getCurrentTabIndex() == (getNumTabs() - 1))
        setCurrentTabIndex (0);
}
