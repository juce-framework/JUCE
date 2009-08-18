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

#include "../jucer_Headers.h"
#include "jucer_PrefsPanel.h"

//==============================================================================
class MiscPage  : public Component
{
    FilenameComponent* templateDir;

public:
    MiscPage()
    {
        addAndMakeVisible (templateDir
            = new FilenameComponent (T("C++ template folder:"),
                                     StoredSettings::getInstance()->getTemplatesDir(),
                                     true,
                                     true,
                                     false,
                                     T("*.*"),
                                     String::empty,
                                     T("(select the directory containing template .cpp and .h files)")));

        (new Label (String::empty, templateDir->getName()))->attachToComponent (templateDir, true);
    }

    ~MiscPage()
    {
        StoredSettings::getInstance()->setTemplatesDir (templateDir->getCurrentFile());

        deleteAllChildren();
    }

    void resized()
    {
        templateDir->setBounds (150, 16, getWidth() - 160, 22);
    }
};

//==============================================================================
class AboutPage   : public Component
{
    HyperlinkButton* link;
    Image* logo;
    TextLayout text1, text2;

public:
    AboutPage()
    {
        logo = ImageCache::getFromMemory (BinaryData::jules_jpg, BinaryData::jules_jpgSize);

        text1.appendText ("Programmer Julian Storer, seen here demonstrating a beard designed to "
                          "gain approval from the Linux programming community. Each hair of the beard "
                          "represents one line of source code from the ", Font (13.0f));
        text1.appendText ("Jucer", Font (13.0f, Font::bold));
        text1.appendText (" component design tool.", Font (13.0f));

        text2.appendText (T("Jucer v") + JUCEApplication::getInstance()->getApplicationVersion()
                            + T(", ") + SystemStats::getJUCEVersion(), Font (14.0f, Font::bold));

        addAndMakeVisible (link = new HyperlinkButton (T("www.rawmaterialsoftware.com/juce"),
                                                       URL (T("http://www.rawmaterialsoftware.com/juce"))));
        link->setFont (Font (10.0f, Font::bold | Font::underlined), true);
    }

    ~AboutPage()
    {
        deleteAllChildren();
        ImageCache::release (logo);
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colour (0xffebebeb));
        g.drawImageWithin (logo, 0, 4, getWidth(), getHeight() - 134,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);

        text1.drawWithin (g, 0, getHeight() - 120, getWidth(), 100, Justification::centredTop);
        text2.drawWithin (g, 0, getHeight() - 50, getWidth(), 100, Justification::centredTop);
    }

    void resized()
    {
        text1.layout (getWidth() - 24, Justification::topLeft, false);
        text2.layout (getWidth() - 24, Justification::centred, false);

        link->setSize (100, 22);
        link->changeWidthToFitText();
        link->setTopLeftPosition ((getWidth() - link->getWidth()) / 2, getHeight() - link->getHeight() - 10);
    }
};


//==============================================================================
static const tchar* miscPage = T("Misc");
static const tchar* keysPage = T("Keys");
static const tchar* aboutPage = T("About");

class PrefsTabComp  : public PreferencesPanel
{
public:
    PrefsTabComp()
    {
        addSettingsPage (miscPage, BinaryData::prefs_misc_png, BinaryData::prefs_misc_pngSize);
        addSettingsPage (keysPage, BinaryData::prefs_keys_png, BinaryData::prefs_keys_pngSize);
        addSettingsPage (aboutPage, BinaryData::prefs_about_png, BinaryData::prefs_about_pngSize);
    }

    ~PrefsTabComp()
    {
        StoredSettings::getInstance()->flush();
    }

    Component* createComponentForPage (const String& pageName)
    {
        if (pageName == miscPage)
        {
            return new MiscPage();
        }
        else if (pageName == keysPage)
        {
            return new KeyMappingEditorComponent (commandManager->getKeyMappings(), true);
        }
        else if (pageName == aboutPage)
        {
            return new AboutPage();
        }

        return new Component();
    }
};

//==============================================================================
static String prefsWindowPos;

PrefsPanel::PrefsPanel()
    : DialogWindow (T("Jucer Preferences"), Colour::greyLevel (0.92f), true)
{
    PrefsTabComp* const p = new PrefsTabComp();
    p->setSize (456, 510);

    setContentComponent (p, true, true);

    if (! restoreWindowStateFromString (prefsWindowPos))
        centreAroundComponent (0, getWidth(), getHeight());

    setResizable (true, true);
    setResizeLimits (400, 400, 1000, 800);
}

PrefsPanel::~PrefsPanel()
{
    prefsWindowPos = getWindowStateAsString();
}

void PrefsPanel::closeButtonPressed()
{
    setVisible (false);
}

void PrefsPanel::show()
{
    PrefsPanel pp;
    pp.runModalLoop();
}
