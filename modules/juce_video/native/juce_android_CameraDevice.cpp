/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

// TODO
class AndroidCameraInternal
{
public:
    AndroidCameraInternal()
    {
    }

    ~AndroidCameraInternal()
    {
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidCameraInternal)
};

//==============================================================================
CameraDevice::CameraDevice (const String& name_, int /*index*/)
    : name (name_)
{
    internal = new AndroidCameraInternal();

    // TODO
}

CameraDevice::~CameraDevice()
{
    stopRecording();
    delete static_cast <AndroidCameraInternal*> (internal);
    internal = 0;
}

Component* CameraDevice::createViewerComponent()
{
    // TODO

    return nullptr;
}

String CameraDevice::getFileExtension()
{
    return ".m4a";  // TODO correct?
}

void CameraDevice::startRecordingToFile (const File& file, int quality)
{
    // TODO
}

Time CameraDevice::getTimeOfFirstRecordedFrame() const
{
    // TODO
    return Time();
}

void CameraDevice::stopRecording()
{
    // TODO
}

void CameraDevice::addListener (Listener* listenerToAdd)
{
    // TODO
}

void CameraDevice::removeListener (Listener* listenerToRemove)
{
    // TODO
}

StringArray CameraDevice::getAvailableDevices()
{
    StringArray devs;

    // TODO

    return devs;
}

CameraDevice* CameraDevice::openDevice (int index,
                                        int minWidth, int minHeight,
                                        int maxWidth, int maxHeight)
{
    // TODO

    return nullptr;
}
