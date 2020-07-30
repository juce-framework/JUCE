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

package com.rmsl.juce;

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
