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

package com.roli.juce;

import android.hardware.camera2.CameraDevice;

public class CameraDeviceStateCallback extends CameraDevice.StateCallback
{
    private native void cameraDeviceStateClosed (long host, CameraDevice camera);
    private native void cameraDeviceStateDisconnected (long host, CameraDevice camera);
    private native void cameraDeviceStateError (long host, CameraDevice camera, int error);
    private native void cameraDeviceStateOpened (long host, CameraDevice camera);

    CameraDeviceStateCallback (long hostToUse)
    {
        host = hostToUse;
    }

    @Override
    public void onClosed (CameraDevice camera)
    {
        cameraDeviceStateClosed (host, camera);
    }

    @Override
    public void onDisconnected (CameraDevice camera)
    {
        cameraDeviceStateDisconnected (host, camera);
    }

    @Override
    public void onError (CameraDevice camera, int error)
    {
        cameraDeviceStateError (host, camera, error);
    }

    @Override
    public void onOpened (CameraDevice camera)
    {
        cameraDeviceStateOpened (host, camera);
    }

    private long host;
}
