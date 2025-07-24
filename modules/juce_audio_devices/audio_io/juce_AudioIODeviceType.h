/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Represents a type of audio driver, such as DirectSound, ASIO, CoreAudio, etc.

    To get a list of available audio driver types, use the AudioDeviceManager::createAudioDeviceTypes()
    method. Each of the objects returned can then be used to list the available
    devices of that type. E.g.
    @code
    OwnedArray<AudioIODeviceType> types;
    myAudioDeviceManager.createAudioDeviceTypes (types);

    for (int i = 0; i < types.size(); ++i)
    {
        String typeName (types[i]->getTypeName());  // This will be things like "DirectSound", "CoreAudio", etc.

        types[i]->scanForDevices();                 // This must be called before getting the list of devices

        StringArray deviceNames (types[i]->getDeviceNames());  // This will now return a list of available devices of this type

        for (int j = 0; j < deviceNames.size(); ++j)
        {
            AudioIODevice* device = types[i]->createDevice (deviceNames [j]);

            ...
        }
    }
    @endcode

    For an easier way of managing audio devices and their settings, have a look at the
    AudioDeviceManager class.

    @see AudioIODevice, AudioDeviceManager

    @tags{Audio}
*/
class JUCE_API  AudioIODeviceType
{
public:
    //==============================================================================
    /** Returns the name of this type of driver that this object manages.

        This will be something like "DirectSound", "ASIO", "CoreAudio", "ALSA", etc.
    */
    const String& getTypeName() const noexcept                      { return typeName; }

    //==============================================================================
    /** Refreshes the object's cached list of known devices.

        This must be called at least once before calling getDeviceNames() or any of
        the other device creation methods.
    */
    virtual void scanForDevices() = 0;

    /** Returns the list of available devices of this type.

        The scanForDevices() method must have been called to create this list.

        @param wantInputNames    for devices which have separate inputs and outputs
                                 this determines which list of names is returned
    */
    virtual StringArray getDeviceNames (bool wantInputNames = false) const = 0;

    /** Returns the name of the default device.

        This will be one of the names from the getDeviceNames() list.

        @param forInput     if true, this means that a default input device should be
                            returned; if false, it should return the default output
    */
    virtual int getDefaultDeviceIndex (bool forInput) const = 0;

    /** Returns the index of a given device in the list of device names.
        If asInput is true, it shows the index in the inputs list, otherwise it
        looks for it in the outputs list.
    */
    virtual int getIndexOfDevice (AudioIODevice* device, bool asInput) const = 0;

    /** Returns true if two different devices can be used for the input and output.
    */
    virtual bool hasSeparateInputsAndOutputs() const = 0;

    /** Creates one of the devices of this type.

        The deviceName must be one of the strings returned by getDeviceNames(), and
        scanForDevices() must have been called before this method is used.
    */
    virtual AudioIODevice* createDevice (const String& outputDeviceName,
                                         const String& inputDeviceName) = 0;

    //==============================================================================
    /**
        A class for receiving events when audio devices are inserted or removed.

        You can register an AudioIODeviceType::Listener with an AudioIODeviceType object
        using the AudioIODeviceType::addListener() method, and it will be called when
        devices of that type are added or removed.

        @see AudioIODeviceType::addListener, AudioIODeviceType::removeListener
    */
    class Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called when the list of available audio devices changes. */
        virtual void audioDeviceListChanged() = 0;
    };

    /** Adds a listener that will be called when this type of device is added or
        removed from the system.
    */
    void addListener (Listener* listener);

    /** Removes a listener that was previously added with addListener(). */
    void removeListener (Listener* listener);

    //==============================================================================
    /** Destructor. */
    virtual ~AudioIODeviceType();

    //==============================================================================
    /** Creates a CoreAudio device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_CoreAudio();
    /** Creates an iOS device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_iOSAudio();
    /** Creates a WASAPI device type in the specified mode if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_WASAPI (WASAPIDeviceMode deviceMode);
    /** Creates a DirectSound device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_DirectSound();
    /** Creates an ASIO device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_ASIO();
    /** Creates an ALSA device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_ALSA();
    /** Creates a JACK device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_JACK();
    /** Creates an Android device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_Android();
    /** Creates an Android OpenSLES device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_OpenSLES();
    /** Creates an Oboe device type if it's available on this platform, or returns null. */
    static AudioIODeviceType* createAudioIODeviceType_Oboe();

    /** @cond */
    [[deprecated ("You should call the method which takes a WASAPIDeviceMode instead.")]]
    static AudioIODeviceType* createAudioIODeviceType_WASAPI (bool exclusiveMode);
    /** @endcond */

protected:
    explicit AudioIODeviceType (const String& typeName);

    /** Synchronously calls all the registered device list change listeners. */
    void callDeviceChangeListeners();

private:
    String typeName;
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE (AudioIODeviceType)
};

} // namespace juce
