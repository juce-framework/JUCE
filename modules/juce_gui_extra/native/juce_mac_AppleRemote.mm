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

AppleRemoteDevice::AppleRemoteDevice()
    : device (0),
      queue (0),
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

        if (IOServiceGetMatchingServices (kIOMasterPortDefault, dict, &iter) == kIOReturnSuccess
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
                HRESULT hr = (*cfPlugInInterface)->QueryInterface (cfPlugInInterface,
                                                                   CFUUIDGetUUIDBytes (kIOHIDDeviceInterfaceID),
                                                                   device);

                (void) hr;

                (*cfPlugInInterface)->Release (cfPlugInInterface);
            }
        }

        return *device != 0;
    }

    void appleRemoteQueueCallback (void* const target, const IOReturn result, void*, void*)
    {
        if (result == kIOReturnSuccess)
            ((AppleRemoteDevice*) target)->handleCallbackInternal();
    }
}

bool AppleRemoteDevice::start (const bool inExclusiveMode)
{
    if (queue != 0)
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
    if (queue != 0)
    {
        (*(IOHIDQueueInterface**) queue)->stop ((IOHIDQueueInterface**) queue);
        (*(IOHIDQueueInterface**) queue)->dispose ((IOHIDQueueInterface**) queue);
        (*(IOHIDQueueInterface**) queue)->Release ((IOHIDQueueInterface**) queue);
        queue = 0;
    }

    if (device != 0)
    {
        (*(IOHIDDeviceInterface**) device)->close ((IOHIDDeviceInterface**) device);
        (*(IOHIDDeviceInterface**) device)->Release ((IOHIDDeviceInterface**) device);
        device = 0;
    }
}

bool AppleRemoteDevice::isActive() const
{
    return queue != 0;
}

bool AppleRemoteDevice::open (const bool openInExclusiveMode)
{
    Array <int> cookies;

    CFArrayRef elements;
    IOHIDDeviceInterface122** const device122 = (IOHIDDeviceInterface122**) device;

    if ((*device122)->copyMatchingElements (device122, 0, &elements) != kIOReturnSuccess)
        return false;

    for (int i = 0; i < CFArrayGetCount (elements); ++i)
    {
        CFDictionaryRef element = (CFDictionaryRef) CFArrayGetValueAtIndex (elements, i);

        // get the cookie
        CFTypeRef object = CFDictionaryGetValue (element, CFSTR (kIOHIDElementCookieKey));

        if (object == 0 || CFGetTypeID (object) != CFNumberGetTypeID())
            continue;

        long number;
        if (! CFNumberGetValue ((CFNumberRef) object, kCFNumberLongType, &number))
            continue;

        cookies.add ((int) number);
    }

    CFRelease (elements);

    if ((*(IOHIDDeviceInterface**) device)
            ->open ((IOHIDDeviceInterface**) device,
                    openInExclusiveMode ? kIOHIDOptionsTypeSeizeDevice
                                        : kIOHIDOptionsTypeNone) == KERN_SUCCESS)
    {
        queue = (*(IOHIDDeviceInterface**) device)->allocQueue ((IOHIDDeviceInterface**) device);

        if (queue != 0)
        {
            (*(IOHIDQueueInterface**) queue)->create ((IOHIDQueueInterface**) queue, 0, 12);

            for (int i = 0; i < cookies.size(); ++i)
            {
                IOHIDElementCookie cookie = (IOHIDElementCookie) cookies.getUnchecked(i);
                (*(IOHIDQueueInterface**) queue)->addElement ((IOHIDQueueInterface**) queue, cookie, 0);
            }

            CFRunLoopSourceRef eventSource;

            if ((*(IOHIDQueueInterface**) queue)
                    ->createAsyncEventSource ((IOHIDQueueInterface**) queue, &eventSource) == KERN_SUCCESS)
            {
                if ((*(IOHIDQueueInterface**) queue)->setEventCallout ((IOHIDQueueInterface**) queue,
                                                                       appleRemoteQueueCallback, this, 0) == KERN_SUCCESS)
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
    //DBG (String::toHexString ((uint8*) cookies, numCookies, 1) + " "  + String (totalValues));

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
