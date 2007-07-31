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

#ifndef __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__
#define __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__

#include "juce_AudioIODeviceType.h"
#include "juce_MidiInput.h"
#include "juce_MidiOutput.h"
#include "../../../juce_core/text/juce_XmlElement.h"
#include "../../gui/components/controls/juce_ComboBox.h"


//==============================================================================
/**
    Manages the state of some audio and midi i/o devices.

    This class keeps tracks of a currently-selected audio device, through
    with which it continuously streams data from an audio callback, as well as
    one or more midi inputs.

    The idea is that your application will create one global instance of this object,
    and let it take care of creating and deleting specific types of audio devices
    internally. So when the device is changed, your callbacks will just keep running
    without having to worry about this.

    The manager can save and reload all of its device settings as XML, which
    makes it very easy for you to save and reload the audio setup of your
    application.

    And to make it easy to let the user change its settings, there's a component
    to do just that - the AudioDeviceSelectorComponent class, which contains a set of
    device selection/sample-rate/latency controls.

    To use an AudioDeviceManager, create one, and use initialise() to set it up. Then
    call setAudioCallback() to register your audio callback with it, and use that to process
    your audio data.

    The manager also acts as a handy hub for incoming midi messages, allowing a
    listener to register for messages from either a specific midi device, or from whatever
    the current default midi input device is. The listener then doesn't have to worry about
    re-registering with different midi devices if they are changed or deleted.

    And yet another neat trick is that amount of CPU time being used is measured and
    available with the getCpuUsage() method.

    The AudioDeviceManager is a ChangeBroadcaster, and will send a change message to
    listeners whenever one of its settings is changed.

    @see AudioDeviceSelectorComponent, AudioIODevice, AudioIODeviceType
*/
class JUCE_API  AudioDeviceManager  : public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates a default AudioDeviceManager.

        Initially no audio device will be selected. You should call the initialise() method
        and register an audio callback with setAudioCallback() before it'll be able to
        actually make any noise.
    */
    AudioDeviceManager();

    /** Destructor. */
    ~AudioDeviceManager();


    //==============================================================================
    /** Opens a set of audio devices ready for use.

        This will attempt to open either a default audio device, or one that was
        previously saved as XML.

        @param numInputChannelsNeeded       a minimum number of input channels needed
                                            by your app.
        @param numOutputChannelsNeeded      a minimum number of output channels to open
        @param savedState                   either a previously-saved state that was produced
                                            by createStateXml(), or 0 if you want the manager
                                            to choose the best device to open.
        @param selectDefaultDeviceOnFailure if true, then if the device specified in the XML
                                            fails to open, then a default device will be used
                                            instead. If false, then on failure, no device is
                                            opened.
        @returns an error message if anything went wrong, or an empty string if it worked ok.
    */
    const String initialise (const int numInputChannelsNeeded,
                             const int numOutputChannelsNeeded,
                             const XmlElement* const savedState,
                             const bool selectDefaultDeviceOnFailure);

    /** Returns some XML representing the current state of the manager.

        This stores the current device, its samplerate, block size, etc, and
        can be restored later with initialise().
    */
    XmlElement* createStateXml() const;


    //==============================================================================
    /** Returns a list of the audio devices that can be used.

        On Windows, this will include both DSound and ASIO devices if they are available. On
        the Mac, it'll be a list of the CoreAudio devices.

        These names are used by setAudioDevice() when changing devices.
    */
    const StringArray getAvailableAudioDeviceNames() const;

    /** Rescans the list of known audio devices, in case it's changed. */
    void refreshDeviceList() const;

    /** Sets a flag to indicate that when listing audio device names, it should treat them
        as inputs rather than outputs.

        This only really applies to DirectSound, where input and output devices are
        separate. On ASIO and CoreAudio this makes no difference.

        @see getAvailableAudioDeviceNames
    */
    void setInputDeviceNamesUsed (const bool useInputNames);

    /** Just adds the list of device names to a combo box.

        The only reason this is in this class is so that it can divide DSound
        and ASIO devices into labelled sections, which makes it look much neater.
    */
    void addDeviceNamesToComboBox (ComboBox& combo) const;

    /** Changes the audio device that should be used.

        If deviceName is empty or not a valid name returned by getAvailableAudioDeviceNames(),
        it will disable the current device.

        @param deviceName           the name of the device you want to use (or an empty string to
                                    deselect the current device)
        @param blockSizeToUse       the samples-per-block you want to use, or 0 to use a default
                                    value
        @param sampleRateToUse      the target sample-rate, or 0 to use a default that the device
                                    is capable of
        @param inputChans           which of the audio device's input channels to open - pass 0 to
                                    open as many of the the first ones as are needed for the number
                                    of input channels that the app has requested
        @param outputChans          which of the audio device's input channels to open - pass 0 to
                                    open as many of the the first ones as are needed for the number
                                    of output channels that the app has requested
        @param treatAsChosenDevice  if this is true and if the device opens correctly, these new
                                    settings will be taken as having been explicitly chosen by the
                                    user, and the next time createStateXml() is called, these settings
                                    will be returned. If it's false, then the device is treated as a
                                    temporary or default device, and a call to createStateXml() will
                                    return either the last settings that were made with treatAsChosenDevice
                                    as true, or the last XML settings that were passed into initialise().

        @returns an error message if anything went wrong, or an empty string if it worked ok.
    */
    const String setAudioDevice (const String& deviceName,
                                 int blockSizeToUse,
                                 double sampleRateToUse,
                                 const BitArray* inputChans,
                                 const BitArray* outputChans,
                                 const bool treatAsChosenDevice);

    /** Returns the name of the currently selected audio device.

        This will be an empty string if none is active.
    */
    const String getCurrentAudioDeviceName() const;

    /** Returns the currently-active audio device. */
    AudioIODevice* getCurrentAudioDevice() const throw()            { return currentAudioDevice; }

    /** Returns the set of audio input channels currently being used.

        To select different channels, use setInputChannels(), or call setAudioDevice() to
        reopen the device with a different set of channels.
    */
    const BitArray getInputChannels() const throw()                 { return inputChannels; }

    /** Changes the active set of input channels.

        @param newEnabledChannels   the set of channels to enable
        @param treatAsChosenDevice  if this is true and if the device opens correctly, these new
                                    settings will be taken as having been explicitly chosen by the
                                    user, and the next time createStateXml() is called, these settings
                                    will be returned. If it's false, then the device is treated as a
                                    temporary or default device, and a call to createStateXml() will
                                    return either the last settings that were made with treatAsChosenDevice
                                    as true, or the last XML settings that were passed into initialise().
        @see getInputChannels
    */
    void setInputChannels (const BitArray& newEnabledChannels,
                           const bool treatAsChosenDevice);

    /** Returns the set of audio output channels currently being used.

        To select different channels, use setOutputChannels(), or call setAudioDevice() to
        reopen the device with a different set of channels.
    */
    const BitArray getOutputChannels() const throw()                { return outputChannels; }

    /** Changes the active set of output channels.

        @param newEnabledChannels   the set of channels to enable
        @param treatAsChosenDevice  if this is true and if the device opens correctly, these new
                                    settings will be taken as having been explicitly chosen by the
                                    user, and the next time createStateXml() is called, these settings
                                    will be returned. If it's false, then the device is treated as a
                                    temporary or default device, and a call to createStateXml() will
                                    return either the last settings that were made with treatAsChosenDevice
                                    as true, or the last XML settings that were passed into initialise().
        @see getOutputChannels
    */
    void setOutputChannels (const BitArray& newEnabledChannels,
                            const bool treatAsChosenDevice);

    /** Gives the manager an audio callback to use.

        The manager will redirect callbacks from whatever audio device is currently
        in use to this callback object.

        You can pass 0 in here to stop callbacks being made.
    */
    void setAudioCallback (AudioIODeviceCallback* newCallback);

    /** Returns the average proportion of available CPU being spent inside the audio callbacks.

        Returns a value between 0 and 1.0
    */
    double getCpuUsage() const;

    //==============================================================================
    /** Enables or disables a midi input device.

        The list of devices can be obtained with the MidiInput::getDevices() method.

        Any incoming messages from enabled input devices will be forwarded on to all the
        listeners that have been registered with the addMidiInputCallback() method. They
        can either register for messages from a particular device, or from just the
        "default" midi input.

        Routing the midi input via an AudioDeviceManager means that when a listener
        registers for the default midi input, this default device can be changed by the
        manager without the listeners having to know about it or re-register.

        It also means that a listener can stay registered for a midi input that is disabled
        or not present, so that when the input is re-enabled, the listener will start
        receiving messages again.

        @see addMidiInputCallback, isMidiInputEnabled
    */
    void setMidiInputEnabled (const String& midiInputDeviceName,
                              const bool enabled);

    /** Returns true if a given midi input device is being used.

        @see setMidiInputEnabled
    */
    bool isMidiInputEnabled (const String& midiInputDeviceName) const;

    /** Registers a listener for callbacks when midi events arrive from a midi input.

        The device name can be empty to indicate that it wants events from whatever the
        current "default" device is. Or it can be the name of one of the midi input devices
        (see MidiInput::getDevices() for the names).

        Only devices which are enabled (see the setMidiInputEnabled() method) will have their
        events forwarded on to listeners.
    */
    void addMidiInputCallback (const String& midiInputDeviceName,
                               MidiInputCallback* callback);

    /** Removes a listener that was previously registered with addMidiInputCallback().
    */
    void removeMidiInputCallback (MidiInputCallback* callback);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    OwnedArray <AudioIODeviceType> availableDeviceTypes;

    AudioIODevice* currentAudioDevice;
    AudioIODeviceCallback* currentCallback;
    int numInputChansNeeded, numOutputChansNeeded;
    BitArray inputChannels, outputChannels;
    XmlElement* lastExplicitSettings;
    mutable bool listNeedsScanning;
    bool useInputNames;

    OwnedArray <MidiInput> enabledMidiInputs;
    Array <MidiInputCallback*> midiCallbacks;
    Array <MidiInput*> midiCallbackDevices;
    CriticalSection audioCallbackLock, midiCallbackLock;

    double cpuUsageMs, timeToCpuScale;

    //==============================================================================
    class CallbackHandler  : public AudioIODeviceCallback,
                             public MidiInputCallback
    {
    public:
        AudioDeviceManager* owner;

        void audioDeviceIOCallback (const float** inputChannelData,
                                    int totalNumInputChannels,
                                    float** outputChannelData,
                                    int totalNumOutputChannels,
                                    int numSamples);

        void audioDeviceAboutToStart (double sampleRate, int blockSize);

        void audioDeviceStopped();

        void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);
    };

    CallbackHandler callbackHandler;

    friend class CallbackHandler;

    void audioDeviceIOCallbackInt (const float** inputChannelData,
                                   int totalNumInputChannels,
                                   float** outputChannelData,
                                   int totalNumOutputChannels,
                                   int numSamples);
    void audioDeviceAboutToStartInt (double sampleRate, int blockSize);
    void audioDeviceStoppedInt();

    void handleIncomingMidiMessageInt (MidiInput* source, const MidiMessage& message);

    const String restartDevice (int blockSizeToUse, double sampleRateToUse,
                                const BitArray& ins, const BitArray& outs);
    void stopDevice();

    AudioDeviceManager (const AudioDeviceManager&);
    const AudioDeviceManager& operator= (const AudioDeviceManager&);
};

#endif   // __JUCE_AUDIODEVICEMANAGER_JUCEHEADER__
