/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
    /**
        Options to configure WebBrowserComponent.
    */
    class JUCE_API Options
    {
    public:
        //==============================================================================
        enum class Backend
        {
            /**
                Default web browser backend. WebKit will be used on macOS, gtk-webkit2 on Linux and internet
                explorer on Windows. On Windows, the default may change to webview2 in the fututre.
            */
            defaultBackend,

            /**
                Use Internet Explorer as the backend on Windows. By default, IE will use an ancient version
                of IE. To change this behaviour, you either need to add the following html element into your page's
                head section:

                    <meta http-equiv="X-UA-Compatible" content="IE=edge" />

                or you need to change windows registry values for your application.  More information on the latter
                can be found here:

                https://learn.microsoft.com/en-us/previous-versions/windows/internet-explorer/ie-developer/general-info/ee330730(v=vs.85)?redirectedfrom=MSDN#browser-emulation
            */
            ie,

            /**
                Use the chromium based  WebView2 engine on Windows
            */
            webview2
        };

        /**
            Use a particular backend to create the WebViewBrowserComponent. JUCE will silently
            fallback to the default backend if the selected backend is not supported. To check
            if a specific backend is supported on your platform or not, use
            WebBrowserComponent::areOptionsSupported.
        */
        [[nodiscard]] Options withBackend (Backend backend) const              { return withMember (*this, &Options::browserBackend, backend); }

        //==============================================================================
        /**
            Tell JUCE to keep the web page alive when the WebBrowserComponent is not visible.
            By default, JUCE will replace the current page with a blank page - this can be
            handy to stop the browser using resources in the background when it's not
            actually being used.
        */
        [[nodiscard]] Options withKeepPageLoadedWhenBrowserIsHidden () const   { return withMember (*this, &Options::keepPageLoadedWhenBrowserIsHidden, true); }

        /**
            Use a specific user agent string when requesting web pages.
        */
        [[nodiscard]] Options withUserAgent (String ua) const                  { return withMember (*this, &Options::userAgent, std::move (ua)); }

        //==============================================================================
        /** Options specific to the WebView2 backend. These options will be ignored
            if another backend is used.
        */
        class WinWebView2
        {
        public:
            //==============================================================================
            /** Sets a custom location for the WebView2Loader.dll that is not a part of the
                standard system DLL search paths.
            */
            [[nodiscard]] WinWebView2 withDLLLocation (const File& location) const   { return withMember (*this, &WinWebView2::dllLocation, location); }

            /** Sets a non-default location for storing user data for the browser instance. */
            [[nodiscard]] WinWebView2 withUserDataFolder (const File& folder) const  { return withMember (*this, &WinWebView2::userDataFolder, folder); }

            /** If this is set, the status bar usually displayed in the lower-left of the webview
                will be disabled.
            */
            [[nodiscard]] WinWebView2 withStatusBarDisabled() const                  { return withMember (*this, &WinWebView2::disableStatusBar, true); }

            /** If this is set, a blank page will be displayed on error instead of the default
                built-in error page.
            */
            [[nodiscard]] WinWebView2 withBuiltInErrorPageDisabled() const           { return withMember (*this, &WinWebView2::disableBuiltInErrorPage, true); }

            /** Sets the background colour that WebView2 renders underneath all web content.

                This colour must either be fully opaque or transparent. On Windows 7 this
                colour must be opaque.
            */
            [[nodiscard]] WinWebView2 withBackgroundColour (const Colour& colour) const
            {
                // the background colour must be either fully opaque or transparent!
                jassert (colour.isOpaque() || colour.isTransparent());

                return withMember (*this, &WinWebView2::backgroundColour, colour);
            }

            //==============================================================================
            File getDLLLocation() const                          { return dllLocation; }
            File getUserDataFolder() const                       { return userDataFolder; }
            bool getIsStatusBarDisabled() const noexcept         { return disableStatusBar; }
            bool getIsBuiltInErrorPageDisabled() const noexcept  { return disableBuiltInErrorPage; }
            Colour getBackgroundColour() const                   { return backgroundColour; }

        private:
            //==============================================================================
            File dllLocation, userDataFolder;
            bool disableStatusBar = false, disableBuiltInErrorPage = false;
            Colour backgroundColour;
        };

        [[nodiscard]] Options withWinWebView2Options (const WinWebView2& winWebView2Options) const
        {
            return withMember (*this, &Options::winWebView2, winWebView2Options);
        }

        //==============================================================================
        Backend getBackend() const noexcept                       { return browserBackend; }
        bool keepsPageLoadedWhenBrowserIsHidden() const noexcept  { return keepPageLoadedWhenBrowserIsHidden; }
        String getUserAgent() const                               { return userAgent; }
        WinWebView2 getWinWebView2BackendOptions() const          { return winWebView2; }

    private:
        //==============================================================================
        Backend browserBackend = Backend::defaultBackend;
        bool keepPageLoadedWhenBrowserIsHidden = false;
        String userAgent;
        WinWebView2 winWebView2;
    };

    //==============================================================================
    /** Creates a WebBrowserComponent with default options*/
    WebBrowserComponent() : WebBrowserComponent (Options {}) {}

    /** Creates a WebBrowserComponent.

        Once it's created and visible, send the browser to a URL using goToURL().
    */
    explicit WebBrowserComponent (const Options& options);

    /** Destructor. */
    ~WebBrowserComponent() override;

    //==============================================================================
    /** Check if the specified options are supported on this platform. */
    static bool areOptionsSupported (const Options& options);

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
    virtual bool pageAboutToLoad (const String& newURL);

    /** This callback happens when the browser has finished loading a page. */
    virtual void pageFinishedLoading (const String& url);

    /** This callback happens when a network error was encountered while
        trying to load a page.

        You can override this method to show some other error page by calling
        goToURL. Return true to allow the browser to carry on to the internal
        browser error page.

        The errorInfo contains some platform dependent string describing the
        error.
    */
    virtual bool pageLoadHadNetworkError (const String& errorInfo);

    /** This callback occurs when a script or other activity in the browser asks for
        the window to be closed.
    */
    virtual void windowCloseRequest();

    /** This callback occurs when the browser attempts to load a URL in a new window.
        This won't actually load the window but gives you a chance to either launch a
        new window yourself or just load the URL into the current window with goToURL().
     */
    virtual void newWindowAttemptingToLoad (const String& newURL);

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
    void focusGainedWithDirection (FocusChangeType, FocusChangeDirection) override;

    /** @internal */
    class Pimpl;

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
    }

    //==============================================================================
    std::unique_ptr<Pimpl> browser;
    bool blankPageShown = false, unloadPageWhenHidden;
    String lastURL;
    StringArray lastHeaders;
    MemoryBlock lastPostData;

    void reloadLastURL();
    void checkWindowAssociation();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComponent)
};

#endif

} // namespace juce
