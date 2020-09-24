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

namespace juce
{

#if JUCE_WEB_BROWSER || DOXYGEN

//==============================================================================
/**
    A component that displays an embedded web browser.

    The browser itself will be platform-dependent. On Mac and iOS it will be
    WebKit, on Android it will be Chrome, and on Linux it will be WebKit.

    On Windows it will be IE, but if JUCE_USE_WIN_WEBVIEW2 is enabled then using
    the WindowsWebView2WebBrowserComponent wrapper instead of this class directly
    will attempt to use the Microsoft Edge (Chromium) WebView2. See the documentation
    of that class for more information about its requirements.

    @tags{GUI}
*/
class JUCE_API  WebBrowserComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a WebBrowserComponent.

        Once it's created and visible, send the browser to a URL using goToURL().

        @param unloadPageWhenBrowserIsHidden  if this is true, then when the browser
                            component is taken offscreen, it'll clear the current page
                            and replace it with a blank page - this can be handy to stop
                            the browser using resources in the background when it's not
                            actually being used.
    */
    explicit WebBrowserComponent (bool unloadPageWhenBrowserIsHidden = true);

    /** Destructor. */
    ~WebBrowserComponent() override;

    //==============================================================================
    /** Sends the browser to a particular URL.

        @param url      the URL to go to.
        @param headers  an optional set of parameters to put in the HTTP header. If
                        you supply this, it should be a set of string in the form
                        "HeaderKey: HeaderValue"
        @param postData an optional block of data that will be attached to the HTTP
                        POST request
    */
    void goToURL (const String& url,
                  const StringArray* headers = nullptr,
                  const MemoryBlock* postData = nullptr);

    /** Stops the current page loading. */
    void stop();

    /** Sends the browser back one page. */
    void goBack();

    /** Sends the browser forward one page. */
    void goForward();

    /** Refreshes the browser. */
    void refresh();

    /** Clear cookies that the OS has stored for the WebComponents of this application */
    static void clearCookies();

    //==============================================================================
    /** This callback is called when the browser is about to navigate
        to a new location.

        You can override this method to perform some action when the user
        tries to go to a particular URL. To allow the operation to carry on,
        return true, or return false to stop the navigation happening.
    */
    virtual bool pageAboutToLoad (const String& newURL)             { ignoreUnused (newURL); return true; }

    /** This callback happens when the browser has finished loading a page. */
    virtual void pageFinishedLoading (const String& url)            { ignoreUnused (url); }

    /** This callback happens when a network error was encountered while
        trying to load a page.

        You can override this method to show some other error page by calling
        goToURL. Return true to allow the browser to carry on to the internal
        browser error page.

        The errorInfo contains some platform dependent string describing the
        error.
    */
    virtual bool pageLoadHadNetworkError (const String& errorInfo)  { ignoreUnused (errorInfo); return true; }

    /** This callback occurs when a script or other activity in the browser asks for
        the window to be closed.
    */
    virtual void windowCloseRequest()                               {}

    /** This callback occurs when the browser attempts to load a URL in a new window.
        This won't actually load the window but gives you a chance to either launch a
        new window yourself or just load the URL into the current window with goToURL().
     */
    virtual void newWindowAttemptingToLoad (const String& newURL)   { ignoreUnused (newURL); }

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    void visibilityChanged() override;
    /** @internal */
    void focusGained (FocusChangeType) override;

    /** @internal */
    class Pimpl;

private:
    //==============================================================================
    friend class WindowsWebView2WebBrowserComponent;

    explicit WebBrowserComponent (bool unloadPageWhenBrowserIsHidden,
                                  const File& dllLocation,
                                  const File& userDataFolder);

    //==============================================================================
    std::unique_ptr<Pimpl> browser;
    bool blankPageShown = false, unloadPageWhenBrowserIsHidden;
    String lastURL;
    StringArray lastHeaders;
    MemoryBlock lastPostData;

    void reloadLastURL();
    void checkWindowAssociation();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComponent)
};

//==============================================================================
/**
    If you have enabled the JUCE_USE_WIN_WEBVIEW2 flag then this wrapper will attempt to
    use the Microsoft Edge (Chromium) WebView2 control instead of IE on Windows. It will
    behave the same as WebBrowserComponent on all other platforms and will fall back to
    IE on Windows if the WebView2 requirements are not met.

    This requires Microsoft Edge (minimum version 82.0.488.0) to be installed at runtime.

    Currently this also requires that WebView2Loader.dll, which can be found in the
    Microsoft.Web.WebView package, is installed at runtime. As this is not a standard
    system DLL, we can't rely on it being found via the normal system DLL search paths.
    Therefore in order to use WebView2 you need to ensure that WebView2Loader.dll is
    installed either to a location covered by the Windows DLL system search paths or
    to the folder specified in the constructor of this class.

    @tags{GUI}
*/
class WindowsWebView2WebBrowserComponent  : public WebBrowserComponent
{
public:
    /** Creates a WebBrowserComponent that is compatible with the WebView2 control
        on Windows.

        This allows you to specify a custom location for the WebView2Loader.dll as
        well as a non-default location for storing user data for the browser instance.

        @param unloadPageWhenBrowserIsHidden  if this is true, then when the browser
                               component is taken offscreen, it'll clear the current page
                               and replace it with a blank page - this can be handy to stop
                               the browser using resources in the background when it's not
                               actually being used.
        @param dllLocation     the path to WebView2Loader.dll, if this is empty then the default
                               system DLL search paths will be used
        @param userDataFolder  a directory in which the WebView2 user data will be stored, if
                               this is empty then a directory will be created next to the
                               executable
    */
    explicit WindowsWebView2WebBrowserComponent (bool unloadPageWhenBrowserIsHidden = true,
                                                 const File& dllLocation = {},
                                                 const File& userDataFolder = {})
        : WebBrowserComponent (unloadPageWhenBrowserIsHidden, dllLocation, userDataFolder)
    {
    }
};

#endif

} // namespace juce
