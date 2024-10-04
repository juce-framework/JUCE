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

namespace juce
{

#if JUCE_WEB_BROWSER || DOXYGEN

#if JUCE_MAC || JUCE_IOS || (JUCE_WINDOWS && JUCE_USE_WIN_WEBVIEW2) || JUCE_ANDROID || JUCE_LINUX
 #define JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE 1
#else
 #define JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE 0
#endif

class WebBrowserComponent;

/** Type for a listener registered with
    WebBrowserComponent::Options::withWebViewLifetimeListener. This can be useful for
    types using the WebBrowserComponent::Options::withOptionsFrom function as they have to be
    constructed before the WebBrowserComponent.
*/
class WebViewLifetimeListener
{
public:
    virtual ~WebViewLifetimeListener() = default;

    /** Called from the WebBrowserComponent constructor. */
    virtual void webViewConstructed (WebBrowserComponent*) = 0;

    /** Called from the WebBrowserComponent destructor. */
    virtual void webViewDestructed (WebBrowserComponent*) = 0;
};

//==============================================================================
/**
    A component that displays an embedded web browser.

    The browser itself will be platform-dependent. On Mac and iOS it will be
    WebKit, on Android it will be Chrome, and on Linux it will be WebKit.

    The default engine on Windows will be IE, but if JUCE_USE_WIN_WEBVIEW2=1 or
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1 is defined, then passing the
    WebBrowserComponent::Options::Backend::webview2 value to the constructor will
    attempt to use the Chrome based Edge WebView, and fall back to IE in case of
    failure. CMake builds also need to specify the NEEDS_WEBVIEW2 option when
    creating a JUCE based target.

    @tags{GUI}
*/
class JUCE_API  WebBrowserComponent  : public Component
{
public:
    //==============================================================================
    /** Type for a listener registered with Options::withNativeEventListener. */
    using NativeEventListener = std::function<void (var)>;

    /** Type for the completion passed as the second parameter of NativeFunction. Can be called
        from any thread.
    */
    using NativeFunctionCompletion = std::function<void (var)>;

    /** Type for functions registered with Options::withNativeFunction. The first parameter is an
        Array object containing the arguments of the Javascript function invocation.

        The second parameter is the result that completes the Promise returned by the Javascript
        function call. It can be called from any thread.
    */
    using NativeFunction = std::function<void (const Array<var>&, NativeFunctionCompletion)>;

    /** A resource returned by a ResourceProvider.

        @see Options::withResourceProvider
    */
    struct Resource
    {
        std::vector<std::byte> data;
        String mimeType;
    };

    /** The type used in Options::withResourceProvider.
    */
    using ResourceProvider = std::function<std::optional<Resource> (const String&)>;

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
        [[nodiscard]] Options withKeepPageLoadedWhenBrowserIsHidden() const    { return withMember (*this, &Options::keepPageLoadedWhenBrowserIsHidden, true); }

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

            /** Sets a non-default location for storing user data for the browser instance.

                In plugin projects you may find it necessary to use this option and specify a
                location such as File::SpecialLocationType::tempDirectory. Otherwise WebView2
                may function incorrectly due to being denied access to the default user data
                location.
            */
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

                This colour must either be fully opaque or fully transparent.
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

        /** Options specific to the WkWebView backend used on Apple systems. These options will be
            ignored on non-Apple platforms.
        */
        class AppleWkWebView
        {
        public:
            /** Specifies whether the WebView is allowed to access siblings of files specified with
                the file:// URL scheme.

                Allowing this is a potential security vulnerability if you don't have full control
                over the file that you are opening.
            */
            [[nodiscard]] AppleWkWebView withAllowAccessToEnclosingDirectory (bool x) const
            {
                return withMember (*this, &AppleWkWebView::allowAccessToEnclosingDirectory, x);
            }

            /** If this options is specified, the underlying WebView will return NO from its
                acceptsFirstMouse method.

                This disables the click-through behaviour, meaning that clicking a previously
                unfocused application window only makes the window focused, but will not pass on the
                click to whichever control inside the WebView is under the mouse.
            */
            [[nodiscard]] AppleWkWebView withDisabledAcceptsFirstMouse() const
            {
                return withMember (*this, &AppleWkWebView::acceptsFirstMouse, false);
            }

            auto getAllowAccessToEnclosingDirectory() const { return allowAccessToEnclosingDirectory; }
            auto getAcceptsFirstMouse() const { return acceptsFirstMouse; }

        private:
            bool allowAccessToEnclosingDirectory = false;
            bool acceptsFirstMouse = true;
        };

        /** Specifies options that apply to the Windows implementation when the WebView2 feature is
            enabled.

            @see withBackend
        */
        [[nodiscard]] Options withWinWebView2Options (const WinWebView2& winWebView2Options) const
        {
            return withMember (*this, &Options::winWebView2, winWebView2Options);
        }

