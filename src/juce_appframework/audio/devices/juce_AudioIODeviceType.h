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

#ifndef __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__
#define __JUCE_AUDIOIODEVICETYPE_JUCEHEADER__

#include "juce_AudioIODevice.h"


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

        String deviceNames (types[i]->getDeviceNames());  // This will now return a list of available devices of this type

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
    /** Creates a list of available types.

        This will add a set of new AudioIODeviceType objects to the specified list, to
        represent each available types of device.

        The objects that are created should be managed by the caller (the OwnedArray
        will delete them when the array is itself deleted).

        When created, the objects are uninitialised, so you should call scanForDevices()
        on each one before getting its list of devices.
    */
    static void createDeviceTypes (OwnedArray <AudioIODeviceType>& list);

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

        @param preferInputNames     only really used by DirectSound where devices are split up
                                    into inputs and outputs, this indicates whether to use
                                    the input or output name to refer to a pair of devices.
    */
    virtual const StringArray getDeviceNames (const bool preferInputNames = false) const = 0;

    /** Returns the name of the default device.

        This will be one of the names from the getDeviceNames() list.

        @param preferInputNames     only really used by DirectSound where devices are split up
                                    into inputs and outputs, this indicates whether to use
                                    the input or output name to refer to a pair of devices.
    */
    virtual const String getDefaultDeviceName (const bool preferInputNames = false) const = 0;

    /** Creates one of the devices of this type.

        The deviceName must be one of the strings returned by getDeviceNames(), and
        scanForDevices() must have been called before this method is used.
    */
    virtual AudioIODevice* createDevice (const String& deviceName) = 0;


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
