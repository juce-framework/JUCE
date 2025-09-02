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

AppleRemoteDevice::AppleRemoteDevice()
    : device (nullptr),
      queue (nullptr),
      remoteId (0)
{
}

AppleRemoteDevice::~AppleRemoteDevice()
{
    stop();
}

namespace
{
    io_object_t getAppleRemoteDevice()
    {
        CFMutableDictionaryRef dict = IOServiceMatching ("AppleIRController");

        io_iterator_t iter = 0;
        io_object_t iod = 0;

        const auto defaultPort = []
        {
           #if JUCE_MAC_API_VERSION_CAN_BE_BUILT (12, 0)
            if (@available (macOS 12.0, *))
                return kIOMainPortDefault;
           #endif

            JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
            return kIOMasterPortDefault;
            JUCE_END_IGNORE_DEPRECATION_WARNINGS
        }();

        if (IOServiceGetMatchingServices (defaultPort, dict, &iter) == kIOReturnSuccess
             && iter != 0)
        {
            iod = IOIteratorNext (iter);
        }

        IOObjectRelease (iter);
        return iod;
    }

    bool createAppleRemoteInterface (io_object_t iod, void** device)
    {
        jassert (*device == nullptr);
        io_name_t classname;

        if (IOObjectGetClass (iod, classname) == kIOReturnSuccess)
        {
            IOCFPlugInInterface** cfPlugInInterface = nullptr;
            SInt32 score = 0;

            if (IOCreatePlugInInterfaceForService (iod,
                                                   kIOHIDDeviceUserClientTypeID,
                                                   kIOCFPlugInInterfaceID,
                                                   &cfPlugInInterface,
                                                   &score) == kIOReturnSuccess)
            {
                [[maybe_unused]] HRESULT hr = (*cfPlugInInterface)->QueryInterface (cfPlugInInterface,
                                                                                    CFUUIDGetUUIDBytes (kIOHIDDeviceInterfaceID),
                                                                                    device);

                (*cfPlugInInterface)->Release (cfPlugInInterface);
            }
        }

        return *device != nullptr;
    }

    void appleRemoteQueueCallback (void* const target, const IOReturn result, void*, void*)
    {
        if (result == kIOReturnSuccess)
            ((AppleRemoteDevice*) target)->handleCallbackInternal();
    }
}

bool AppleRemoteDevice::start (const bool inExclusiveMode)
{
    if (queue != nullptr)
        return true;

    stop();

    bool result = false;
    io_object_t iod = getAppleRemoteDevice();

    if (iod != 0)
    {
        if (createAppleRemoteInterface (iod, &device) && open (inExclusiveMode))
            result = true;
        else
            stop();

        IOObjectRelease (iod);
    }

    return result;
}

void AppleRemoteDevice::stop()
{
    if (queue != nullptr)
    {
        (*(IOHIDQueueInterface**) queue)->stop ((IOHIDQueueInterface**) queue);
        (*(IOHIDQueueInterface**) queue)->dispose ((IOHIDQueueInterface**) queue);
        (*(IOHIDQueueInterface**) queue)->Release ((IOHIDQueueInterface**) queue);
        queue = nullptr;
    }

    if (device != nullptr)
    {
        (*(IOHIDDeviceInterface**) device)->close ((IOHIDDeviceInterface**) device);
        (*(IOHIDDeviceInterface**) device)->Release ((IOHIDDeviceInterface**) device);
        device = nullptr;
    }
}

bool AppleRemoteDevice::isActive() const
{
    return queue != nullptr;
}

