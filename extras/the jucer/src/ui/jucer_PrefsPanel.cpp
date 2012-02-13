/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
public:
    MiscPage()
        : templateDir ("C++ template folder:",
                       StoredSettings::getInstance()->getTemplatesDir(),
                       true, true, false,
                       "*.*", String::empty,
                       "(select the directory containing template .cpp and .h files)"),
          label (String::empty, templateDir.getName())
    {
        addAndMakeVisible (&templateDir);
        label.attachToComponent (&templateDir, true);
    }

    ~MiscPage()
    {
        StoredSettings::getInstance()->setTemplatesDir (templateDir.getCurrentFile());
    }

    void resized()
    {
        templateDir.setBounds (150, 16, getWidth() - 160, 22);
    }

private:
    FilenameComponent templateDir;
    Label label;
};

//==============================================================================
class AboutPage   : public Component
{
public:
    AboutPage()
        : link ("www.rawmaterialsoftware.com/juce",
                URL ("http://www.rawmaterialsoftware.com/juce")),
          logo (ImageCache::getFromMemory (BinaryData::jules_jpg, BinaryData::jules_jpgSize))
    {
        text1.setJustification (Justification::centredTop);
        text1.append ("Programmer Julian Storer, seen here demonstrating a beard designed to "
                      "gain approval from the Linux programming community. Each hair of the beard "
                      "represents one line of source code from the ", Font (13.0f));
        text1.append ("Jucer", Font (13.0f, Font::bold));
        text1.append (" component design tool.", Font (13.0f));

        text2.setJustification (Justification::centred);
        text2.append ("Jucer v" + JUCEApplication::getInstance()->getApplicationVersion()
                        + ", " + SystemStats::getJUCEVersion(), Font (12.0f, Font::bold));

        addAndMakeVisible (&link);
        link.setFont (Font (10.0f, Font::bold | Font::underlined), true);
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colour (0xffebebeb));
        g.drawImageWithin (logo, 0, 4, getWidth(), getHeight() - 144,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);

        text1.draw (g, Rectangle<int> (12, getHeight() - 130, getWidth() - 24, 100).toFloat());
        text2.draw (g, Rectangle<int> (12, getHeight() - 50, getWidth() - 24, 20).toFloat());
    }

    void resized()
    {
        link.setSize (100, 22);
        link.changeWidthToFitText();
        link.setTopLeftPosition ((getWidth() - link.getWidth()) / 2, getHeight() - link.getHeight() - 10);
    }

private:
    HyperlinkButton link;
    Image logo;
    AttributedString text1, text2;
};


//==============================================================================
static const char* miscPage = "Misc";
static const char* keysPage = "Keys";
static const char* aboutPage = "About";

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
            return new KeyMappingEditorComponent (*commandManager->getKeyMappings(), true);
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
    : DialogWindow ("Jucer Preferences", Colour::greyLevel (0.92f), true)
{
    PrefsTabComp* const p = new PrefsTabComp();
    p->setSize (456, 510);

    setContentOwned (p, true);

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
