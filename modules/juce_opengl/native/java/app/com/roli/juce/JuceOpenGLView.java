package com.roli.juce;

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
