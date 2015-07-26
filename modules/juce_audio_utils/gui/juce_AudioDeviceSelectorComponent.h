/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_AUDIODEVICESELECTORCOMPONENT_H_INCLUDED
#define JUCE_AUDIODEVICESELECTORCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A component containing controls to let the user change the audio settings of
    an AudioDeviceManager object.

    Very easy to use - just create one of these and show it to the user.

    @see AudioDeviceManager
*/
class JUCE_API  AudioDeviceSelectorComponent  : public Component,
                                                private ComboBoxListener, // (can't use ComboBox::Listener due to idiotic VC2005 bug)
                                                private ChangeListener
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
                                  int minAudioInputChannels,
                                  int maxAudioInputChannels,
                                  int minAudioOutputChannels,
                                  int maxAudioOutputChannels,
                                  bool showMidiInputOptions,
                                  bool showMidiOutputSelector,
                                  bool showChannelsAsStereoPairs,
                                  bool hideAdvancedOptionsWithButton);

    /** Destructor */
    ~AudioDeviceSelectorComponent();

    /** The device manager that this component is controlling */
    AudioDeviceManager& deviceManager;

    /** Sets the standard height used for items in the panel. */
    void setItemHeight (int itemHeight);

    /** Returns the standard height used for items in the panel. */
    int getItemHeight() const noexcept      { return itemHeight; }

    //==============================================================================
    /** @internal */
    void resized() override;

private:
    //==============================================================================
    ScopedPointer<ComboBox> deviceTypeDropDown;
    ScopedPointer<Label> deviceTypeDropDownLabel;
    ScopedPointer<Component> audioDeviceSettingsComp;
    String audioDeviceSettingsCompType;
    int itemHeight;
    const int minOutputChannels, maxOutputChannels, minInputChannels, maxInputChannels;
    const bool showChannelsAsStereoPairs;
    const bool hideAdvancedOptionsWithButton;

    class MidiInputSelectorComponentListBox;
    friend struct ContainerDeletePolicy<MidiInputSelectorComponentListBox>;
    ScopedPointer<MidiInputSelectorComponentListBox> midiInputsList;
    ScopedPointer<ComboBox> midiOutputSelector;
    ScopedPointer<Label> midiInputsLabel, midiOutputLabel;

    void comboBoxChanged (ComboBox*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void updateAllControls();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDeviceSelectorComponent)
};


#endif   // JUCE_AUDIODEVICESELECTORCOMPONENT_H_INCLUDED
