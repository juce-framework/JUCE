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

import android.content.Context;
import android.graphics.Bitmap;
import android.net.http.SslError;
import android.os.Build;
import android.os.Message;
import android.webkit.JavascriptInterface;
import android.webkit.ValueCallback;
import android.webkit.WebResourceResponse;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.webkit.SslErrorHandler;
import android.webkit.WebChromeClient;

import java.lang.annotation.Native;
import java.util.ArrayList;

//==============================================================================
public class JuceWebViewClasses
{
    static public class NativeInterface
    {
        private long host;
        private final Object hostLock = new Object ();

        //==============================================================================
        public NativeInterface (long hostToUse)
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

        //==============================================================================
        public void onPageStarted (WebView view, String url)
        {
            if (host == 0)
                return;

            handleResourceRequest (host, view, url);
        }

        public WebResourceResponse shouldInterceptRequest (WebView view, WebResourceRequest request)
        {
            synchronized (hostLock)
            {
                if (host != 0)
                {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                        return handleResourceRequest (host, view, request.getUrl().toString());
                }
            }

            return null;
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

        public void onReceivedHttpError (WebView view, WebResourceRequest request, WebResourceResponse errorResponse)
        {
            if (host == 0)
                return;

            webViewReceivedHttpError (host, view, request, errorResponse);
        }

        public void onCloseWindow (WebView window)
        {
            if (host == 0)
                return;

            webViewCloseWindowRequest (host, window);
        }

        public boolean onCreateWindow (WebView view, boolean isDialog,
                                       boolean isUserGesture, Message resultMsg)
        {
            if (host == 0)
                return false;

            webViewCreateWindowRequest (host, view);
            return false;
        }

        public String postMessageHandler (String message)
        {
            synchronized (hostLock)
            {
                if (host != 0)
                {
                    String result = postMessage (host, message);

                    if (result == null)
                        return "";

                    return result;
                }
            }

            return "";
        }

        public void handleJavascriptEvaluationResult (long evalId, String result)
        {
            if (host == 0)
                return;

            evaluationResultHandler (host, evalId, result);
        }

        public boolean shouldOverrideUrlLoading (String url)
        {
            if (host == 0)
                return false;

            return ! pageAboutToLoad (host, url);
        }

        //==============================================================================
        private native WebResourceResponse handleResourceRequest (long host, WebView view, String url);
        private native void webViewPageLoadFinished (long host, WebView view, String url);
        private native void webViewReceivedSslError (long host, WebView view, SslErrorHandler handler, SslError error);
        private native void webViewReceivedHttpError (long host, WebView view, WebResourceRequest request, WebResourceResponse errorResponse);

        private native void webViewCloseWindowRequest (long host, WebView view);
        private native void webViewCreateWindowRequest (long host, WebView view);

        private native void evaluationResultHandler (long host, long evalId, String result);
        private native String postMessage (long host, String message);

        private native boolean pageAboutToLoad (long host, String url);
    }

    static public class Client extends WebViewClient
    {
        private NativeInterface nativeInterface;

        //==============================================================================
        public Client (NativeInterface nativeInterfaceIn)
        {
            nativeInterface = nativeInterfaceIn;
        }

        //==============================================================================
        @Override
        public void onPageFinished (WebView view, String url)
        {
            nativeInterface.onPageFinished (view, url);
        }

        @Override
        public void onReceivedSslError (WebView view, SslErrorHandler handler, SslError error)
        {
            nativeInterface.onReceivedSslError (view, handler, error);
        }

        @Override
        public void onReceivedHttpError (WebView view, WebResourceRequest request, WebResourceResponse errorResponse)
        {
            nativeInterface.onReceivedHttpError (view, request, errorResponse);
        }

        @Override
        public void onPageStarted (WebView view, String url, Bitmap favicon)
        {
            nativeInterface.onPageStarted (view, url);
        }

        @Override
        public WebResourceResponse shouldInterceptRequest (WebView view, WebResourceRequest request)
        {
            return nativeInterface.shouldInterceptRequest (view, request);
        }

        @Override
        public boolean shouldOverrideUrlLoading (WebView view, WebResourceRequest request)
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                return nativeInterface.shouldOverrideUrlLoading (request.getUrl().toString());

            return false;
        }

        @Override
        public boolean shouldOverrideUrlLoading (WebView view, String url)
        {
            return nativeInterface.shouldOverrideUrlLoading (url);
        }
    }

    static public class ChromeClient extends WebChromeClient
    {
        private NativeInterface nativeInterface;

        //==============================================================================
        public ChromeClient (NativeInterface nativeInterfaceIn)
        {
            nativeInterface = nativeInterfaceIn;
        }

        //==============================================================================
        @Override
        public void onCloseWindow (WebView window)
        {
            nativeInterface.onCloseWindow (window);
        }

        @Override
        public boolean onCreateWindow (WebView view, boolean isDialog,
                                       boolean isUserGesture, Message resultMsg)
        {
            return nativeInterface.onCreateWindow (view, isDialog, isUserGesture, resultMsg);
        }
    }

    static public class WebAppInterface
    {
        private NativeInterface nativeInterface;
        String userScripts;

        //==============================================================================
        public WebAppInterface (NativeInterface nativeInterfaceIn, String userScriptsIn)
        {
            nativeInterface = nativeInterfaceIn;
            userScripts = userScriptsIn;
        }

        //==============================================================================
        @JavascriptInterface
        public String postMessage (String message)
        {
            return nativeInterface.postMessageHandler (message);
        }

        @JavascriptInterface
        public String getAndroidUserScripts()
        {
            return userScripts;
        }
    }

    static public class JuceWebView extends WebView
    {
        private NativeInterface nativeInterface;
        private long evaluationId = 0;

        public JuceWebView (Context context, long host, String userAgent, String initScripts)
        {
            super (context);

            nativeInterface = new NativeInterface (host);

            WebSettings settings = getSettings();
            settings.setJavaScriptEnabled (true);
            settings.setBuiltInZoomControls (true);
            settings.setDisplayZoomControls (false);
            settings.setSupportMultipleWindows (true);
            settings.setUserAgentString (userAgent);

            setWebChromeClient (new ChromeClient (nativeInterface));

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
            {
                setWebContentsDebuggingEnabled (true);
            }

            setWebViewClient (new Client (nativeInterface));
            addJavascriptInterface (new WebAppInterface (nativeInterface, initScripts), "__JUCE__");
        }

        public void disconnectNative()
        {
            nativeInterface.hostDeleted();
        }

        public long evaluateJavascript (String script)
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
            {
                final long currentEvaluationId = evaluationId++;
                final NativeInterface accessibleNativeInterface = nativeInterface;

                super.evaluateJavascript (script, new ValueCallback<String>()
                                                  {
                                                      @Override
                                                      public void onReceiveValue(String result)
                                                      {
                                                          accessibleNativeInterface.handleJavascriptEvaluationResult (currentEvaluationId,
                                                                                                                      result);
                                                      }
                                                  });

                return currentEvaluationId;
            }

            return -1;
        }
    }
}
