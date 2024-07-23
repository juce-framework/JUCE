/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.CaptureFailure;
import android.hardware.camera2.CaptureResult;

public class CameraCaptureSessionCaptureCallback extends CameraCaptureSession.CaptureCallback
{
    private native void cameraCaptureSessionCaptureCompleted (long host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, TotalCaptureResult result);
    private native void cameraCaptureSessionCaptureFailed (long host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, CaptureFailure failure);
    private native void cameraCaptureSessionCaptureProgressed (long host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, CaptureResult partialResult);
    private native void cameraCaptureSessionCaptureStarted (long host, boolean isPreview, CameraCaptureSession session, CaptureRequest request, long timestamp, long frameNumber);
    private native void cameraCaptureSessionCaptureSequenceAborted (long host, boolean isPreview, CameraCaptureSession session, int sequenceId);
    private native void cameraCaptureSessionCaptureSequenceCompleted (long host, boolean isPreview, CameraCaptureSession session, int sequenceId, long frameNumber);

    CameraCaptureSessionCaptureCallback (long hostToUse, boolean shouldBePreview)
    {
        host = hostToUse;
        preview = shouldBePreview;
    }

    @Override
    public void onCaptureCompleted (CameraCaptureSession session, CaptureRequest request,
                                    TotalCaptureResult result)
    {
        cameraCaptureSessionCaptureCompleted (host, preview, session, request, result);
    }

    @Override
    public void onCaptureFailed (CameraCaptureSession session, CaptureRequest request, CaptureFailure failure)
    {
        cameraCaptureSessionCaptureFailed (host, preview, session, request, failure);
    }

    @Override
    public void onCaptureProgressed (CameraCaptureSession session, CaptureRequest request,
                                     CaptureResult partialResult)
    {
        cameraCaptureSessionCaptureProgressed (host, preview, session, request, partialResult);
    }

    @Override
    public void onCaptureSequenceAborted (CameraCaptureSession session, int sequenceId)
    {
        cameraCaptureSessionCaptureSequenceAborted (host, preview, session, sequenceId);
    }

    @Override
    public void onCaptureSequenceCompleted (CameraCaptureSession session, int sequenceId, long frameNumber)
    {
        cameraCaptureSessionCaptureSequenceCompleted (host, preview, session, sequenceId, frameNumber);
    }

    @Override
    public void onCaptureStarted (CameraCaptureSession session, CaptureRequest request, long timestamp,
                                  long frameNumber)
    {
        cameraCaptureSessionCaptureStarted (host, preview, session, request, timestamp, frameNumber);
    }

    private long host;
    private boolean preview;
}
