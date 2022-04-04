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

import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Message;
import android.webkit.WebResourceResponse;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.SslErrorHandler;
import android.webkit.WebChromeClient;


//==============================================================================
public class JuceWebView21
{
    static public class Client extends WebViewClient
    {
        public Client (long hostToUse)
        {
            host = hostToUse;
        }

        public void hostDeleted ()
        {
            synchronized (hostLock)
            {
                host = 0;
            }
        }

        public void onPageFinished (WebView view, String url)
        {
            if (host == 0)
                return;

            webViewPageLoadFinished (host, view, url);
        }

        public void onReceivedSslError (WebView view, SslErrorHandler handler, SslError error)
        {
            if (host == 0)
                return;

            webViewReceivedSslError (host, view, handler, error);
        }

        public void onPageStarted (WebView view, String url, Bitmap favicon)
        {
            if (host != 0)
                webViewPageLoadStarted (host, view, url);
        }

        public WebResourceResponse shouldInterceptRequest (WebView view, String url)
        {
            synchronized (hostLock)
            {
                if (host != 0)
                {
                    boolean shouldLoad = webViewPageLoadStarted (host, view, url);

                    if (shouldLoad)
                        return null;
                }
            }

            return new WebResourceResponse ("text/html", null, null);
        }

        private native boolean webViewPageLoadStarted (long host, WebView view, String url);

        private native void webViewPageLoadFinished (long host, WebView view, String url);

        private native void webViewReceivedSslError (long host, WebView view, SslErrorHandler handler, SslError error);

        private long host;
        private final Object hostLock = new Object ();
    }

    static public class ChromeClient extends WebChromeClient
    {
        public ChromeClient (long hostToUse)
        {
            host = hostToUse;
        }

        @Override
        public void onCloseWindow (WebView window)
        {
            webViewCloseWindowRequest (host, window);
        }

        @Override
        public boolean onCreateWindow (WebView view, boolean isDialog,
                                       boolean isUserGesture, Message resultMsg)
        {
            webViewCreateWindowRequest (host, view);
            return false;
        }

        private native void webViewCloseWindowRequest (long host, WebView view);

        private native void webViewCreateWindowRequest (long host, WebView view);

        private long host;
        private final Object hostLock = new Object ();
    }
}
