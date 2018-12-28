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
