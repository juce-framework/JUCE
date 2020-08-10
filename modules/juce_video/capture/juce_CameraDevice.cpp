/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_MAC
 #include "../native/juce_mac_CameraDevice.h"
#elif JUCE_WINDOWS
 #include "../native/juce_win32_CameraDevice.h"
#elif JUCE_IOS
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability-new")

 #include "../native/juce_ios_CameraDevice.h"

 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#elif JUCE_ANDROID
 #include "../native/juce_android_CameraDevice.h"
#endif

#if JUCE_ANDROID || JUCE_IOS
//==============================================================================
class CameraDevice::CameraFactory
{
public:
    static CameraFactory& getInstance()
    {
        static CameraFactory factory;
        return factory;
    }

    void openCamera (int index, OpenCameraResultCallback resultCallback,
                     int minWidth, int minHeight, int maxWidth, int maxHeight, bool useHighQuality)
    {
        auto cameraId = getAvailableDevices()[index];

        if (getCameraIndex (cameraId) != -1)
        {
            // You are trying to open the same camera twice.
            jassertfalse;
            return;
        }

        std::unique_ptr<CameraDevice> device (new CameraDevice (cameraId, index,
                                                                minWidth, minHeight, maxWidth,
                                                                maxHeight, useHighQuality));

        camerasToOpen.add ({ nextRequestId++,
                             std::unique_ptr<CameraDevice> (device.release()),
                             resultCallback });

        auto& pendingOpen = camerasToOpen.getReference (camerasToOpen.size() - 1);

        pendingOpen.device->pimpl->open ([this] (const String& deviceId, const String& error)
                                         {
                                             int cIndex = getCameraIndex (deviceId);

                                             if (cIndex == -1)
                                                 return;

                                             auto& cameraPendingOpen = camerasToOpen.getReference (cIndex);

                                             if (error.isEmpty())
                                                 cameraPendingOpen.resultCallback (cameraPendingOpen.device.release(), error);
                                             else
                                                 cameraPendingOpen.resultCallback (nullptr, error);

                                             int id = cameraPendingOpen.requestId;

                                             MessageManager::callAsync ([this, id]() { removeRequestWithId (id); });
                                         });
    }

private:
    int getCameraIndex (const String& cameraId) const
    {
        for (int i = 0; i < camerasToOpen.size(); ++i)
        {
            auto& pendingOpen = camerasToOpen.getReference (i);

            if (pendingOpen.device->pimpl->getCameraId() == cameraId)
                return i;
        }

        return -1;
    }

    void removeRequestWithId (int id)
    {
        for (int i = camerasToOpen.size(); --i >= 0;)
        {
            if (camerasToOpen.getReference (i).requestId == id)
            {
                camerasToOpen.remove (i);
                return;
            }
        }
    }

    struct PendingCameraOpen
    {
        int requestId;
        std::unique_ptr<CameraDevice> device;
        OpenCameraResultCallback resultCallback;
    };

    Array<PendingCameraOpen> camerasToOpen;
    static int nextRequestId;
};

int CameraDevice::CameraFactory::nextRequestId = 0;

#endif

//==============================================================================
CameraDevice::CameraDevice (const String& nm, int index, int minWidth, int minHeight, int maxWidth, int maxHeight, bool useHighQuality)
   : name (nm), pimpl (new Pimpl (*this, name, index, minWidth, minHeight, maxWidth, maxHeight, useHighQuality))
{
}

CameraDevice::~CameraDevice()
{
    jassert (juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    stopRecording();
    pimpl.reset();
}

Component* CameraDevice::createViewerComponent()
{
    return new ViewerComponent (*this);
}

void CameraDevice::takeStillPicture (std::function<void (const Image&)> pictureTakenCallback)
{
    pimpl->takeStillPicture (pictureTakenCallback);
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
    jassert (juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

   #if ! JUCE_ANDROID && ! JUCE_IOS
    std::unique_ptr<CameraDevice> d (new CameraDevice (getAvailableDevices() [index], index,
                                                       minWidth, minHeight, maxWidth, maxHeight, useHighQuality));
    if (d != nullptr && d->pimpl->openedOk())
        return d.release();
   #else
    ignoreUnused (index, minWidth, minHeight);
    ignoreUnused (maxWidth, maxHeight, useHighQuality);

    // Use openDeviceAsync to open a camera device on iOS or Android.
    jassertfalse;
   #endif

    return nullptr;
}

void CameraDevice::openDeviceAsync (int index, OpenCameraResultCallback resultCallback,
                                    int minWidth, int minHeight, int maxWidth, int maxHeight, bool useHighQuality)
{
    jassert (juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

    if (resultCallback == nullptr)
    {
        // A valid callback must be passed.
        jassertfalse;
        return;
    }

   #if JUCE_ANDROID || JUCE_IOS
    CameraFactory::getInstance().openCamera (index, std::move (resultCallback),
                                             minWidth, minHeight, maxWidth, maxHeight, useHighQuality);
   #else
    auto* device = openDevice (index, minWidth, minHeight, maxWidth, maxHeight, useHighQuality);

    resultCallback (device, device != nullptr ? String() : "Could not open camera device");
   #endif
}

} // namespace juce