bool AppleRemoteDevice::open (const bool openInExclusiveMode)
{
    Array<int> cookies;

    CFObjectHolder<CFArrayRef> elements;
    auto device122 = (IOHIDDeviceInterface122**) device;

    if ((*device122)->copyMatchingElements (device122, nullptr, &elements.object) != kIOReturnSuccess)
        return false;

    for (int i = 0; i < CFArrayGetCount (elements.object); ++i)
    {
        auto element = (CFDictionaryRef) CFArrayGetValueAtIndex (elements.object, i);

        // get the cookie
        CFTypeRef object = CFDictionaryGetValue (element, CFSTR (kIOHIDElementCookieKey));

        if (object == nullptr || CFGetTypeID (object) != CFNumberGetTypeID())
            continue;

        long number;
        if (! CFNumberGetValue ((CFNumberRef) object, kCFNumberLongType, &number))
            continue;

        cookies.add ((int) number);
    }

    if ((*(IOHIDDeviceInterface**) device)
            ->open ((IOHIDDeviceInterface**) device,
                    openInExclusiveMode ? kIOHIDOptionsTypeSeizeDevice
                                        : kIOHIDOptionsTypeNone) == KERN_SUCCESS)
    {
        queue = (*(IOHIDDeviceInterface**) device)->allocQueue ((IOHIDDeviceInterface**) device);

        if (queue != nullptr)
        {
            (*(IOHIDQueueInterface**) queue)->create ((IOHIDQueueInterface**) queue, 0, 12);

            for (int i = 0; i < cookies.size(); ++i)
            {
                IOHIDElementCookie cookie = (IOHIDElementCookie) cookies.getUnchecked (i);
                (*(IOHIDQueueInterface**) queue)->addElement ((IOHIDQueueInterface**) queue, cookie, 0);
            }

            CFRunLoopSourceRef eventSource;

            if ((*(IOHIDQueueInterface**) queue)
                    ->createAsyncEventSource ((IOHIDQueueInterface**) queue, &eventSource) == KERN_SUCCESS)
            {
                if ((*(IOHIDQueueInterface**) queue)->setEventCallout ((IOHIDQueueInterface**) queue,
                                                                       appleRemoteQueueCallback, this, nullptr) == KERN_SUCCESS)
                {
                    CFRunLoopAddSource (CFRunLoopGetCurrent(), eventSource, kCFRunLoopDefaultMode);

                    (*(IOHIDQueueInterface**) queue)->start ((IOHIDQueueInterface**) queue);

                    return true;
                }
            }
        }
    }

    return false;
}

void AppleRemoteDevice::handleCallbackInternal()
{
    int totalValues = 0;
    AbsoluteTime nullTime = { 0, 0 };
    char cookies [12];
    int numCookies = 0;

    while (numCookies < numElementsInArray (cookies))
    {
        IOHIDEventStruct e;

        if ((*(IOHIDQueueInterface**) queue)->getNextEvent ((IOHIDQueueInterface**) queue, &e, nullTime, 0) != kIOReturnSuccess)
            break;

        if ((int) e.elementCookie == 19)
        {
            remoteId = e.value;
            buttonPressed (switched, false);
        }
        else
        {
            totalValues += e.value;
            cookies [numCookies++] = (char) (pointer_sized_int) e.elementCookie;
        }
    }

    cookies [numCookies++] = 0;

    static const char buttonPatterns[] =
    {
        0x1f, 0x14, 0x12, 0x1f, 0x14, 0x12, 0,
        0x1f, 0x15, 0x12, 0x1f, 0x15, 0x12, 0,
        0x1f, 0x1d, 0x1c, 0x12, 0,
        0x1f, 0x1e, 0x1c, 0x12, 0,
        0x1f, 0x16, 0x12, 0x1f, 0x16, 0x12, 0,
        0x1f, 0x17, 0x12, 0x1f, 0x17, 0x12, 0,
        0x1f, 0x12, 0x04, 0x02, 0,
        0x1f, 0x12, 0x03, 0x02, 0,
        0x1f, 0x12, 0x1f, 0x12, 0,
        0x23, 0x1f, 0x12, 0x23, 0x1f, 0x12, 0,
        19, 0
    };

    int buttonNum = (int) menuButton;
    int i = 0;

    while (i < numElementsInArray (buttonPatterns))
    {
        if (strcmp (cookies, buttonPatterns + i) == 0)
        {
            buttonPressed ((ButtonType) buttonNum, totalValues > 0);
            break;
        }

        i += (int) strlen (buttonPatterns + i) + 1;
        ++buttonNum;
    }
}

} // namespace juce
