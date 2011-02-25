/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

package com.juce;

import android.app.Activity;
import android.os.Bundle;
import android.content.Context;
import android.view.ViewGroup;
import android.graphics.Paint;
import android.graphics.Canvas;
import android.graphics.Path;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.Rect;
import android.text.ClipboardManager;
import com.juce.ComponentPeerView;


//==============================================================================
public final class JuceAppActivity   extends Activity
{
    //==============================================================================
    static
    {
        System.loadLibrary ("juce_jni");
    }

    @Override
    public final void onCreate (Bundle savedInstanceState)
    {
        super.onCreate (savedInstanceState);

        viewHolder = new ViewHolder (this);
        setContentView (viewHolder);
    }

    @Override
    protected final void onDestroy()
    {
        quitApp();
        super.onDestroy();
    }

    private void callAppLauncher()
    {
        launchApp (getApplicationInfo().publicSourceDir,
                   getApplicationInfo().dataDir);
    }

    //==============================================================================
    public native void launchApp (String appFile, String appDataDir);
    public native void quitApp();
    public native void setScreenSize (int screenWidth, int screenHeight);

    //==============================================================================
    public static final void printToConsole (String s)
    {
        android.util.Log.i ("Juce", s);
    }

    //==============================================================================
    public native void deliverMessage (long value);
    private android.os.Handler messageHandler = new android.os.Handler();

    public final void postMessage (long value)
    {
        messageHandler.post (new MessageCallback (value));
    }

    final class MessageCallback  implements Runnable
    {
        public MessageCallback (long value_)
        {
            value = value_;
        }

        public final void run()
        {
            deliverMessage (value);
        }

        private long value;
    }

    //==============================================================================
    private ViewHolder viewHolder;

    public final ComponentPeerView createNewView (boolean opaque)
    {
        ComponentPeerView v = new ComponentPeerView (this, opaque);
        viewHolder.addView (v);
        return v;
    }

    public final void deleteView (ComponentPeerView view)
    {
        viewHolder.removeView (view);
    }

    final class ViewHolder  extends ViewGroup
    {
        public ViewHolder (Context context)
        {
            super (context);
            setDescendantFocusability (ViewGroup.FOCUS_AFTER_DESCENDANTS);
            setFocusable (false);
        }

        protected final void onLayout (boolean changed, int left, int top, int right, int bottom)
        {
            setScreenSize (getWidth(), getHeight());

            if (isFirstResize)
            {
                isFirstResize = false;
                callAppLauncher();
            }
        }

        private boolean isFirstResize = true;
    }

    public final void excludeClipRegion (android.graphics.Canvas canvas, float left, float top, float right, float bottom)
    {
        canvas.clipRect (left, top, right, bottom, android.graphics.Region.Op.DIFFERENCE);
    }

    //==============================================================================
    public final String getClipboardContent()
    {
        ClipboardManager clipboard = (ClipboardManager) getSystemService (CLIPBOARD_SERVICE);
        return clipboard.getText().toString();
    }

    public final void setClipboardContent (String newText)
    {
        ClipboardManager clipboard = (ClipboardManager) getSystemService (CLIPBOARD_SERVICE);
        clipboard.setText (newText);
    }

    //==============================================================================
    public final int[] renderGlyph (char glyph, Paint paint, Matrix matrix, Rect bounds)
    {
        Path p = new Path();
        paint.getTextPath (String.valueOf (glyph), 0, 1, 0.0f, 0.0f, p);

        RectF boundsF = new RectF();
        p.computeBounds (boundsF, true);
        matrix.mapRect (boundsF);

        boundsF.roundOut (bounds);
        bounds.left--;
        bounds.right++;

        final int w = bounds.width();
        final int h = bounds.height();

        Bitmap bm = Bitmap.createBitmap (w, h, Bitmap.Config.ARGB_8888);

        Canvas c = new Canvas (bm);
        matrix.postTranslate (-bounds.left, -bounds.top);
        c.setMatrix (matrix);
        c.drawPath (p, paint);

        int sizeNeeded = w * h;
        if (cachedRenderArray.length < sizeNeeded)
            cachedRenderArray = new int [sizeNeeded];

        bm.getPixels (cachedRenderArray, 0, w, 0, 0, w, h);
        bm.recycle();
        return cachedRenderArray;
    }

    private int[] cachedRenderArray = new int [256];
}
