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

#include "DemoContentComponent.h"
#include "SettingsContent.h"
#include "MainComponent.h"

//==============================================================================
struct DemoContent final : public Component
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
struct CodeContent final : public Component
{
    CodeContent()
    {
        addAndMakeVisible (codeEditor);

        codeEditor.setReadOnly (true);
        codeEditor.setScrollbarThickness (8);

        updateLookAndFeel();
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

    void updateLookAndFeel()
    {
        auto* v4 = dynamic_cast<LookAndFeel_V4*> (&Desktop::getInstance().getDefaultLookAndFeel());

        if (v4 != nullptr && (v4->getCurrentColourScheme() != LookAndFeel_V4::getLightColourScheme()))
            codeEditor.setColourScheme (getDarkColourScheme());
        else
            codeEditor.setColourScheme (getLightColourScheme());
    }

    void lookAndFeelChanged() override
    {
        updateLookAndFeel();
    }

    CodeDocument document;
    CPlusPlusCodeTokeniser cppTokensier;
    CodeEditorComponent codeEditor  { document, &cppTokensier };
};
#endif

//==============================================================================
DemoContentComponent::DemoContentComponent (Component& mainComponent, std::function<void (bool)> callback)
    : TabbedComponent (TabbedButtonBar::Orientation::TabsAtTop),
      demoChangedCallback (std::move (callback))
{
    demoContent.reset (new DemoContent());
    addTab ("Demo",     Colours::transparentBlack, demoContent.get(), false);

   #if ! (JUCE_ANDROID || JUCE_IOS)
    codeContent.reset (new CodeContent());
    addTab ("Code",     Colours::transparentBlack, codeContent.get(), false);
   #endif

    addTab ("Settings", Colours::transparentBlack, new SettingsContent (dynamic_cast<MainComponent&> (mainComponent)), true);

    setTabBarDepth (40);
    updateLookAndFeel();
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

void DemoContentComponent::updateLookAndFeel()
{
    auto backgroundColour = findColour (ResizableWindow::backgroundColourId);

    for (int i = 0; i < getNumTabs(); ++i)
        setTabBackgroundColour (i, backgroundColour);
}

void DemoContentComponent::lookAndFeelChanged()
{
    updateLookAndFeel();
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
