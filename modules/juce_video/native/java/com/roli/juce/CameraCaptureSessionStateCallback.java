package com.roli.juce;

import android.hardware.camera2.CameraCaptureSession;

public class CameraCaptureSessionStateCallback extends CameraCaptureSession.StateCallback
{
    private native void cameraCaptureSessionActive (long host, CameraCaptureSession session);

    private native void cameraCaptureSessionClosed (long host, CameraCaptureSession session);

    private native void cameraCaptureSessionConfigureFailed (long host, CameraCaptureSession session);

    private native void cameraCaptureSessionConfigured (long host, CameraCaptureSession session);

    private native void cameraCaptureSessionReady (long host, CameraCaptureSession session);

    CameraCaptureSessionStateCallback (long hostToUse)
    {
        host = hostToUse;
    }

    @Override
    public void onActive (CameraCaptureSession session)
    {
        cameraCaptureSessionActive (host, session);
    }

    @Override
    public void onClosed (CameraCaptureSession session)
    {
        cameraCaptureSessionClosed (host, session);
    }

    @Override
    public void onConfigureFailed (CameraCaptureSession session)
    {
        cameraCaptureSessionConfigureFailed (host, session);
    }

    @Override
    public void onConfigured (CameraCaptureSession session)
    {
        cameraCaptureSessionConfigured (host, session);
    }

    @Override
    public void onReady (CameraCaptureSession session)
    {
        cameraCaptureSessionReady (host, session);
    }

    private long host;
}
