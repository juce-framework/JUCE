/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

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
