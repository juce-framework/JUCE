/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_MAC || JUCE_IOS
 #include "../native/juce_mac_CameraDevice.h"
#elif JUCE_WINDOWS
 #include "../native/juce_win32_CameraDevice.h"
#elif JUCE_ANDROID
 #include "../native/juce_android_CameraDevice.h"
#endif

//==============================================================================
CameraDevice::CameraDevice (const String& nm, int index, int minWidth, int minHeight, int maxWidth, int maxHeight, bool useHighQuality)
   : name (nm), pimpl (new Pimpl (name, index, minWidth, minHeight, maxWidth, maxHeight, useHighQuality))
{
}

CameraDevice::~CameraDevice()
{
    stopRecording();
    pimpl.reset();
}

Component* CameraDevice::createViewerComponent()
{
    return new ViewerComponent (*this);
}

void CameraDevice::startRecordingToFile (const File& file, int quality)
{
    stopRecording();
    pimpl->startRecordingToFile (file, quality);
}

Time CameraDevice::getTimeOfFirstRecordedFrame() const
{
    return pimpl->getTimeOfFirstRecordedFrame();
}

void CameraDevice::stopRecording()
{
    pimpl->stopRecording();
}

void CameraDevice::addListener (Listener* listenerToAdd)
{
    if (listenerToAdd != nullptr)
        pimpl->addListener (listenerToAdd);
}

void CameraDevice::removeListener (Listener* listenerToRemove)
{
    if (listenerToRemove != nullptr)
        pimpl->removeListener (listenerToRemove);
}

//==============================================================================
StringArray CameraDevice::getAvailableDevices()
{
    JUCE_AUTORELEASEPOOL
    {
        return Pimpl::getAvailableDevices();
    }
}

CameraDevice* CameraDevice::openDevice (int index,
                                        int minWidth, int minHeight,
                                        int maxWidth, int maxHeight,
                                        bool useHighQuality)
{
    if (ScopedPointer<CameraDevice> d = new CameraDevice (getAvailableDevices() [index], index,
                                                          minWidth, minHeight, maxWidth, maxHeight, useHighQuality))
        if (d->pimpl->openedOk())
            return d.release();

    return nullptr;
}

} // namespace juce
