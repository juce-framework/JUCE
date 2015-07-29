/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

package com.juce;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.view.*;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.graphics.*;
import android.opengl.*;
import android.text.ClipboardManager;
import android.text.InputType;
import android.util.DisplayMetrics;
import android.util.Log;
import java.io.*;
import java.net.URL;
import java.net.HttpURLConnection;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.media.AudioManager;
import android.media.MediaScannerConnection;
import android.media.MediaScannerConnection.MediaScannerConnectionClient;

//==============================================================================
public class JuceAppActivity   extends Activity
{
    //==============================================================================
    static
    {
        System.loadLibrary ("juce_jni");
    }

    @Override
    public void onCreate (Bundle savedInstanceState)
    {
        super.onCreate (savedInstanceState);

        viewHolder = new ViewHolder (this);
        setContentView (viewHolder);

        setVolumeControlStream (AudioManager.STREAM_MUSIC);
    }

    @Override
    protected void onDestroy()
    {
        quitApp();
        super.onDestroy();

        clearDataCache();
    }

    @Override
    protected void onPause()
    {
        if (viewHolder != null)
            viewHolder.onPause();

        suspendApp();
        super.onPause();
    }

    @Override
    protected void onResume()
    {
        super.onResume();

        if (viewHolder != null)
            viewHolder.onResume();

        resumeApp();
    }

    @Override
    public void onConfigurationChanged (Configuration cfg)
    {
        super.onConfigurationChanged (cfg);
        setContentView (viewHolder);
    }

    private void callAppLauncher()
    {
        launchApp (getApplicationInfo().publicSourceDir,
                   getApplicationInfo().dataDir);
    }

    //==============================================================================
    private native void launchApp (String appFile, String appDataDir);
    private native void quitApp();
    private native void suspendApp();
    private native void resumeApp();
    private native void setScreenSize (int screenWidth, int screenHeight, int dpi);

    //==============================================================================
    public native void deliverMessage (long value);
    private android.os.Handler messageHandler = new android.os.Handler();

    public final void postMessage (long value)
    {
        messageHandler.post (new MessageCallback (value));
    }

    private final class MessageCallback  implements Runnable
    {
        public MessageCallback (long value_)        { value = value_; }
        public final void run()                     { deliverMessage (value); }

        private long value;
    }

    //==============================================================================
    private ViewHolder viewHolder;

    public final ComponentPeerView createNewView (boolean opaque, long host)
    {
        ComponentPeerView v = new ComponentPeerView (this, opaque, host);
        viewHolder.addView (v);
        return v;
    }

    public final void deleteView (ComponentPeerView view)
    {
        ViewGroup group = (ViewGroup) (view.getParent());

        if (group != null)
            group.removeView (view);
    }

    public final void deleteOpenGLView (OpenGLView view)
    {
        ViewGroup group = (ViewGroup) (view.getParent());

        if (group != null)
            group.removeView (view);
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
            setScreenSize (getWidth(), getHeight(), getDPI());

            if (isFirstResize)
            {
                isFirstResize = false;
                callAppLauncher();
            }
        }

        public final void onPause()
        {
            for (int i = getChildCount(); --i >= 0;)
            {
                View v = getChildAt (i);

                if (v instanceof ComponentPeerView)
                    ((ComponentPeerView) v).onPause();
            }
        }

        public final void onResume()
        {
            for (int i = getChildCount(); --i >= 0;)
            {
                View v = getChildAt (i);

                if (v instanceof ComponentPeerView)
                    ((ComponentPeerView) v).onResume();
             }
         }

