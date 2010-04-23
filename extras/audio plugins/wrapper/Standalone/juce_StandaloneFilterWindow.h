/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "juce_AudioFilterStreamer.h"


//==============================================================================
/**
    A class that can be used to run a simple standalone application containing your filter.

    Just create one of these objects in your JUCEApplication::initialise() method, and
    let it do its work. It will create your filter object using the same createFilter() function
    that the other plugin wrappers use.
*/
class StandaloneFilterWindow    : public DocumentWindow,
                                  public ButtonListener
{
public:
    //==============================================================================
    StandaloneFilterWindow (const String& title,
                            const Colour& backgroundColour);

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

    /** Returns the property set to use for storing the app's last state.

        This will be used to store the audio set-up and the filter's last state.
    */
    virtual PropertySet* getGlobalSettings();

    //==============================================================================
    /** @internal */
    void closeButtonPressed();
    /** @internal */
    void buttonClicked (Button*);
    /** @internal */
    void resized();

    juce_UseDebuggingNewOperator

private:
    AudioProcessor* filter;
    AudioFilterStreamingDeviceManager* deviceManager;
    Button* optionsButton;

    void deleteFilter();

    StandaloneFilterWindow (const StandaloneFilterWindow&);
    StandaloneFilterWindow& operator= (const StandaloneFilterWindow&);
};

#endif   // __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__
