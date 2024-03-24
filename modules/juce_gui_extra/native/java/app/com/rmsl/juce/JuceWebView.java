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

import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Message;
import android.webkit.WebResourceResponse;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.SslErrorHandler;
import android.webkit.WebChromeClient;


//==============================================================================
public class JuceWebView
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
