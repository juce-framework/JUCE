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

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.Rect;
import android.os.Bundle;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import java.lang.reflect.Method;

public final class ComponentPeerView extends ViewGroup
        implements View.OnFocusChangeListener, Application.ActivityLifecycleCallbacks
{
    public ComponentPeerView (Context context, boolean opaque_, long host)
    {
        super (context);

        if (Application.class.isInstance (context))
        {
            ((Application) context).registerActivityLifecycleCallbacks (this);
        } else
        {
            ((Application) context.getApplicationContext ()).registerActivityLifecycleCallbacks (this);
        }

        this.host = host;
        setWillNotDraw (false);
        opaque = opaque_;

        setFocusable (true);
        setFocusableInTouchMode (true);
        setOnFocusChangeListener (this);

        // swap red and blue colours to match internal opengl texture format
        ColorMatrix colorMatrix = new ColorMatrix ();

        float[] colorTransform = {0, 0, 1.0f, 0, 0,
                0, 1.0f, 0, 0, 0,
                1.0f, 0, 0, 0, 0,
                0, 0, 0, 1.0f, 0};

        colorMatrix.set (colorTransform);
        paint.setColorFilter (new ColorMatrixColorFilter (colorMatrix));

        java.lang.reflect.Method method = null;

        try
        {
            method = getClass ().getMethod ("setLayerType", int.class, Paint.class);
        } catch (SecurityException e)
        {
        } catch (NoSuchMethodException e)
        {
        }

        if (method != null)
        {
            try
            {
                int layerTypeNone = 0;
                method.invoke (this, layerTypeNone, null);
            } catch (java.lang.IllegalArgumentException e)
            {
            } catch (java.lang.IllegalAccessException e)
            {
            } catch (java.lang.reflect.InvocationTargetException e)
            {
            }
        }
    }

    public void clear ()
    {
        host = 0;
    }

    //==============================================================================
    private native void handlePaint (long host, Canvas canvas, Paint paint);

    @Override
    public void onDraw (Canvas canvas)
    {
        if (host == 0)
            return;

        handlePaint (host, canvas, paint);
    }

    @Override
    public boolean isOpaque ()
    {
        return opaque;
    }

    private boolean opaque;
    private long host;
    private Paint paint = new Paint ();

    //==============================================================================
    private native void handleMouseDown (long host, int index, float x, float y, long time);
    private native void handleMouseDrag (long host, int index, float x, float y, long time);
    private native void handleMouseUp (long host, int index, float x, float y, long time);

    @Override
    public boolean onTouchEvent (MotionEvent event)
    {
        if (host == 0)
            return false;

        int action = event.getAction ();
        long time = event.getEventTime ();

        switch (action & MotionEvent.ACTION_MASK)
        {
            case MotionEvent.ACTION_DOWN:
                handleMouseDown (host, event.getPointerId (0), event.getRawX (), event.getRawY (), time);
                return true;

            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP:
                handleMouseUp (host, event.getPointerId (0), event.getRawX (), event.getRawY (), time);
                return true;

            case MotionEvent.ACTION_MOVE:
            {
                handleMouseDrag (host, event.getPointerId (0), event.getRawX (), event.getRawY (), time);

                int n = event.getPointerCount ();

                if (n > 1)
                {
                    int point[] = new int[2];
                    getLocationOnScreen (point);

                    for (int i = 1; i < n; ++i)
                        handleMouseDrag (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
                }

                return true;
            }

            case MotionEvent.ACTION_POINTER_UP:
            {
                int i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;

                if (i == 0)
                {
                    handleMouseUp (host, event.getPointerId (0), event.getRawX (), event.getRawY (), time);
                } else
                {
                    int point[] = new int[2];
                    getLocationOnScreen (point);

                    handleMouseUp (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
                }
                return true;
            }

            case MotionEvent.ACTION_POINTER_DOWN:
            {
                int i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;

                if (i == 0)
                {
                    handleMouseDown (host, event.getPointerId (0), event.getRawX (), event.getRawY (), time);
                } else
                {
                    int point[] = new int[2];
                    getLocationOnScreen (point);

                    handleMouseDown (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
                }
                return true;
            }

            default:
                break;
        }

        return false;
    }

    //==============================================================================
    private native void handleKeyDown (long host, int keycode, int textchar);
    private native void handleKeyUp (long host, int keycode, int textchar);
    private native void handleBackButton (long host);
    private native void handleKeyboardHidden (long host);

    public void showKeyboard (String type)
    {
        InputMethodManager imm = (InputMethodManager) getContext ().getSystemService (Context.INPUT_METHOD_SERVICE);

        if (imm != null)
        {
            if (type.length () > 0)
            {
                imm.showSoftInput (this, android.view.inputmethod.InputMethodManager.SHOW_IMPLICIT);
                imm.setInputMethod (getWindowToken (), type);
                keyboardDismissListener.startListening ();
            } else
            {
                imm.hideSoftInputFromWindow (getWindowToken (), 0);
                keyboardDismissListener.stopListening ();
            }
        }
    }

    public void backButtonPressed ()
    {
        if (host == 0)
            return;

        handleBackButton (host);
    }

    @Override
    public boolean onKeyDown (int keyCode, KeyEvent event)
    {
        if (host == 0)
            return false;

        switch (keyCode)
        {
            case KeyEvent.KEYCODE_VOLUME_UP:
            case KeyEvent.KEYCODE_VOLUME_DOWN:
                return super.onKeyDown (keyCode, event);
            case KeyEvent.KEYCODE_BACK:
            {
                backButtonPressed();
                return true;
            }

            default:
                break;
        }

        handleKeyDown (host, keyCode, event.getUnicodeChar ());
        return true;
    }

    @Override
    public boolean onKeyUp (int keyCode, KeyEvent event)
    {
        if (host == 0)
            return false;

        handleKeyUp (host, keyCode, event.getUnicodeChar ());
        return true;
    }

    @Override
    public boolean onKeyMultiple (int keyCode, int count, KeyEvent event)
    {
        if (host == 0)
            return false;

        if (keyCode != KeyEvent.KEYCODE_UNKNOWN || event.getAction () != KeyEvent.ACTION_MULTIPLE)
            return super.onKeyMultiple (keyCode, count, event);

        if (event.getCharacters () != null)
        {
            int utf8Char = event.getCharacters ().codePointAt (0);
            handleKeyDown (host, utf8Char, utf8Char);
            return true;
        }

        return false;
    }

    //==============================================================================
    private final class KeyboardDismissListener
    {
        public KeyboardDismissListener (ComponentPeerView viewToUse)
        {
            view = viewToUse;
        }

        private void startListening ()
        {
            view.getViewTreeObserver ().addOnGlobalLayoutListener (viewTreeObserver);
        }

        private void stopListening ()
        {
            view.getViewTreeObserver ().removeGlobalOnLayoutListener (viewTreeObserver);
        }

        private class TreeObserver implements ViewTreeObserver.OnGlobalLayoutListener
        {
            TreeObserver ()
            {
                keyboardShown = false;
            }

            @Override
            public void onGlobalLayout ()
            {
                Rect r = new Rect ();

                View parentView = getRootView ();
                int diff = 0;

                if (parentView == null)
                {
                    getWindowVisibleDisplayFrame (r);
                    diff = getHeight () - (r.bottom - r.top);
                } else
                {
                    parentView.getWindowVisibleDisplayFrame (r);
                    diff = parentView.getHeight () - (r.bottom - r.top);
                }

                // Arbitrary threshold, surely keyboard would take more than 20 pix.
                if (diff < 20 && keyboardShown)
                {
                    keyboardShown = false;
                    handleKeyboardHidden (view.host);
                }

                if (!keyboardShown && diff > 20)
                    keyboardShown = true;
            }

            ;

            private boolean keyboardShown;
        }

        ;

        private ComponentPeerView view;
        private TreeObserver viewTreeObserver = new TreeObserver ();
    }

    private KeyboardDismissListener keyboardDismissListener = new KeyboardDismissListener (this);

    // this is here to make keyboard entry work on a Galaxy Tab2 10.1
    @Override
    public InputConnection onCreateInputConnection (EditorInfo outAttrs)
    {
        outAttrs.actionLabel = "";
        outAttrs.hintText = "";
        outAttrs.initialCapsMode = 0;
        outAttrs.initialSelEnd = outAttrs.initialSelStart = -1;
        outAttrs.label = "";
        outAttrs.imeOptions = EditorInfo.IME_ACTION_DONE | EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        outAttrs.inputType = InputType.TYPE_NULL;

        return new BaseInputConnection (this, false);
    }

    //==============================================================================
    @Override
    protected void onSizeChanged (int w, int h, int oldw, int oldh)
    {
        super.onSizeChanged (w, h, oldw, oldh);

        if (host != 0)
            viewSizeChanged (host);
    }

    @Override
    protected void onLayout (boolean changed, int left, int top, int right, int bottom)
    {
    }

    private native void viewSizeChanged (long host);

    @Override
    public void onFocusChange (View v, boolean hasFocus)
    {
        if (host == 0)
            return;

        if (v == this)
            focusChanged (host, hasFocus);
    }

    private native void focusChanged (long host, boolean hasFocus);

    public void setViewName (String newName)
    {
    }

    public void setSystemUiVisibilityCompat (int visibility)
    {
        Method systemUIVisibilityMethod = null;
        try
        {
            systemUIVisibilityMethod = this.getClass ().getMethod ("setSystemUiVisibility", int.class);
        } catch (SecurityException e)
        {
            return;
        } catch (NoSuchMethodException e)
        {
            return;
        }
        if (systemUIVisibilityMethod == null) return;

        try
        {
            systemUIVisibilityMethod.invoke (this, visibility);
        } catch (java.lang.IllegalArgumentException e)
        {
        } catch (java.lang.IllegalAccessException e)
        {
        } catch (java.lang.reflect.InvocationTargetException e)
        {
        }
    }

    public boolean isVisible ()
    {
        return getVisibility () == VISIBLE;
    }

    public void setVisible (boolean b)
    {
        setVisibility (b ? VISIBLE : INVISIBLE);
    }

    public boolean containsPoint (int x, int y)
    {
        return true; //xxx needs to check overlapping views
    }

    //==============================================================================
    private native void handleAppPaused (long host);
    private native void handleAppResumed (long host);

    @Override
    public void onActivityPaused (Activity activity)
    {
        if (host == 0)
            return;

        handleAppPaused (host);
    }

    @Override
    public void onActivityStopped (Activity activity)
    {

    }

    @Override
    public void onActivitySaveInstanceState (Activity activity, Bundle bundle)
    {

    }

    @Override
    public void onActivityDestroyed (Activity activity)
    {

    }

    @Override
    public void onActivityCreated (Activity activity, Bundle bundle)
    {

    }

    @Override
    public void onActivityStarted (Activity activity)
    {

    }

    @Override
    public void onActivityResumed (Activity activity)
    {
        if (host == 0)
            return;

        // Ensure that navigation/status bar visibility is correctly restored.
        handleAppResumed (host);
    }
}
