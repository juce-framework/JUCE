/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_PreferencesPanel.h"
#include "../buttons/juce_DrawableButton.h"
#include "../windows/juce_DialogWindow.h"
#include "../../graphics/imaging/juce_ImageCache.h"
#include "../../graphics/drawables/juce_DrawableImage.h"


//==============================================================================
PreferencesPanel::PreferencesPanel()
    : buttonSize (70)
{
}

PreferencesPanel::~PreferencesPanel()
{
    currentPage = 0;
    deleteAllChildren();
}

//==============================================================================
void PreferencesPanel::addSettingsPage (const String& title,
                                        const Drawable* icon,
                                        const Drawable* overIcon,
                                        const Drawable* downIcon)
{
    DrawableButton* button = new DrawableButton (title, DrawableButton::ImageAboveTextLabel);
    button->setImages (icon, overIcon, downIcon);
    button->setRadioGroupId (1);
    button->addButtonListener (this);
    button->setClickingTogglesState (true);
    button->setWantsKeyboardFocus (false);
    addAndMakeVisible (button);

    resized();

    if (currentPage == 0)
        setCurrentPage (title);
}

void PreferencesPanel::addSettingsPage (const String& title,
                                        const char* imageData,
                                        const int imageDataSize)
{
    DrawableImage icon, iconOver, iconDown;
    icon.setImage (ImageCache::getFromMemory (imageData, imageDataSize), true);

    iconOver.setImage (ImageCache::getFromMemory (imageData, imageDataSize), true);
    iconOver.setOverlayColour (Colours::black.withAlpha (0.12f));

    iconDown.setImage (ImageCache::getFromMemory (imageData, imageDataSize), true);
    iconDown.setOverlayColour (Colours::black.withAlpha (0.25f));

    addSettingsPage (title, &icon, &iconOver, &iconDown);
}

//==============================================================================
class PrefsDialogWindow  : public DialogWindow
{
public:
    PrefsDialogWindow (const String& dialogtitle,
                       const Colour& backgroundColour)
        : DialogWindow (dialogtitle, backgroundColour, true)
    {
    }

    ~PrefsDialogWindow()
    {
    }

    void closeButtonPressed()
    {
        exitModalState (0);
    }

private:
    PrefsDialogWindow (const PrefsDialogWindow&);
    const PrefsDialogWindow& operator= (const PrefsDialogWindow&);
};

//==============================================================================
void PreferencesPanel::showInDialogBox (const String& dialogtitle,
                                        int dialogWidth,
                                        int dialogHeight,
                                        const Colour& backgroundColour)
{
    setSize (dialogWidth, dialogHeight);

    PrefsDialogWindow dw (dialogtitle, backgroundColour);

    dw.setContentComponent (this, true, true);
    dw.centreAroundComponent (0, dw.getWidth(), dw.getHeight());
    dw.runModalLoop();
    dw.setContentComponent (0, false, false);
}

//==============================================================================
void PreferencesPanel::resized()
{
    int x = 0;

    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        Component* c = getChildComponent (i);

        if (dynamic_cast <DrawableButton*> (c) == 0)
        {
            c->setBounds (0, buttonSize + 5, getWidth(), getHeight() - buttonSize - 5);
        }
        else
        {
            c->setBounds (x, 0, buttonSize, buttonSize);
            x += buttonSize;
        }
    }
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

        currentPage = 0;
        currentPage = createComponentForPage (pageName);

        if (currentPage != 0)
        {
            addAndMakeVisible (currentPage);
            currentPage->toBack();
            resized();
        }

        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            DrawableButton* db = dynamic_cast <DrawableButton*> (getChildComponent (i));

            if (db != 0 && db->getName() == pageName)
            {
                db->setToggleState (true, false);
                break;
            }
        }
    }
}

void PreferencesPanel::buttonClicked (Button*)
{
    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        DrawableButton* db = dynamic_cast <DrawableButton*> (getChildComponent (i));

        if (db != 0 && db->getToggleState())
        {
            setCurrentPage (db->getName());
            break;
        }
    }
}

END_JUCE_NAMESPACE
