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
