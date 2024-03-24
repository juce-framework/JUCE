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

#if JUCE_MAC
 #include "../native/juce_CameraDevice_mac.h"
#elif JUCE_WINDOWS
 #include "../native/juce_CameraDevice_windows.h"
#elif JUCE_IOS
 #include "../native/juce_CameraDevice_ios.h"
#elif JUCE_ANDROID
 #include "../native/juce_CameraDevice_android.h"
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

CameraDevice* CameraDevice::openDevice ([[maybe_unused]] int index,
                                        [[maybe_unused]] int minWidth, [[maybe_unused]] int minHeight,
                                        [[maybe_unused]] int maxWidth, [[maybe_unused]] int maxHeight,
                                        [[maybe_unused]] bool useHighQuality)
{
    jassert (juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager());

   #if ! JUCE_ANDROID && ! JUCE_IOS
    std::unique_ptr<CameraDevice> d (new CameraDevice (getAvailableDevices() [index], index,
                                                       minWidth, minHeight, maxWidth, maxHeight, useHighQuality));
    if (d != nullptr && d->pimpl->openedOk())
        return d.release();
   #else
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
