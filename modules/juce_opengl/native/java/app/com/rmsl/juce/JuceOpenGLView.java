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

import android.content.Context;
import android.graphics.Canvas;
import android.view.SurfaceView;

public class JuceOpenGLView extends SurfaceView
{
    private long host = 0;

    JuceOpenGLView (Context context, long nativeThis)
    {
        super (context);
        host = nativeThis;
    }

    public void cancel ()
    {
        host = 0;
    }

    //==============================================================================
    @Override
    protected void onAttachedToWindow ()
    {
        super.onAttachedToWindow ();

        if (host != 0)
            onAttchedWindowNative (host);
    }

    @Override
    protected void onDetachedFromWindow ()
    {
        if (host != 0)
            onDetachedFromWindowNative (host);

        super.onDetachedFromWindow ();
    }

    @Override
    protected void dispatchDraw (Canvas canvas)
    {
        super.dispatchDraw (canvas);

        if (host != 0)
            onDrawNative (host, canvas);
    }

    //==============================================================================
    private native void onAttchedWindowNative (long nativeThis);

    private native void onDetachedFromWindowNative (long nativeThis);

    private native void onDrawNative (long nativeThis, Canvas canvas);
}