        /** Specifies options that influence the WebBrowserComponent's behaviour on Apple systems.
        */
        [[nodiscard]] Options withAppleWkWebViewOptions (const AppleWkWebView& appleWkWebViewOptions) const
        {
            return withMember (*this, &Options::appleWkWebView, appleWkWebViewOptions);
        }

        /** Enables native integration features for the code running inside the WebBrowserComponent.

            This injects data and function objects under `window.__JUCE__.backend` through which
            scripts running in the WebBrowserComponent can send events to the backend and call
            registered native functions.

            You should only enable native integrations if you have full control over the content
            loaded into the component. Navigating to 3rd party websites with these integrations
            enabled may expose the application and the computer to security risks.

            @see withNativeFunction, withEventListener
        */
        [[nodiscard]] Options withNativeIntegrationEnabled (bool enabled = true) const
        {
            return withMember (*this, &Options::enableNativeIntegration, enabled);
        }

        /** Registers a NativeFunction under the given name.

            To call this function from the frontend, you can import the JUCE frontend helper module
            or issue a call to the low-level frontend API.

            The callback is always called on the message thread.

            @code
            import { getNativeFunction } from "./juce";

            function someJavascriptFunction() {
              const myBackendFunction = getNativeFunction("myBackendFunction");
              myBackendFunction (1, 2, "some string");
            }
            @endcode
        */
        [[nodiscard]] Options withNativeFunction (const Identifier& name, NativeFunction callback) const
        {
            auto copy = *this;
            jassert (copy.nativeFunctions.count (name) == 0);
            copy.nativeFunctions[name] = std::move (callback);
            return copy;
        }

        /** Registers a NativeEventListener that receives events sent to the specified eventId.

            To send a message to this listener from the frontend, call for example
            @code window.__JUCE__.backend.emitEvent(eventId, { x: 2, y: 6 }); @endcode
        */
        [[nodiscard]] Options withEventListener (const Identifier& eventId, NativeEventListener listener) const
        {
            auto copy = *this;
            copy.eventListeners.emplace_back (eventId, std::move (listener));
            return copy;
        }

        /** Adds a Javascript code that will be evaluated before any other resource is loaded but
            after the JUCE backend definitions become available, hence the specified script can
            rely on the presence of `window.__JUCE__.backend`.

            This script will be evaluated after all goToUrl() calls.
        */
        [[nodiscard]] Options withUserScript (StringRef script) const
        {
            auto copy = *this;
            copy.userScripts.add (script);
            return copy;
        }

        /** Ensures that there will be a Javascript Array under
            `window.__JUCE__.initialisationData.name` and that it will contain the value
            provided here.

            The initialisation data is injected prior to loading any resource. Multiple values added
            for the same name will all be available in the Array.
        */
        [[nodiscard]] Options withInitialisationData (StringRef name, const var& value) const
        {
            auto copy = *this;
            copy.initialisationData.push_back (std::make_pair (String { name }, value));
            return copy;
        }

#if JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE || JUCE_DOXYGEN
        /** Sets a ResourceProvider object that can complete WebView resource requests and return
            data without having to issue a network operation.

            Requests sent to WebBrowserComponent::getResourceProviderRoot() + "resource.path" will
            invoke the provider with the path "/resource.path".

            If you call WebBrowserComponent::goToURL with the value returned by
            WebBrowserComponent::getResourceProviderRoot, your resource provider will receive a
            request for the resource "/" for which you will typically want to return the contents of
            your index.html.

            You can also specify an optional allowedOriginIn parameter that will make your
            ResourceProvider available to scripts loaded from that origin. E.g. if you specify
            "http://localhost:3000", then a script loaded from such a local development server will
            be able to access resources such as getResourceProviderRoot() + "live_data.bin".

            Allowing external origins is handy for development, but is a potential security risk in
            publicly released binaries.
        */
        [[nodiscard]] Options withResourceProvider (ResourceProvider provider,
                                                    std::optional<String> allowedOriginIn = std::nullopt) const
        {
            return withMember (withMember (*this, &Options::resourceProvider, std::move (provider)),
                               &Options::allowedOrigin,
                               std::move (allowedOriginIn));
        }
#endif

        /** Adds an object that will be notified when the WebBrowserComponent is constructed and
            destructed.
        */
        [[nodiscard]] Options withWebViewLifetimeListener (WebViewLifetimeListener* listener)
        {
            auto copy = *this;
            copy.lifetimeListeners.push_back (listener);
            return copy;
        }

        /** Adds all options provided by the builder to the returned Options object.
        */
        template <typename OptionsType>
        OptionsType withOptionsFrom (OptionsBuilder<OptionsType>& builder) const
        {
            return builder.buildOptions (static_cast<const OptionsType&> (*this));
        }

