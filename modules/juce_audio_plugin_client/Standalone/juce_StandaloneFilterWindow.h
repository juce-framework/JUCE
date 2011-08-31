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

#ifndef __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__
#define __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__

#include "../utility/juce_PluginHeaders.h"


//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your JUCEApplication::initialise() method, and
    let it do its work. It will create your filter object using the same createFilter() function
    that the other plugin wrappers use.
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                  public ButtonListener   // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    //==============================================================================
    /** Creates a window with a given title and colour.
        The settings object can be a PropertySet that the class should use to
        store its settings - the object that is passed-in will be owned by this
        class and deleted automatically when no longer needed. (It can also be null)
    */
    StandaloneFilterWindow (const String& title,
                            const Colour& backgroundColour,
                            PropertySet* settingsToUse);

    ~StandaloneFilterWindow();

    //==============================================================================
    /** Deletes and re-creates the filter and its UI. */
    void resetFilter();

    /** Pops up a dialog letting the user save the filter's state to a file. */
    void saveState();

    /** Pops up a dialog letting the user re-load the filter's state from a file. */
    void loadState();

    /** Shows the audio properties dialog box modally. */
    virtual void showAudioSettingsDialog();

    //==============================================================================
    /** @internal */
    void closeButtonPressed();
    /** @internal */
    void buttonClicked (Button*);
    /** @internal */
    void resized();

private:
    ScopedPointer<PropertySet> settings;
    ScopedPointer<AudioProcessor> filter;
    ScopedPointer<AudioDeviceManager> deviceManager;
    AudioProcessorPlayer player;
    TextButton optionsButton;

    void deleteFilter();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StandaloneFilterWindow);
};

#endif   // __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__
