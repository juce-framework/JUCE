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
