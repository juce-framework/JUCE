/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
    AudioFilterBase* filter;
    AudioFilterStreamingDeviceManager* deviceManager;
    Button* optionsButton;

    void deleteFilter();

    StandaloneFilterWindow (const StandaloneFilterWindow&);
    const StandaloneFilterWindow& operator= (const StandaloneFilterWindow&);
};

#endif   // __JUCE_STANDALONEFILTERWINDOW_JUCEHEADER__
