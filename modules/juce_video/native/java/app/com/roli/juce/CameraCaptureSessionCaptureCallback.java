package com.roli.juce;

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
