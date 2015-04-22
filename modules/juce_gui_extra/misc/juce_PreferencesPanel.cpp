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
    DrawableButton* const button = new DrawableButton (title, DrawableButton::ImageAboveTextLabel);
    buttons.add (button);

    button->setImages (icon, overIcon, downIcon);
    button->setRadioGroupId (1);
    button->addListener (this);
    button->setClickingTogglesState (true);
    button->setWantsKeyboardFocus (false);
    addAndMakeVisible (button);

    resized();

    if (currentPage == nullptr)
        setCurrentPage (title);
}

void PreferencesPanel::addSettingsPage (const String& title, const void* imageData, const int imageDataSize)
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
        buttons.getUnchecked(i)->setBounds (i * buttonSize, 0, buttonSize, buttonSize);

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

        currentPage = nullptr;
        currentPage = createComponentForPage (pageName);

        if (currentPage != nullptr)
        {
            addAndMakeVisible (currentPage);
            currentPage->toBack();
            resized();
        }

        for (int i = 0; i < buttons.size(); ++i)
        {
            if (buttons.getUnchecked(i)->getName() == pageName)
            {
                buttons.getUnchecked(i)->setToggleState (true, dontSendNotification);
                break;
            }
        }
    }
}

void PreferencesPanel::buttonClicked (Button*)
{
    for (int i = 0; i < buttons.size(); ++i)
    {
        if (buttons.getUnchecked(i)->getToggleState())
        {
            setCurrentPage (buttons.getUnchecked(i)->getName());
            break;
        }
    }
}