        //==============================================================================
        auto        getBackend() const noexcept                          { return browserBackend; }
        auto        keepsPageLoadedWhenBrowserIsHidden() const noexcept  { return keepPageLoadedWhenBrowserIsHidden; }
        auto        getUserAgent() const                                 { return userAgent; }
        auto        getWinWebView2BackendOptions() const                 { return winWebView2; }
        auto        getAppleWkWebViewOptions() const                     { return appleWkWebView; }
        auto        getNativeIntegrationsEnabled() const                 { return enableNativeIntegration; }
        const auto& getNativeFunctions() const                           { return nativeFunctions; }
        const auto& getEventListeners() const                            { return eventListeners; }
        const auto& getUserScripts() const                               { return userScripts; }
        const auto& getInitialisationData() const                        { return initialisationData; }
        auto        getResourceProvider() const                          { return resourceProvider; }
        const auto& getAllowedOrigin() const                             { return allowedOrigin; }
        const auto& getLifetimeListeners() const                         { return lifetimeListeners; }

    private:
        //==============================================================================
        Backend browserBackend = Backend::defaultBackend;
        bool keepPageLoadedWhenBrowserIsHidden = false;
        bool enableNativeIntegration = false;
        String userAgent;
        WinWebView2 winWebView2;
        AppleWkWebView appleWkWebView;
        std::map<Identifier, NativeFunction> nativeFunctions;
        std::vector<std::pair<Identifier, NativeEventListener>> eventListeners;
        StringArray userScripts;
        std::vector<std::pair<String, var>> initialisationData;
        ResourceProvider resourceProvider;
        std::optional<String> allowedOrigin;
        std::vector<WebViewLifetimeListener*> lifetimeListeners;
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

    /** Returns a platform specific string that represents the root address for resources served
        by the ResourceProvider.

        If you pass this value to goToURL the ResourceProvider will receive a request with the "/"
        path parameter. In response to this request the provider may typically want to return the
        contents of the "index.html" file.

        @see ResourceProvider, Options::withResourceProvider, goToURL
    */
    static const String& getResourceProviderRoot();

    //==============================================================================
    /** On MacOS, iOS and Linux getResult will return a nullptr if the evaluation failed. In this
        case getError will return a non-null Error result, which contains more information about
        why the evaluation failed. It could be e.g. a syntax error or referencing an undefined
        object.

        On Windows and Android getResult will always return a valid pointer to a variant, and
        getError will always return a nullptr. In case there was an evaluation failure, getResult
        returns a void variant, which is indistinguishable from a successful evaluation that yielded
        a null result. Unfortunately these platforms don't offer a way to detect evaluation errors.
    */
    class EvaluationResult
    {
    public:
        /** A simple error type class. */
        struct Error
        {
            /** Error type. */
            enum class Type
            {
                /** Error occurring for a reason unknown to us. */
                unknown,

                /** Error occurring because of a Javascript exception being thrown. */
                javascriptException,

                /** Error occurring because the returned result cannot be serialised into a native
                    type e.g. Promise.
                */
                unsupportedReturnType
            };

            Type type;
            String message;
        };

        EvaluationResult (const var& result) : value { result }
        {
        }

        EvaluationResult (const Error& error) : value { error }
        {
        }

        const var* getResult() const
        {
            return std::get_if<var> (&value);
        }

        const Error* getError() const
        {
            return std::get_if<Error> (&value);
        }

    private:
        std::variant<var, Error> value;
    };

    /** Callback type that can be passed optionally to evaluateJavascript. */
    using EvaluationCallback = std::function<void (EvaluationResult)>;

    /** Evaluates the specified script in the context of the current state of the
        WebBrowserComponent.

        If the optional callback is provided it will be called with the result of the evaluation.
        The callback will be called on the message thread.
    */
    void evaluateJavascript (const String& script, EvaluationCallback callback = nullptr);

    /** Emits an object on the frontend under the specified eventId.

        Ids beginning with `__juce` are reserved for the framework implementation.

        Example for listening to such events on the frontend:
        @code
        // Subscribing
        const removalToken = window.__JUCE__.backend.addEventListener(eventId, (objectFromBackend) => {
            console.log(objectFromBackend.message);
        });

        // Unsubscribing
        window.__JUCE__.backend.removeEventListener(removalToken);
        @endcode
    */
    void emitEventIfBrowserIsVisible (const Identifier& eventId, const var& object);

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
    class Impl;

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
    }

    std::unique_ptr<Impl> impl;
    bool blankPageShown = false, unloadPageWhenHidden;
    String lastURL;
    StringArray lastHeaders;
    MemoryBlock lastPostData;
    ListenerList<WebViewLifetimeListener> lifetimeListeners;

    void reloadLastURL();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComponent)
};

#endif

} // namespace juce
