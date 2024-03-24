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

namespace juce
{

PreferencesPanel::PreferencesPanel()
    : buttonSize (70)
{
}

PreferencesPanel::~PreferencesPanel()
{
}

int PreferencesPanel::getButtonSize() const noexcept
{
    return buttonSize;
}

void PreferencesPanel::setButtonSize (int newSize)
{
    buttonSize = newSize;
    resized();
}

//==============================================================================
void PreferencesPanel::addSettingsPage (const String& title,
                                        const Drawable* icon,
                                        const Drawable* overIcon,
                                        const Drawable* downIcon)
{
    auto* button = new DrawableButton (title, DrawableButton::ImageAboveTextLabel);
    buttons.add (button);

    button->setImages (icon, overIcon, downIcon);
    button->setRadioGroupId (1);
    button->onClick = [this] { clickedPage(); };
    button->setClickingTogglesState (true);
    button->setWantsKeyboardFocus (false);
    addAndMakeVisible (button);

    resized();

    if (currentPage == nullptr)
        setCurrentPage (title);
}

void PreferencesPanel::addSettingsPage (const String& title, const void* imageData, int imageDataSize)
{
    DrawableImage icon, iconOver, iconDown;
    icon.setImage (ImageCache::getFromMemory (imageData, imageDataSize));

    iconOver.setImage (ImageCache::getFromMemory (imageData, imageDataSize));
    iconOver.setOverlayColour (Colours::black.withAlpha (0.12f));

    iconDown.setImage (ImageCache::getFromMemory (imageData, imageDataSize));
    iconDown.setOverlayColour (Colours::black.withAlpha (0.25f));

    addSettingsPage (title, &icon, &iconOver, &iconDown);
}

//==============================================================================
void PreferencesPanel::showInDialogBox (const String& dialogTitle, int dialogWidth, int dialogHeight, Colour backgroundColour)
{
    setSize (dialogWidth, dialogHeight);

    DialogWindow::LaunchOptions o;
    o.content.setNonOwned (this);
    o.dialogTitle                   = dialogTitle;
    o.dialogBackgroundColour        = backgroundColour;
    o.escapeKeyTriggersCloseButton  = false;
    o.useNativeTitleBar             = false;
    o.resizable                     = false;

    o.launchAsync();
}

//==============================================================================
void PreferencesPanel::resized()
{
    for (int i = 0; i < buttons.size(); ++i)
        buttons.getUnchecked (i)->setBounds (i * buttonSize, 0, buttonSize, buttonSize);

    if (currentPage != nullptr)
        currentPage->setBounds (getLocalBounds().withTop (buttonSize + 5));
}

void PreferencesPanel::paint (Graphics& g)
{
    g.setColour (Colours::grey);
    g.fillRect (0, buttonSize + 2, getWidth(), 1);
}

void PreferencesPanel::setCurrentPage (const String& pageName)
{
    if (currentPageName != pageName)
    {
        currentPageName = pageName;

        currentPage.reset();
        currentPage.reset (createComponentForPage (pageName));

        if (currentPage != nullptr)
        {
            addAndMakeVisible (currentPage.get());
            currentPage->toBack();
            resized();
        }

        for (auto* b : buttons)
        {
            if (b->getName() == pageName)
            {
                b->setToggleState (true, dontSendNotification);
                break;
            }
        }
    }
}

void PreferencesPanel::clickedPage()
{
    for (auto* b : buttons)
    {
        if (b->getToggleState())
        {
            setCurrentPage (b->getName());
            break;
        }
    }
}

} // namespace juce
