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

import static android.view.WindowInsetsController.APPEARANCE_LIGHT_CAPTION_BARS;
import static android.view.WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS;
import static android.view.WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS;
import static android.view.WindowInsetsController.BEHAVIOR_DEFAULT;
import static android.view.WindowInsetsController.BEHAVIOR_SHOW_BARS_BY_SWIPE;
import static android.view.WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorMatrix;
import android.graphics.ColorMatrixColorFilter;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.text.Editable;
import android.text.InputType;
import android.text.Selection;
import android.text.SpanWatcher;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextWatcher;
import android.util.Pair;
import android.view.Choreographer;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityNodeProvider;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;

import java.util.ArrayList;

import java.util.List;

public final class ComponentPeerView extends ViewGroup
        implements View.OnFocusChangeListener, Application.ActivityLifecycleCallbacks, Choreographer.FrameCallback
{
    public ComponentPeerView (Context context, boolean opaque_, long host)
    {
        super (context);

        if (context instanceof Application)
        {
            ((Application) context).registerActivityLifecycleCallbacks (this);
        }
        else
        {
            ((Application) context.getApplicationContext()).registerActivityLifecycleCallbacks (this);
        }

        this.host = host;
        setWillNotDraw (false);
        opaque = opaque_;

        setFocusable (true);
        setFocusableInTouchMode (true);
        setOnFocusChangeListener (this);

        // swap red and blue colours to match internal opengl texture format
        ColorMatrix colorMatrix = new ColorMatrix();

        float[] colorTransform = {0, 0, 1.0f, 0, 0,
                0, 1.0f, 0, 0, 0,
                1.0f, 0, 0, 0, 0,
                0, 0, 0, 1.0f, 0};

        colorMatrix.set (colorTransform);
        paint.setColorFilter (new ColorMatrixColorFilter (colorMatrix));

        setLayerType (LAYER_TYPE_NONE, null);

        Choreographer.getInstance().postFrameCallback (this);
    }

    public void clear()
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

    private native void handleDoFrame (long host, long frameTimeNanos);

    @Override
    public void doFrame (long frameTimeNanos)
    {
        if (host == 0)
            return;

        handleDoFrame (host, frameTimeNanos);

        Choreographer.getInstance().postFrameCallback (this);
    }

    @Override
    public boolean isOpaque()
    {
        return opaque;
    }

    private final boolean opaque;
    private long host;
    private final Paint paint = new Paint();

    //==============================================================================
    private static native void handleMouseDown (long host, int index, float x, float y, long time);
    private static native void handleMouseDrag (long host, int index, float x, float y, long time);
    private static native void handleMouseUp (long host, int index, float x, float y, long time);
    private static native void handleAccessibilityHover (long host, int action, float x, float y, long time);

    @FunctionalInterface
    private interface MouseHandler
    {
        void handle (long host, int index, float x, float y, long time);
    }

    void handleMultiPointerEvent (MotionEvent event, MouseHandler callback)
    {
        long time = event.getEventTime();
        callback.handle (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);

        int n = event.getPointerCount();

        if (n > 1)
        {
            int point[] = new int[2];
            getLocationOnScreen (point);

            for (int i = 1; i < n; ++i)
                callback.handle (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
        }
    }

    void handleSecondaryPointerEvent (MotionEvent event, MouseHandler callback)
    {
        long time = event.getEventTime();
        int i = event.getActionIndex();

        if (i == 0)
        {
            callback.handle (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);
        }
        else
        {
            int point[] = new int[2];
            getLocationOnScreen (point);

            callback.handle (host, event.getPointerId (i), event.getX (i) + point[0], event.getY (i) + point[1], time);
        }
    }

    @Override
    public boolean onTouchEvent (MotionEvent event)
    {
        if (host == 0)
            return false;

        int action = event.getAction();
        long time = event.getEventTime();

        switch (action & MotionEvent.ACTION_MASK)
        {
            case MotionEvent.ACTION_DOWN:
                handleMouseDown (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);
                return true;

            case MotionEvent.ACTION_UP:
                handleMouseUp (host, event.getPointerId (0), event.getRawX(), event.getRawY(), time);
                return true;

            case MotionEvent.ACTION_CANCEL:
                handleMultiPointerEvent (event, ComponentPeerView::handleMouseUp);
                return true;

            case MotionEvent.ACTION_MOVE:
                handleMultiPointerEvent (event, ComponentPeerView::handleMouseDrag);
                return true;

            case MotionEvent.ACTION_POINTER_UP:
                handleSecondaryPointerEvent (event, ComponentPeerView::handleMouseUp);
                return true;

            case MotionEvent.ACTION_POINTER_DOWN:
                handleSecondaryPointerEvent (event, ComponentPeerView::handleMouseDown);
                return true;

            default:
                break;
        }

        return false;
    }

    @Override
    public boolean onHoverEvent (MotionEvent event)
    {
        if (accessibilityManager.isTouchExplorationEnabled())
        {
            handleAccessibilityHover (host, event.getActionMasked(), event.getRawX(), event.getRawY(), event.getEventTime());
            return true;
        }

        return false;
    }

    //==============================================================================
    public static class TextInputTarget
    {
        public TextInputTarget (long owner) { host = owner; }

        public boolean isTextInputActive()                                      { return ComponentPeerView.textInputTargetIsTextInputActive (host); }
        public int getHighlightedRegionBegin()                                  { return ComponentPeerView.textInputTargetGetHighlightedRegionBegin (host); }
        public int getHighlightedRegionEnd()                                    { return ComponentPeerView.textInputTargetGetHighlightedRegionEnd (host); }
        public void setHighlightedRegion (int b, int e)                         {        ComponentPeerView.textInputTargetSetHighlightedRegion (host, b, e); }
        public String getTextInRange (int b, int e)                             { return ComponentPeerView.textInputTargetGetTextInRange (host, b, e); }
        public void insertTextAtCaret (String text)                             {        ComponentPeerView.textInputTargetInsertTextAtCaret (host, text); }
        public int getCaretPosition()                                           { return ComponentPeerView.textInputTargetGetCaretPosition (host); }
        public int getTotalNumChars()                                           { return ComponentPeerView.textInputTargetGetTotalNumChars (host); }
        public int getCharIndexForPoint (Point point)                           { return ComponentPeerView.textInputTargetGetCharIndexForPoint (host, point); }
        public int getKeyboardType()                                            { return ComponentPeerView.textInputTargetGetKeyboardType (host); }
        public void setTemporaryUnderlining (List<Pair<Integer, Integer>> list) {        ComponentPeerView.textInputTargetSetTemporaryUnderlining (host, list); }

        //==============================================================================
        private final long host;
    }

    private native static boolean   textInputTargetIsTextInputActive (long host);
    private native static int       textInputTargetGetHighlightedRegionBegin (long host);
    private native static int       textInputTargetGetHighlightedRegionEnd (long host);
    private native static void      textInputTargetSetHighlightedRegion (long host, int begin, int end);
    private native static String    textInputTargetGetTextInRange (long host, int begin, int end);
    private native static void      textInputTargetInsertTextAtCaret (long host, String text);
    private native static int       textInputTargetGetCaretPosition (long host);
    private native static int       textInputTargetGetTotalNumChars (long host);
    private native static int       textInputTargetGetCharIndexForPoint (long host, Point point);
    private native static int       textInputTargetGetKeyboardType (long host);
    private native static void      textInputTargetSetTemporaryUnderlining (long host, List<Pair<Integer, Integer>> list);

    private native long getFocusedTextInputTargetPointer (long host);

    private TextInputTarget getFocusedTextInputTarget (long host)
    {
        final long ptr = getFocusedTextInputTargetPointer (host);
        return ptr != 0 ? new TextInputTarget (ptr) : null;
    }

    //==============================================================================
    private native void handleKeyDown (long host, int keycode, int textchar, int kbFlags);
    private native void handleKeyUp (long host, int keycode, int textchar);
    private native void handleBackButton (long host);
    private native void handleKeyboardHidden (long host);

    private static int getInputTypeForJuceVirtualKeyboardType (int type)
    {
        switch (type)
        {
            case 0:                                             // textKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_NORMAL
                     | InputType.TYPE_TEXT_FLAG_MULTI_LINE
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            case 1:                                             // numericKeyboard
                return InputType.TYPE_CLASS_NUMBER
                     | InputType.TYPE_NUMBER_VARIATION_NORMAL;
            case 2:                                             // decimalKeyboard
                return InputType.TYPE_CLASS_NUMBER
                     | InputType.TYPE_NUMBER_VARIATION_NORMAL
                     | InputType.TYPE_NUMBER_FLAG_DECIMAL;
            case 3:                                             // urlKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_URI
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            case 4:                                             // emailAddressKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
            case 5:                                             // phoneNumberKeyboard
                return InputType.TYPE_CLASS_PHONE;
            case 6:                                             // passwordKeyboard
                return InputType.TYPE_CLASS_TEXT
                     | InputType.TYPE_TEXT_VARIATION_PASSWORD
                     | InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
        }

        return 0;
    }

    InputMethodManager getInputMethodManager()
    {
        return (InputMethodManager) getContext().getSystemService (Context.INPUT_METHOD_SERVICE);
    }

    public void closeInputMethodContext()
    {
        InputMethodManager imm = getInputMethodManager();

        if (imm == null)
            return;

        if (cachedConnection != null)
            cachedConnection.closeConnection();

        imm.restartInput (this);
    }

    public void showKeyboard (int virtualKeyboardType, int selectionStart, int selectionEnd)
    {
        InputMethodManager imm = getInputMethodManager();

        if (imm == null)
            return;

        // restartingInput causes a call back to onCreateInputConnection, where we'll pick
        // up the correct keyboard characteristics to use for the focused TextInputTarget.
        imm.restartInput (this);
        imm.showSoftInput (this, 0);
        keyboardDismissListener.startListening();
    }

    public void hideKeyboard()
    {
        InputMethodManager imm = getInputMethodManager();

        if (imm == null)
            return;

        imm.hideSoftInputFromWindow (getWindowToken(), 0);
        keyboardDismissListener.stopListening();
    }

    public void backButtonPressed()
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

        // The key event may move the cursor, or in some cases it might enter characters (e.g.
        // digits). In this case, we need to reset the IME so that it's aware of the new contents
        // of the TextInputTarget.
        closeInputMethodContext();

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

        handleKeyDown (host,
                       keyCode,
                       event.getUnicodeChar(),
                       event.getMetaState());

        return true;
    }

    @Override
    public boolean onKeyUp (int keyCode, KeyEvent event)
    {
        if (host == 0)
            return false;

        handleKeyUp (host, keyCode, event.getUnicodeChar());
        return true;
    }

    @Override
    public boolean onKeyMultiple (int keyCode, int count, KeyEvent event)
    {
        if (host == 0)
            return false;

        if (keyCode != KeyEvent.KEYCODE_UNKNOWN || (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q && event.getAction() != KeyEvent.ACTION_MULTIPLE))
            return super.onKeyMultiple (keyCode, count, event);

        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.Q && event.getCharacters() != null)
        {
            int utf8Char = event.getCharacters().codePointAt (0);

            handleKeyDown (host,
                           keyCode,
                           utf8Char,
                           event.getMetaState());
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

        private void startListening()
        {
            view.getViewTreeObserver().addOnGlobalLayoutListener (viewTreeObserver);
        }

        private void stopListening()
        {
            view.getViewTreeObserver().removeOnGlobalLayoutListener (viewTreeObserver);
        }

        private class TreeObserver implements ViewTreeObserver.OnGlobalLayoutListener
        {
            TreeObserver()
            {
                keyboardShown = false;
            }

            @Override
            public void onGlobalLayout()
            {
                Rect r = new Rect();

                View parentView = getRootView();
                int diff;

                if (parentView == null)
                {
                    getWindowVisibleDisplayFrame (r);
                    diff = getHeight() - (r.bottom - r.top);
                }
                else
                {
                    parentView.getWindowVisibleDisplayFrame (r);
                    diff = parentView.getHeight() - (r.bottom - r.top);
                }

                // Arbitrary threshold, surely keyboard would take more than 20 pix.
                if (diff < 20 && keyboardShown)
                {
                    keyboardShown = false;
                    handleKeyboardHidden (view.host);
                }

                if (! keyboardShown && diff > 20)
                    keyboardShown = true;
            }

            private boolean keyboardShown;
        }

        private final ComponentPeerView view;
        private final TreeObserver viewTreeObserver = new TreeObserver();
    }

    private final KeyboardDismissListener keyboardDismissListener = new KeyboardDismissListener (this);

    //==============================================================================
    // This implementation is quite similar to the ChangeListener in Android's built-in TextView.
    private static final class ChangeWatcher implements SpanWatcher, TextWatcher
    {
        public ChangeWatcher (ComponentPeerView viewIn, Editable editableIn, TextInputTarget targetIn)
        {
            view = viewIn;
            editable = editableIn;
            target = targetIn;

            updateEditableSelectionFromTarget (editable, target);
        }

        @Override
        public void onSpanAdded (Spannable text, Object what, int start, int end)
        {
            updateTargetRangesFromEditable (editable, target);
        }

        @Override
        public void onSpanRemoved (Spannable text, Object what, int start, int end)
        {
            updateTargetRangesFromEditable (editable, target);
        }

        @Override
        public void onSpanChanged (Spannable text, Object what, int ostart, int oend, int nstart, int nend)
        {
            updateTargetRangesFromEditable (editable, target);
        }

        @Override
        public void afterTextChanged (Editable s)
        {
        }

        @Override
        public void beforeTextChanged (CharSequence s, int start, int count, int after)
        {
            contentsBeforeChange = s.toString();
        }

        @Override
        public void onTextChanged (CharSequence s, int start, int before, int count)
        {
            if (editable != s || contentsBeforeChange == null)
                return;

            final String newText = s.subSequence (start, start + count).toString();

            int code = 0;

            if (newText.endsWith ("\n") || newText.endsWith ("\r"))
                code = KeyEvent.KEYCODE_ENTER;

            if (newText.endsWith ("\t"))
                code = KeyEvent.KEYCODE_TAB;

            target.setHighlightedRegion (contentsBeforeChange.codePointCount (0, start),
                                         contentsBeforeChange.codePointCount (0, start + before));
            target.insertTextAtCaret (code != 0 ? newText.substring (0, newText.length() - 1)
                                                : newText);

            // Treating return/tab as individual keypresses rather than part of the composition
            // sequence allows TextEditor onReturn and onTab to work as expected.
            if (code != 0)
                view.onKeyDown (code, new KeyEvent (KeyEvent.ACTION_DOWN, code));

            updateTargetRangesFromEditable (editable, target);
            contentsBeforeChange = null;
        }

        private static void updateEditableSelectionFromTarget (Editable editable, TextInputTarget text)
        {
            final int start = text.getHighlightedRegionBegin();
            final int end   = text.getHighlightedRegionEnd();

            if (start < 0 || end < 0)
                return;

            final String string = editable.toString();
            Selection.setSelection (editable,
                                    string.offsetByCodePoints (0, start),
                                    string.offsetByCodePoints (0, end));
        }

        private static void updateTargetSelectionFromEditable (Editable editable, TextInputTarget target)
        {
            final int start = Selection.getSelectionStart (editable);
            final int end   = Selection.getSelectionEnd   (editable);

            if (start < 0 || end < 0)
                return;

            final String string = editable.toString();
            target.setHighlightedRegion (string.codePointCount (0, start),
                                         string.codePointCount (0, end));
        }

        private static List<Pair<Integer, Integer>> getUnderlinedRanges (Editable editable)
        {
            final int start = BaseInputConnection.getComposingSpanStart (editable);
            final int end   = BaseInputConnection.getComposingSpanEnd   (editable);

            if (start < 0 || end < 0)
                return null;

            final String string = editable.toString();

            final ArrayList<Pair<Integer, Integer>> pairs = new ArrayList<>();
            pairs.add (new Pair<> (string.codePointCount (0, start), string.codePointCount (0, end)));
            return pairs;
        }

        private static void updateTargetCompositionRangesFromEditable (Editable editable, TextInputTarget target)
        {
            target.setTemporaryUnderlining (getUnderlinedRanges (editable));
        }

        private static void updateTargetRangesFromEditable (Editable editable, TextInputTarget target)
        {
            updateTargetSelectionFromEditable         (editable, target);
            updateTargetCompositionRangesFromEditable (editable, target);
        }

        private final ComponentPeerView view;
        private final TextInputTarget target;
        private final Editable editable;
        private String contentsBeforeChange;
    }

    private static final class Connection extends BaseInputConnection
    {
        Connection (ComponentPeerView viewIn, boolean fullEditor, TextInputTarget targetIn)
        {
            super (viewIn, fullEditor);
            view = viewIn;
            target = targetIn;
        }

        @Override
        public Editable getEditable()
        {
            if (cached != null)
                return cached;

            if (target == null)
                return cached = super.getEditable();

            int length = target.getTotalNumChars();
            String initialText = target.getTextInRange (0, length);
            cached = new SpannableStringBuilder (initialText);
            // Span the entire range of text, so that we pick up changes at any location.
            // Use cached.length rather than target.getTotalNumChars here, because this
            // range is in UTF-16 code units, rather than code points.
            changeWatcher = new ChangeWatcher (view, cached, target);
            cached.setSpan (changeWatcher, 0, cached.length(), Spanned.SPAN_INCLUSIVE_INCLUSIVE);
            return cached;
        }

        /** Call this to stop listening for selection/composition updates.

            We do this before closing the current input method context (e.g. when the user
            taps on a text view to move the cursor), because otherwise the input system
            might send another round of notifications *during* the restartInput call, after we've
            requested that the input session should end.
        */
        @Override
        public void closeConnection()
        {
            if (cached != null && changeWatcher != null)
                cached.removeSpan (changeWatcher);

            cached = null;
            target = null;

            if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
            {
                super.closeConnection();

                if (android.os.Build.VERSION.SDK_INT >= Build.VERSION_CODES.S)
                    setImeConsumesInput (false);
            }
            else
            {
                finishComposingText();
            }
        }

        private ComponentPeerView view;
        private TextInputTarget target;
        private Editable cached;
        private ChangeWatcher changeWatcher;
    }

    @Override
    public InputConnection onCreateInputConnection (EditorInfo outAttrs)
    {
        TextInputTarget focused = getFocusedTextInputTarget (host);

        outAttrs.actionLabel = "";
        outAttrs.hintText = "";
        outAttrs.initialCapsMode = 0;
        outAttrs.initialSelStart = focused != null ? focused.getHighlightedRegionBegin() : -1;
        outAttrs.initialSelEnd   = focused != null ? focused.getHighlightedRegionEnd()   : -1;
        outAttrs.label = "";
        outAttrs.imeOptions = EditorInfo.IME_ACTION_UNSPECIFIED
                            | EditorInfo.IME_FLAG_NO_EXTRACT_UI
                            | EditorInfo.IME_FLAG_NO_ENTER_ACTION;
        outAttrs.inputType = focused != null ? getInputTypeForJuceVirtualKeyboardType (focused.getKeyboardType())
                                             : 0;

        cachedConnection = new Connection (this, true, focused);
        return cachedConnection;
    }

    private Connection cachedConnection;

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

    public void setSystemUiVisibilityCompat (Window window, boolean visible, boolean isLight)
    {
        if (window != null)
        {
            // Although this is deprecated in Android 35+, it still seems to be necessary
            // to adjust the colour of the nav bar icons when in button-mode.
            window.setNavigationBarColor (isLight ? Color.BLACK : Color.WHITE);
        }

        if (30 <= Build.VERSION.SDK_INT)
        {
            WindowInsetsController controller = getWindowInsetsController();

            if (controller != null)
            {
                final int mask = (35 <= Build.VERSION.SDK_INT ? APPEARANCE_LIGHT_CAPTION_BARS : 0)
                               | APPEARANCE_LIGHT_NAVIGATION_BARS
                               | APPEARANCE_LIGHT_STATUS_BARS;
                controller.setSystemBarsAppearance (isLight ? mask : 0, mask);

                if (visible)
                {
                    controller.show (WindowInsets.Type.systemBars());
                    controller.setSystemBarsBehavior (31 <= Build.VERSION.SDK_INT ? BEHAVIOR_DEFAULT
                                                                                  : BEHAVIOR_SHOW_BARS_BY_SWIPE);
                }
                else
                {
                    controller.hide (WindowInsets.Type.systemBars());
                    controller.setSystemBarsBehavior (BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                }

                return;
            }
        }

        if (window == null)
            return;

        // Displays::findDisplays queries the DecorView to determine the
        // most recently-requested visibility state of the system UI.
        // As we're creating new top-level views via WindowManager,
        // updating only the DecorView isn't sufficient to hide the global
        // system UI; we also need to update the view that was added to
        // the WindowManager.
        ArrayList<View> views = new ArrayList<>();
        views.add (window.getDecorView());
        views.add (this);

        for (View view : views)
        {
            final int lightStyle = (26 <= Build.VERSION.SDK_INT ? SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR : 0)
                                 | SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            final int prevFlags = view.getSystemUiVisibility();
            final int fullScreenFlags = SYSTEM_UI_FLAG_HIDE_NAVIGATION
                                      | SYSTEM_UI_FLAG_FULLSCREEN
                                      | SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
            final int withVisibility = visible ? (prevFlags & ~fullScreenFlags)
                                               : (prevFlags |  fullScreenFlags);
            final int withColour = isLight ? (withVisibility |  lightStyle)
                                           : (withVisibility & ~lightStyle);

            view.setSystemUiVisibility (withColour);
        }
    }

    public boolean isVisible()
    {
        return getVisibility() == VISIBLE;
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

    //==============================================================================
    private native View getNativeView (long host, int virtualViewId);
    private native boolean populateAccessibilityNodeInfo (long host, int virtualViewId, AccessibilityNodeInfo info);
    private native boolean handlePerformAction (long host, int virtualViewId, int action, Bundle arguments);
    private native Integer getInputFocusViewId (long host);
    private native Integer getAccessibilityFocusViewId (long host);

    private final class JuceAccessibilityNodeProvider extends AccessibilityNodeProvider
    {
        public JuceAccessibilityNodeProvider (ComponentPeerView viewToUse)
        {
            view = viewToUse;
        }

        @Override
        public AccessibilityNodeInfo createAccessibilityNodeInfo (int virtualViewId)
        {
            if (host == 0)
                return null;

            View nativeView = getNativeView (host, virtualViewId);

            if (nativeView != null)
                return nativeView.createAccessibilityNodeInfo();

            final AccessibilityNodeInfo nodeInfo;

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R)
            {
                nodeInfo = new AccessibilityNodeInfo (view, virtualViewId);
            }
            else
            {
                nodeInfo = AccessibilityNodeInfo.obtain (view, virtualViewId);
            }

            if (! populateAccessibilityNodeInfo (host, virtualViewId, nodeInfo))
            {
                if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.R)
                    nodeInfo.recycle();

                return null;
            }

            return nodeInfo;
        }

        @Override
        public List<AccessibilityNodeInfo> findAccessibilityNodeInfosByText (String text, int virtualViewId)
        {
            return new ArrayList<>();
        }

        @Override
        public AccessibilityNodeInfo findFocus (int focus)
        {
            if (host == 0)
                return null;

            Integer focusViewId = (focus == AccessibilityNodeInfo.FOCUS_INPUT ? getInputFocusViewId (host)
                                                                              : getAccessibilityFocusViewId (host));

            if (focusViewId != null)
                return createAccessibilityNodeInfo (focusViewId);

            return null;
        }

        @Override
        public boolean performAction (int virtualViewId, int action, Bundle arguments)
        {
            if (host == 0)
                return false;

            return handlePerformAction (host, virtualViewId, action, arguments);
        }

        private final ComponentPeerView view;
    }

    private final JuceAccessibilityNodeProvider nodeProvider = new JuceAccessibilityNodeProvider (this);
    private final AccessibilityManager accessibilityManager = (AccessibilityManager) getContext().getSystemService (Context.ACCESSIBILITY_SERVICE);

    @Override
    public AccessibilityNodeProvider getAccessibilityNodeProvider()
    {
        return nodeProvider;
    }
}
