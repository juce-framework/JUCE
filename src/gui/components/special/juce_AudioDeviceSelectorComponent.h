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

#ifndef __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
#define __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__

#include "../controls/juce_ComboBox.h"
#include "../controls/juce_ListBox.h"
#include "../../../audio/devices/juce_AudioDeviceManager.h"
class MidiInputSelectorComponentListBox;

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
        @param showMidiInputOptions     if true, the component will allow the user to select which midi inputs are enabled
        @param showMidiOutputSelector   if true, the component will let the user choose a default midi output device
        @param showChannelsAsStereoPairs    if true, channels will be treated as pairs; if false, channels will be
                                        treated as a set of separate mono channels.
        @param hideAdvancedOptionsWithButton    if true, only the minimum amount of UI components
                                        are shown, with an "advanced" button that shows the rest of them
    */
    AudioDeviceSelectorComponent (AudioDeviceManager& deviceManager,
                                  const int minAudioInputChannels,
                                  const int maxAudioInputChannels,
                                  const int minAudioOutputChannels,
                                  const int maxAudioOutputChannels,
                                  const bool showMidiInputOptions,
                                  const bool showMidiOutputSelector,
                                  const bool showChannelsAsStereoPairs,
                                  const bool hideAdvancedOptionsWithButton);

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
    /** @internal */
    void childBoundsChanged (Component*);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioDeviceManager& deviceManager;
    ComboBox* deviceTypeDropDown;
    Label* deviceTypeDropDownLabel;
    Component* audioDeviceSettingsComp;
    String audioDeviceSettingsCompType;
    const int minOutputChannels, maxOutputChannels, minInputChannels, maxInputChannels;
    const bool showChannelsAsStereoPairs;
    const bool hideAdvancedOptionsWithButton;

    MidiInputSelectorComponentListBox* midiInputsList;
    Label* midiInputsLabel;
    ComboBox* midiOutputSelector;
    Label* midiOutputLabel;

    AudioDeviceSelectorComponent (const AudioDeviceSelectorComponent&);
    const AudioDeviceSelectorComponent& operator= (const AudioDeviceSelectorComponent&);
};


#endif   // __JUCE_AUDIODEVICESELECTORCOMPONENT_JUCEHEADER__
