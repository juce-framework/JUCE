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

#ifndef __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
#define __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__

#include "../controls/juce_ComboBox.h"
#include "../controls/juce_ListBox.h"
#include "../../../audio/devices/juce_AudioDeviceManager.h"
class AudioDeviceSelectorComponentListBox;

//==============================================================================
/**
    A component containing controls to let the user change the audio settings of
    an AudioDeviceManager object.

    Very easy to use - just create one of these and show it to the user.

    @see AudioDeviceManager
*/
class JUCE_API  AudioDeviceSelectorComponent  : public Component,
                                                public ComboBoxListener,
                                                public ButtonListener,
                                                public ChangeListener
{
public:
    //==============================================================================
    /** Creates the component.

        If your app needs only output channels, you might ask for a maximum of 0 input
        channels, and the component won't display any options for choosing the input
        channels. And likewise if you're doing an input-only app.

        @param deviceManager            the device manager that this component should control
        @param minAudioInputChannels    the minimum number of audio input channels that the application needs
        @param maxAudioInputChannels    the maximum number of audio input channels that the application needs
        @param minAudioOutputChannels   the minimum number of audio output channels that the application needs
        @param maxAudioOutputChannels   the maximum number of audio output channels that the application needs
        @param showMidiOptions          if true, the component will allow the user to select which midi inputs are
                                        enabled.
    */
    AudioDeviceSelectorComponent (AudioDeviceManager& deviceManager,
                                  const int minAudioInputChannels,
                                  const int maxAudioInputChannels,
                                  const int minAudioOutputChannels,
                                  const int maxAudioOutputChannels,
                                  const bool showMidiOptions);

    /** Destructor */
    ~AudioDeviceSelectorComponent();


    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    void comboBoxChanged (ComboBox*);
    /** @internal */
    void buttonClicked (Button*);
    /** @internal */
    void changeListenerCallback (void*);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioDeviceManager& deviceManager;
    ComboBox* audioDeviceDropDown;
    const int minOutputChannels, maxOutputChannels, minInputChannels, maxInputChannels;
    const bool showMidiOptions;

    ComboBox* sampleRateDropDown;
    AudioDeviceSelectorComponentListBox* inputChansBox;
    Label* inputsLabel;
    AudioDeviceSelectorComponentListBox* outputChansBox;
    Label* outputsLabel;
    Label* sampleRateLabel;
    ComboBox* bufferSizeDropDown;
    Label* bufferSizeLabel;
    Button* launchUIButton;
    AudioDeviceSelectorComponentListBox* midiInputsList;
    Label* midiInputsLabel;

    AudioDeviceSelectorComponent (const AudioDeviceSelectorComponent&);
    const AudioDeviceSelectorComponent& operator= (const AudioDeviceSelectorComponent&);
};


#endif   // __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