        private final int getDPI()
        {
            DisplayMetrics metrics = new DisplayMetrics();
            getWindowManager().getDefaultDisplay().getMetrics (metrics);
            return metrics.densityDpi;
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
    public final void showMessageBox (String title, String message, final long callback)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder (this);
        builder.setTitle (title)
               .setMessage (message)
               .setCancelable (true)
               .setPositiveButton ("OK", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceAppActivity.this.alertDismissed (callback, 0);
                        }
                    });

        builder.create().show();
    }

    public final void showOkCancelBox (String title, String message, final long callback)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder (this);
        builder.setTitle (title)
               .setMessage (message)
               .setCancelable (true)
               .setPositiveButton ("OK", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceAppActivity.this.alertDismissed (callback, 1);
                        }
                    })
               .setNegativeButton ("Cancel", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceAppActivity.this.alertDismissed (callback, 0);
                        }
                    });

        builder.create().show();
    }

    public final void showYesNoCancelBox (String title, String message, final long callback)
    {
        AlertDialog.Builder builder = new AlertDialog.Builder (this);
        builder.setTitle (title)
               .setMessage (message)
               .setCancelable (true)
               .setPositiveButton ("Yes", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceAppActivity.this.alertDismissed (callback, 1);
                        }
                    })
               .setNegativeButton ("No", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceAppActivity.this.alertDismissed (callback, 2);
                        }
                    })
               .setNeutralButton ("Cancel", new DialogInterface.OnClickListener()
                    {
                        public void onClick (DialogInterface dialog, int id)
                        {
                            dialog.cancel();
                            JuceAppActivity.this.alertDismissed (callback, 0);
                        }
                    });

        builder.create().show();
    }

    public native void alertDismissed (long callback, int id);

    //==============================================================================
    public final class ComponentPeerView extends ViewGroup
                                         implements View.OnFocusChangeListener
    {
        public ComponentPeerView (Context context, boolean opaque_, long host)
        {
            super (context);
            this.host = host;
            setWillNotDraw (false);
            opaque = opaque_;

            setFocusable (true);
            setFocusableInTouchMode (true);
            setOnFocusChangeListener (this);
            requestFocus();

            // swap red and blue colours to match internal opengl texture format
            ColorMatrix colorMatrix = new ColorMatrix();

            float[] colorTransform = { 0,    0,    1.0f, 0,    0,
                                       0,    1.0f, 0,    0,    0,
                                       1.0f, 0,    0,    0,    0,
                                       0,    0,    0,    1.0f, 0 };

            colorMatrix.set (colorTransform);
            paint.setColorFilter (new ColorMatrixColorFilter (colorMatrix));
        }

        //==============================================================================
        private native void handlePaint (long host, Canvas canvas, Paint paint);

        @Override
        public void onDraw (Canvas canvas)
        {
            handlePaint (host, canvas, paint);
        }

        @Override
        public boolean isOpaque()
        {
            return opaque;
        }

        private boolean opaque;
        private long host;
        private Paint paint = new Paint();

        //==============================================================================
        private native void handleMouseDown (long host, int index, float x, float y, long time);
        private native void handleMouseDrag (long host, int index, float x, float y, long time);
        private native void handleMouseUp   (long host, int index, float x, float y, long time);

        @Override
        public boolean onTouchEvent (MotionEvent event)
        {
            int action = event.getAction();
            long time = event.getEventTime();

            switch (action & MotionEvent.ACTION_MASK)
            {
                case MotionEvent.ACTION_DOWN:
                    handleMouseDown (host, event.getPointerId(0), event.getX(), event.getY(), time);
                    return true;

                case MotionEvent.ACTION_CANCEL:
                case MotionEvent.ACTION_UP:
                    handleMouseUp (host, event.getPointerId(0), event.getX(), event.getY(), time);
                    return true;

                case MotionEvent.ACTION_MOVE:
                {
                    int n = event.getPointerCount();
                    for (int i = 0; i < n; ++i)
                        handleMouseDrag (host, event.getPointerId(i), event.getX(i), event.getY(i), time);

                    return true;
                }

                case MotionEvent.ACTION_POINTER_UP:
                {
                    int i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                    handleMouseUp (host, event.getPointerId(i), event.getX(i), event.getY(i), time);
                    return true;
                }

                case MotionEvent.ACTION_POINTER_DOWN:
                {
                    int i = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
                    handleMouseDown (host, event.getPointerId(i), event.getX(i), event.getY(i), time);
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

        public void showKeyboard (String type)
        {
            InputMethodManager imm = (InputMethodManager) getSystemService (Context.INPUT_METHOD_SERVICE);

            if (imm != null)
            {
                if (type.length() > 0)
                {
                    imm.showSoftInput (this, android.view.inputmethod.InputMethodManager.SHOW_IMPLICIT);
                    imm.setInputMethod (getWindowToken(), type);
                }
                else
                {
                    imm.hideSoftInputFromWindow (getWindowToken(), 0);
                }
            }
        }

        @Override
        public boolean onKeyDown (int keyCode, KeyEvent event)
        {
            switch (keyCode)
            {
                case KeyEvent.KEYCODE_VOLUME_UP:
                case KeyEvent.KEYCODE_VOLUME_DOWN:
                    return super.onKeyDown (keyCode, event);

                default: break;
            }

            handleKeyDown (host, keyCode, event.getUnicodeChar());
            return true;
        }

        @Override
        public boolean onKeyUp (int keyCode, KeyEvent event)
        {
            handleKeyUp (host, keyCode, event.getUnicodeChar());
            return true;
        }

        @Override
        public boolean onKeyMultiple (int keyCode, int count, KeyEvent event)
        {
            if (keyCode != KeyEvent.KEYCODE_UNKNOWN || event.getAction() != KeyEvent.ACTION_MULTIPLE)
                return super.onKeyMultiple (keyCode, count, event);

            if (event.getCharacters() != null)
            {
                int utf8Char = event.getCharacters().codePointAt (0);
                handleKeyDown (host, utf8Char, utf8Char);
                return true;
            }

            return false;
        }

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
            viewSizeChanged (host);
        }

        @Override
        protected void onLayout (boolean changed, int left, int top, int right, int bottom)
        {
            for (int i = getChildCount(); --i >= 0;)
                requestTransparentRegion (getChildAt (i));
        }

        private native void viewSizeChanged (long host);

        @Override
        public void onFocusChange (View v, boolean hasFocus)
        {
            if (v == this)
                focusChanged (host, hasFocus);
        }

        private native void focusChanged (long host, boolean hasFocus);

        public void setViewName (String newName)    {}

        public boolean isVisible()                  { return getVisibility() == VISIBLE; }
        public void setVisible (boolean b)          { setVisibility (b ? VISIBLE : INVISIBLE); }

        public boolean containsPoint (int x, int y)
        {
            return true; //xxx needs to check overlapping views
        }

        public final void onPause()
        {
            for (int i = getChildCount(); --i >= 0;)
            {
                View v = getChildAt (i);

                if (v instanceof OpenGLView)
                    ((OpenGLView) v).onPause();
            }
        }

        public final void onResume()
        {
            for (int i = getChildCount(); --i >= 0;)
            {
                View v = getChildAt (i);

                if (v instanceof OpenGLView)
                    ((OpenGLView) v).onResume();
            }
        }

        public OpenGLView createGLView()
        {
            OpenGLView glView = new OpenGLView (getContext());
            addView (glView);
            return glView;
        }
    }

    //==============================================================================
    public final class OpenGLView   extends GLSurfaceView
                                    implements GLSurfaceView.Renderer
    {
        OpenGLView (Context context)
        {
            super (context);
            setEGLContextClientVersion (2);
            setRenderer (this);
            setRenderMode (RENDERMODE_WHEN_DIRTY);
        }

        @Override
        public void onSurfaceCreated (GL10 unused, EGLConfig config)
        {
            contextCreated();
        }

        @Override
        public void onSurfaceChanged (GL10 unused, int width, int height)
        {
            contextChangedSize();
        }

        @Override
        public void onDrawFrame (GL10 unused)
        {
            render();
        }

        private native void contextCreated();
        private native void contextChangedSize();
        private native void render();
    }

    //==============================================================================
    public final int[] renderGlyph (char glyph, Paint paint, android.graphics.Matrix matrix, Rect bounds)
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
        final int h = Math.max (1, bounds.height());

        Bitmap bm = Bitmap.createBitmap (w, h, Bitmap.Config.ARGB_8888);

        Canvas c = new Canvas (bm);
        matrix.postTranslate (-bounds.left, -bounds.top);
        c.setMatrix (matrix);
        c.drawPath (p, paint);

        final int sizeNeeded = w * h;
        if (cachedRenderArray.length < sizeNeeded)
            cachedRenderArray = new int [sizeNeeded];

        bm.getPixels (cachedRenderArray, 0, w, 0, 0, w, h);
        bm.recycle();
        return cachedRenderArray;
    }

    private int[] cachedRenderArray = new int [256];

    //==============================================================================
    public static class HTTPStream
    {
        public HTTPStream (HttpURLConnection connection_,
                           int[] statusCode, StringBuffer responseHeaders) throws IOException
        {
            connection = connection_;

            try
            {
                inputStream = new BufferedInputStream (connection.getInputStream());
            }
            catch (IOException e)
            {
                if (connection.getResponseCode() < org.apache.http.HttpStatus.SC_BAD_REQUEST)
                    throw e;
            }
            finally
            {
                statusCode[0] = connection.getResponseCode();
            }

            if (statusCode[0] >= org.apache.http.HttpStatus.SC_BAD_REQUEST)
                inputStream = connection.getErrorStream();
            else
                inputStream = connection.getInputStream();

            for (java.util.Map.Entry<String, java.util.List<String>> entry : connection.getHeaderFields().entrySet())
                if (entry.getKey() != null && entry.getValue() != null)
                    responseHeaders.append (entry.getKey() + ": "
                                             + android.text.TextUtils.join (",", entry.getValue()) + "\n");
        }

        public final void release()
        {
            try
            {
                inputStream.close();
            }
            catch (IOException e)
            {}

            connection.disconnect();
        }

        public final int read (byte[] buffer, int numBytes)
        {
            int num = 0;

            try
            {
                num = inputStream.read (buffer, 0, numBytes);
            }
            catch (IOException e)
            {}

            if (num > 0)
                position += num;

            return num;
        }

        public final long getPosition()                 { return position; }
        public final long getTotalLength()              { return -1; }
        public final boolean isExhausted()              { return false; }
        public final boolean setPosition (long newPos)  { return false; }

        private HttpURLConnection connection;
        private InputStream inputStream;
        private long position;
    }

    public static final HTTPStream createHTTPStream (String address,
                                                     boolean isPost, byte[] postData, String headers,
                                                     int timeOutMs, int[] statusCode,
                                                     StringBuffer responseHeaders,
                                                     int numRedirectsToFollow)
    {
        // timeout parameter of zero for HttpUrlConnection is a blocking connect (negative value for juce::URL)
        if (timeOutMs < 0)
            timeOutMs = 0;
        else if (timeOutMs == 0)
            timeOutMs = 30000;

        // headers - if not empty, this string is appended onto the headers that are used for the request. It must therefore be a valid set of HTML header directives, separated by newlines.
        // So convert headers string to an array, with an element for each line
        String headerLines[] = headers.split("\\n");

        for (;;)
        {
            try
            {
                HttpURLConnection connection = (HttpURLConnection) (new URL(address).openConnection());

                if (connection != null)
                {
                    try
                    {
                        connection.setInstanceFollowRedirects (false);
                        connection.setConnectTimeout (timeOutMs);
                        connection.setReadTimeout (timeOutMs);

                        // Set request headers
                        for (int i = 0; i < headerLines.length; ++i)
                        {
                            int pos = headerLines[i].indexOf (":");

                            if (pos > 0 && pos < headerLines[i].length())
                            {
                                String field = headerLines[i].substring (0, pos);
                                String value = headerLines[i].substring (pos + 1);

                                if (value.length() > 0)
                                    connection.setRequestProperty (field, value);
                            }
                        }

                        if (isPost)
                        {
                            connection.setRequestMethod ("POST");
                            connection.setDoOutput (true);

                            if (postData != null)
                            {
                                OutputStream out = connection.getOutputStream();
                                out.write(postData);
                                out.flush();
                            }
                        }

                        HTTPStream httpStream = new HTTPStream (connection, statusCode, responseHeaders);

                        // Process redirect & continue as necessary
                        int status = statusCode[0];

                        if (--numRedirectsToFollow >= 0
                             && (status == 301 || status == 302 || status == 303 || status == 307))
                        {
                            // Assumes only one occurrence of "Location"
                            int pos1 = responseHeaders.indexOf ("Location:") + 10;
                            int pos2 = responseHeaders.indexOf ("\n", pos1);

                            if (pos2 > pos1)
                            {
                                String newLocation = responseHeaders.substring(pos1, pos2);
                                // Handle newLocation whether it's absolute or relative
                                URL baseUrl = new URL (address);
                                URL newUrl = new URL (baseUrl, newLocation);
                                String transformedNewLocation = newUrl.toString();

                                if (transformedNewLocation != address)
                                {
                                    address = transformedNewLocation;
                                    // Clear responseHeaders before next iteration
                                    responseHeaders.delete (0, responseHeaders.length());
                                    continue;
                                }
                            }
                        }

                        return httpStream;
                    }
                    catch (Throwable e)
                    {
                        connection.disconnect();
                    }
                }
            }
            catch (Throwable e) {}

            return null;
        }
    }

    public final void launchURL (String url)
    {
        startActivity (new Intent (Intent.ACTION_VIEW, Uri.parse (url)));
    }

    public static final String getLocaleValue (boolean isRegion)
    {
        java.util.Locale locale = java.util.Locale.getDefault();

        return isRegion ? locale.getDisplayCountry  (java.util.Locale.US)
                        : locale.getDisplayLanguage (java.util.Locale.US);
    }

    //==============================================================================
    private final class SingleMediaScanner  implements MediaScannerConnectionClient
    {
        public SingleMediaScanner (Context context, String filename)
        {
            file = filename;
            msc = new MediaScannerConnection (context, this);
            msc.connect();
        }

        @Override
        public void onMediaScannerConnected()
        {
            msc.scanFile (file, null);
        }

        @Override
        public void onScanCompleted (String path, Uri uri)
        {
            msc.disconnect();
        }

        private MediaScannerConnection msc;
        private String file;
    }

    public final void scanFile (String filename)
    {
        new SingleMediaScanner (this, filename);
    }

    public final Typeface getTypeFaceFromAsset (String assetName)
    {
        try
        {
            return Typeface.createFromAsset (this.getResources().getAssets(), assetName);
        }
        catch (Throwable e) {}

        return null;
    }

    final protected static char[] hexArray = "0123456789ABCDEF".toCharArray();

    public static String bytesToHex (byte[] bytes)
    {
        char[] hexChars = new char[bytes.length * 2];

        for (int j = 0; j < bytes.length; ++j)
        {
            int v = bytes[j] & 0xff;
            hexChars[j * 2]     = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0f];
        }

        return new String (hexChars);
    }

    final private java.util.Map dataCache = new java.util.HashMap();

    synchronized private final File getDataCacheFile (byte[] data)
    {
        try
        {
            java.security.MessageDigest digest = java.security.MessageDigest.getInstance ("MD5");
            digest.update (data);

            String key = bytesToHex (digest.digest());

            if (dataCache.containsKey (key))
                return (File) dataCache.get (key);

            File f = new File (this.getCacheDir(), "bindata_" + key);
            f.delete();
            FileOutputStream os = new FileOutputStream (f);
            os.write (data, 0, data.length);
            dataCache.put (key, f);
            return f;
        }
        catch (Throwable e) {}

        return null;
    }

    private final void clearDataCache()
    {
        java.util.Iterator it = dataCache.values().iterator();

        while (it.hasNext())
        {
            File f = (File) it.next();
            f.delete();
        }
    }

    public final Typeface getTypeFaceFromByteArray (byte[] data)
    {
        try
        {
            File f = getDataCacheFile (data);

            if (f != null)
                return Typeface.createFromFile (f);
        }
        catch (Exception e)
        {
            Log.e ("JUCE", e.toString());
        }

        return null;
    }
}
