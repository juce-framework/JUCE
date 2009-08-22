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

#ifndef __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__
#define __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__

#include "juce_AudioIODevice.h"
class AudioDeviceManager;
class Component;

//==============================================================================
/**
    Represents a type of audio driver, such as DirectSound, ASIO, CoreAudio, etc.

    To get a list of available audio driver types, use the createDeviceTypes()
    method. Each of the objects returned can then be used to list the available
    devices of that type. E.g.
    @code
    OwnedArray <AudioIODeviceType> types;
    AudioIODeviceType::createDeviceTypes (types);

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
*/
class JUCE_API  AudioIODeviceType
{
public:
    //==============================================================================
    /** Returns the name of this type of driver that this object manages.

        This will be something like "DirectSound", "ASIO", "CoreAudio", "ALSA", etc.
    */
    const String& getTypeName() const throw()                       { return typeName; }

    //==============================================================================
    /** Refreshes the object's cached list of known devices.

        This must be called at least once before calling getDeviceNames() or any of
        the other device creation methods.
    */
    virtual void scanForDevices() = 0;

    /** Returns the list of available devices of this type.

        The scanForDevices() method must have been called to create this list.

        @param wantInputNames     only really used by DirectSound where devices are split up
                                  into inputs and outputs, this indicates whether to use
                                  the input or output name to refer to a pair of devices.
    */
    virtual const StringArray getDeviceNames (const bool wantInputNames = false) const = 0;

    /** Returns the name of the default device.

        This will be one of the names from the getDeviceNames() list.

        @param forInput     if true, this means that a default input device should be
                            returned; if false, it should return the default output
    */
    virtual int getDefaultDeviceIndex (const bool forInput) const = 0;

    /** Returns the index of a given device in the list of device names.
        If asInput is true, it shows the index in the inputs list, otherwise it
        looks for it in the outputs list.
    */
    virtual int getIndexOfDevice (AudioIODevice* device, const bool asInput) const = 0;

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
    struct DeviceSetupDetails
    {
        AudioDeviceManager* manager;
        int minNumInputChannels, maxNumInputChannels;
        int minNumOutputChannels, maxNumOutputChannels;
        bool useStereoPairs;
    };

    //==============================================================================
    /** Destructor. */
    virtual ~AudioIODeviceType();

protected:
    AudioIODeviceType (const tchar* const typeName);

private:
    String typeName;

    AudioIODeviceType (const AudioIODeviceType&);
    const AudioIODeviceType& operator= (const AudioIODeviceType&);
};


#endif   // __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__
