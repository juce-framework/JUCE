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

#if (defined (MAC_OS_X_VERSION_10_11) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_11) \
   || (defined (__IPHONE_8_0) && __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_8_0)

 #define JUCE_USE_WKWEBVIEW 1

 #if (defined (MAC_OS_X_VERSION_10_12) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12)
  #define WKWEBVIEW_OPENPANEL_SUPPORTED 1
 #endif

#endif

NSMutableURLRequest* getRequestForURL (const String& url, const StringArray* headers, const MemoryBlock* postData)
{
    NSString* urlString = juceStringToNS (url);

    #if (JUCE_MAC && (defined (MAC_OS_X_VERSION_10_9) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_9)) \
      || (JUCE_IOS && (defined (__IPHONE_7_0) && __IPHONE_OS_VERSION_MIN_REQUIRED >= __IPHONE_7_0))
     urlString = [urlString stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]];
    #else
     urlString = [urlString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    #endif

     if (NSURL* nsURL = [NSURL URLWithString: urlString])
     {
         NSMutableURLRequest* r
             = [NSMutableURLRequest requestWithURL: nsURL
                                       cachePolicy: NSURLRequestUseProtocolCachePolicy
                                   timeoutInterval: 30.0];

         if (postData != nullptr && postData->getSize() > 0)
         {
             [r setHTTPMethod: nsStringLiteral ("POST")];
             [r setHTTPBody: [NSData dataWithBytes: postData->getData()
                                            length: postData->getSize()]];
         }

         if (headers != nullptr)
         {
             for (int i = 0; i < headers->size(); ++i)
             {
                 auto headerName  = (*headers)[i].upToFirstOccurrenceOf (":", false, false).trim();
                 auto headerValue = (*headers)[i].fromFirstOccurrenceOf (":", false, false).trim();

                 [r setValue: juceStringToNS (headerValue)
                    forHTTPHeaderField: juceStringToNS (headerName)];
             }
         }

         return r;
     }

    return nullptr;
}

#if JUCE_USE_WKWEBVIEW

struct WebViewDelegateClass  : public ObjCClass<NSObject>
{
    WebViewDelegateClass()  : ObjCClass<NSObject> ("JUCEWebViewDelegate_")
    {
        addIvar<WebBrowserComponent*> ("owner");

        addMethod (@selector (webView:decidePolicyForNavigationAction:decisionHandler:),  decidePolicyForNavigationAction, "v@:@@@");
        addMethod (@selector (webView:didFinishNavigation:),                              didFinishNavigation,             "v@:@@");
        addMethod (@selector (webView:didFailNavigation:withError:),                      didFailNavigation,               "v@:@@@");
        addMethod (@selector (webView:didFailProvisionalNavigation:withError:),           didFailProvisionalNavigation,    "v@:@@@");

        addMethod (@selector (webView:webViewDidClose:),                                  webViewDidClose,                 "v@:@");
        addMethod (@selector (webView:createWebViewWithConfiguration:forNavigationAction:
                              windowFeatures:),                                           createWebView,                   "@@:@@@@");

       #if WKWEBVIEW_OPENPANEL_SUPPORTED
        addMethod (@selector (webView:runOpenPanelWithParameters:
                              initiatedByFrame:completionHandler:),                      runOpenPanel,                    "v@:@@@@");
       #endif

        registerClass();
    }

    static void setOwner (id self, WebBrowserComponent* owner)   { object_setInstanceVariable (self, "owner", owner); }
    static WebBrowserComponent* getOwner (id self)               { return getIvar<WebBrowserComponent*> (self, "owner"); }

private:
    static void decidePolicyForNavigationAction (id self, SEL, WKWebView*, WKNavigationAction* navigationAction,
                                                 void (^decisionHandler)(WKNavigationActionPolicy))
    {
        if (getOwner (self)->pageAboutToLoad (nsStringToJuce ([[[navigationAction request] URL] absoluteString])))
            decisionHandler (WKNavigationActionPolicyAllow);
        else
            decisionHandler (WKNavigationActionPolicyCancel);
    }

    static void didFinishNavigation (id self, SEL, WKWebView* webview, WKNavigation*)
    {
        getOwner (self)->pageFinishedLoading (nsStringToJuce ([[webview URL] absoluteString]));
    }

    static void displayError (WebBrowserComponent* owner, NSError* error)
    {
        if ([error code] != NSURLErrorCancelled)
        {
            auto errorString = nsStringToJuce ([error localizedDescription]);
            bool proceedToErrorPage = owner->pageLoadHadNetworkError (errorString);

            // WKWebView doesn't have an internal error page, so make a really simple one ourselves
            if (proceedToErrorPage)
                owner->goToURL ("data:text/plain;charset=UTF-8," + errorString);
        }
    }

    static void didFailNavigation (id self, SEL, WKWebView*, WKNavigation*, NSError* error)
    {
        displayError (getOwner (self), error);
    }

    static void didFailProvisionalNavigation (id self, SEL, WKWebView*, WKNavigation*, NSError* error)
    {
        displayError (getOwner (self), error);
    }

    static void webViewDidClose (id self, SEL, WKWebView*)
    {
        getOwner (self)->windowCloseRequest();
    }

    static WKWebView* createWebView (id self, SEL, WKWebView*, WKWebViewConfiguration*,
                                     WKNavigationAction* navigationAction, WKWindowFeatures*)
    {
        getOwner (self)->newWindowAttemptingToLoad (nsStringToJuce ([[[navigationAction request] URL] absoluteString]));
        return nil;
    }

   #if WKWEBVIEW_OPENPANEL_SUPPORTED
    static void runOpenPanel (id, SEL, WKWebView*, WKOpenPanelParameters* parameters, WKFrameInfo*,
                              void (^completionHandler)(NSArray<NSURL*>*))
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        FileChooser chooser (TRANS("Select the file you want to upload..."),
                             File::getSpecialLocation (File::userHomeDirectory), "*");

        auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles
                    | ([parameters allowsMultipleSelection] ? FileBrowserComponent::canSelectMultipleItems : 0);

        #if (defined (MAC_OS_X_VERSION_10_14) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_14)
         if ([parameters allowsDirectories])
             flags |= FileBrowserComponent::canSelectDirectories;
        #endif

        if (chooser.showDialog (flags, nullptr))
        {
            auto results = chooser.getResults();
            auto urls = [NSMutableArray arrayWithCapacity: (NSUInteger) results.size()];

            for (auto& f : results)
                [urls addObject: [NSURL fileURLWithPath: juceStringToNS (f.getFullPathName())]];

            completionHandler (urls);
        }
        else
        {
            completionHandler (nil);
        }
       #else
        ignoreUnused (parameters, completionHandler);
        jassertfalse; // Can't use this without modal loops being enabled!
       #endif
    }
   #endif
};

//==============================================================================
class WebBrowserComponent::Pimpl
                                   #if JUCE_MAC
                                    : public NSViewComponent
                                   #else
                                    : public UIViewComponent
                                   #endif
{
public:
    Pimpl (WebBrowserComponent* owner)
    {
        ignoreUnused (owner);

        WKWebViewConfiguration* config = [[WKWebViewConfiguration alloc] init];

       #if JUCE_MAC
        auto frame = NSMakeRect (0, 0, 100.0f, 100.0f);
       #else
        auto frame = CGRectMake (0, 0, 100.0f, 100.0f);
       #endif

        webView = [[WKWebView alloc] initWithFrame: frame
                                     configuration: config];

        static WebViewDelegateClass cls;
        webViewDelegate = [cls.createInstance() init];
        WebViewDelegateClass::setOwner (webViewDelegate, owner);

        [webView setNavigationDelegate: webViewDelegate];
        [webView setUIDelegate:         webViewDelegate];

        setView (webView);
    }

    ~Pimpl()
    {
        [webView setNavigationDelegate: nil];
        [webView setUIDelegate:         nil];

        [webViewDelegate release];

        setView (nil);
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        stop();

        if (url.trimStart().startsWithIgnoreCase ("javascript:"))
        {
            [webView evaluateJavaScript: juceStringToNS (url.fromFirstOccurrenceOf (":", false, false))
                     completionHandler: nil];
        }
        else if (NSMutableURLRequest* request = getRequestForURL (url, headers, postData))
        {
            [webView loadRequest: request];
        }
    }

    void goBack()       { [webView goBack]; }
    void goForward()    { [webView goForward]; }

    void stop()         { [webView stopLoading]; }
    void refresh()      { [webView reload]; }

private:
    WKWebView* webView = nil;
    id webViewDelegate;
};

#else

#if JUCE_MAC

struct WebViewKeyEquivalentResponder : public ObjCClass<WebView>
{
    WebViewKeyEquivalentResponder() : ObjCClass<WebView> ("WebViewKeyEquivalentResponder_")
    {
        addMethod (@selector (performKeyEquivalent:), performKeyEquivalent, @encode (BOOL), "@:@");
        registerClass();
    }

private:
    static BOOL performKeyEquivalent (id self, SEL selector, NSEvent* event)
    {
        NSResponder* first = [[self window] firstResponder];

        if (([event modifierFlags] & NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask)
        {
            if ([[event charactersIgnoringModifiers] isEqualToString:@"x"]) return [NSApp sendAction:@selector(cut:)       to:first from:self];
            if ([[event charactersIgnoringModifiers] isEqualToString:@"c"]) return [NSApp sendAction:@selector(copy:)      to:first from:self];
            if ([[event charactersIgnoringModifiers] isEqualToString:@"v"]) return [NSApp sendAction:@selector(paste:)     to:first from:self];
            if ([[event charactersIgnoringModifiers] isEqualToString:@"a"]) return [NSApp sendAction:@selector(selectAll:) to:first from:self];
        }

        objc_super s = { self, [WebView class] };
        return ObjCMsgSendSuper<BOOL, NSEvent*> (&s, selector, event);
    }
};

struct DownloadClickDetectorClass  : public ObjCClass<NSObject>
{
    DownloadClickDetectorClass()  : ObjCClass<NSObject> ("JUCEWebClickDetector_")
    {
        addIvar<WebBrowserComponent*> ("owner");

        addMethod (@selector (webView:decidePolicyForNavigationAction:request:frame:decisionListener:),
                   decidePolicyForNavigationAction, "v@:@@@@@");
        addMethod (@selector (webView:decidePolicyForNewWindowAction:request:newFrameName:decisionListener:),
                   decidePolicyForNewWindowAction, "v@:@@@@@");
        addMethod (@selector (webView:didFinishLoadForFrame:), didFinishLoadForFrame, "v@:@@");
        addMethod (@selector (webView:didFailLoadWithError:forFrame:),  didFailLoadWithError,  "v@:@@@");
        addMethod (@selector (webView:didFailProvisionalLoadWithError:forFrame:),  didFailLoadWithError,  "v@:@@@");
        addMethod (@selector (webView:willCloseFrame:), willCloseFrame, "v@:@@");
        addMethod (@selector (webView:runOpenPanelForFileButtonWithResultListener:allowMultipleFiles:), runOpenPanel, "v@:@@", @encode (BOOL));

        registerClass();
    }

    static void setOwner (id self, WebBrowserComponent* owner)   { object_setInstanceVariable (self, "owner", owner); }
    static WebBrowserComponent* getOwner (id self)               { return getIvar<WebBrowserComponent*> (self, "owner"); }

private:
    static String getOriginalURL (NSDictionary* actionInformation)
    {
        if (NSURL* url = [actionInformation valueForKey: nsStringLiteral ("WebActionOriginalURLKey")])
            return nsStringToJuce ([url absoluteString]);

        return {};
    }

    static void decidePolicyForNavigationAction (id self, SEL, WebView*, NSDictionary* actionInformation,
                                                 NSURLRequest*, WebFrame*, id<WebPolicyDecisionListener> listener)
    {
        if (getOwner (self)->pageAboutToLoad (getOriginalURL (actionInformation)))
            [listener use];
        else
            [listener ignore];
    }

    static void decidePolicyForNewWindowAction (id self, SEL, WebView*, NSDictionary* actionInformation,
                                                NSURLRequest*, NSString*, id<WebPolicyDecisionListener> listener)
    {
        getOwner (self)->newWindowAttemptingToLoad (getOriginalURL (actionInformation));
        [listener ignore];
    }

    static void didFinishLoadForFrame (id self, SEL, WebView* sender, WebFrame* frame)
    {
        if ([frame isEqual: [sender mainFrame]])
        {
            NSURL* url = [[[frame dataSource] request] URL];
            getOwner (self)->pageFinishedLoading (nsStringToJuce ([url absoluteString]));
        }
    }

    static void didFailLoadWithError (id self, SEL, WebView* sender, NSError* error, WebFrame* frame)
    {
        if ([frame isEqual: [sender mainFrame]] && error != nullptr && [error code] != NSURLErrorCancelled)
        {
            auto errorString = nsStringToJuce ([error localizedDescription]);
            bool proceedToErrorPage = getOwner (self)->pageLoadHadNetworkError (errorString);

            // WebKit doesn't have an internal error page, so make a really simple one ourselves
            if (proceedToErrorPage)
                getOwner (self)->goToURL ("data:text/plain;charset=UTF-8," + errorString);
        }
    }

    static void willCloseFrame (id self, SEL, WebView*, WebFrame*)
    {
        getOwner (self)->windowCloseRequest();
    }

    static void runOpenPanel (id, SEL, WebView*, id<WebOpenPanelResultListener> resultListener, BOOL allowMultipleFiles)
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        FileChooser chooser (TRANS("Select the file you want to upload..."),
                             File::getSpecialLocation (File::userHomeDirectory), "*");

        if (allowMultipleFiles ? chooser.browseForMultipleFilesToOpen()
                               : chooser.browseForFileToOpen())
        {
            for (auto& f : chooser.getResults())
                [resultListener chooseFilename: juceStringToNS (f.getFullPathName())];
        }
       #else
        ignoreUnused (resultListener, allowMultipleFiles);
        jassertfalse; // Can't use this without modal loops being enabled!
       #endif
    }
};

#else

struct WebViewDelegateClass  : public ObjCClass<NSObject>
{
    WebViewDelegateClass()  : ObjCClass<NSObject> ("JUCEWebViewDelegate_")
    {
        addIvar<WebBrowserComponent*> ("owner");

        addMethod (@selector (gestureRecognizer:shouldRecognizeSimultaneouslyWithGestureRecognizer:),
                   shouldRecognizeSimultaneouslyWithGestureRecognizer, "c@:@@");

        addMethod (@selector (webView:shouldStartLoadWithRequest:navigationType:),  shouldStartLoadWithRequest, "c@:@@@");
        addMethod (@selector (webViewDidFinishLoad:),                               webViewDidFinishLoad,       "v@:@");

        registerClass();
    }

    static void setOwner (id self, WebBrowserComponent* owner)   { object_setInstanceVariable (self, "owner", owner); }
    static WebBrowserComponent* getOwner (id self)               { return getIvar<WebBrowserComponent*> (self, "owner"); }

private:
    static BOOL shouldRecognizeSimultaneouslyWithGestureRecognizer (id, SEL, UIGestureRecognizer*, UIGestureRecognizer*)
    {
        return YES;
    }

    static BOOL shouldStartLoadWithRequest (id self, SEL, UIWebView*, NSURLRequest* request, UIWebViewNavigationType)
    {
        return getOwner (self)->pageAboutToLoad (nsStringToJuce ([[request URL] absoluteString]));
    }

    static void webViewDidFinishLoad (id self, SEL, UIWebView* webView)
    {
        getOwner (self)->pageFinishedLoading (nsStringToJuce ([[[webView request] URL] absoluteString]));
    }
};

#endif

//==============================================================================
class WebBrowserComponent::Pimpl
                                   #if JUCE_MAC
                                    : public NSViewComponent
                                   #else
                                    : public UIViewComponent
                                   #endif
{
public:
    Pimpl (WebBrowserComponent* owner)
    {
       #if JUCE_MAC
        static WebViewKeyEquivalentResponder webviewClass;
        webView = (WebView*) webviewClass.createInstance();

        webView = [webView initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                               frameName: nsEmptyString()
                               groupName: nsEmptyString()];

        static DownloadClickDetectorClass cls;
        clickListener = [cls.createInstance() init];
        DownloadClickDetectorClass::setOwner (clickListener, owner);

        [webView setPolicyDelegate:    clickListener];
        [webView setFrameLoadDelegate: clickListener];
        [webView setUIDelegate:        clickListener];
       #else
        webView = [[UIWebView alloc] initWithFrame: CGRectMake (0, 0, 1.0f, 1.0f)];

        static WebViewDelegateClass cls;
        webViewDelegate = [cls.createInstance() init];
        WebViewDelegateClass::setOwner (webViewDelegate, owner);

        [webView setDelegate: webViewDelegate];
       #endif

        setView (webView);
    }

    ~Pimpl()
    {
       #if JUCE_MAC
        [webView setPolicyDelegate:    nil];
        [webView setFrameLoadDelegate: nil];
        [webView setUIDelegate:        nil];

        [clickListener release];
       #else
        [webView setDelegate: nil];
        [webViewDelegate release];
       #endif

        setView (nil);
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        stop();

        if (url.trimStart().startsWithIgnoreCase ("javascript:"))
        {
            [webView stringByEvaluatingJavaScriptFromString:
                juceStringToNS (url.fromFirstOccurrenceOf (":", false, false))];
        }
        else if (NSMutableURLRequest* request = getRequestForURL (url, headers, postData))
        {
           #if JUCE_MAC
            [[webView mainFrame] loadRequest: request];
           #else
            [webView loadRequest: request];
           #endif

           #if JUCE_IOS
            [webView setScalesPageToFit: YES];
           #endif
        }
    }

    void goBack()       { [webView goBack]; }
    void goForward()    { [webView goForward]; }

   #if JUCE_MAC
    void stop()         { [webView stopLoading: nil]; }
    void refresh()      { [webView reload: nil]; }
   #else
    void stop()         { [webView stopLoading]; }
    void refresh()      { [webView reload]; }
   #endif

    void mouseMove (const MouseEvent&)
    {
        // WebKit doesn't capture mouse-moves itself, so it seems the only way to make
        // them work is to push them via this non-public method..
        if ([webView respondsToSelector: @selector (_updateMouseoverWithFakeEvent)])
            [webView performSelector:    @selector (_updateMouseoverWithFakeEvent)];
    }

private:
   #if JUCE_MAC
    WebView* webView = nil;
    id clickListener;
   #else
    UIWebView* webView = nil;
    id webViewDelegate;
   #endif
};

#endif

//==============================================================================
WebBrowserComponent::WebBrowserComponent (bool unloadWhenHidden)
    : unloadPageWhenBrowserIsHidden (unloadWhenHidden)
{
    setOpaque (true);
    browser.reset (new Pimpl (this));
    addAndMakeVisible (browser.get());
}

WebBrowserComponent::WebBrowserComponent (bool unloadWhenHidden,
                                          const File&,
                                          const File&)
    : WebBrowserComponent (unloadWhenHidden)
{
}

WebBrowserComponent::~WebBrowserComponent()
{
}

//==============================================================================
void WebBrowserComponent::goToURL (const String& url,
                                   const StringArray* headers,
                                   const MemoryBlock* postData)
{
    lastURL = url;

    if (headers != nullptr)
        lastHeaders = *headers;
    else
        lastHeaders.clear();

    if (postData != nullptr)
        lastPostData = *postData;
    else
        lastPostData.reset();

    blankPageShown = false;

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    browser->stop();
}

void WebBrowserComponent::goBack()
{
    lastURL.clear();
    blankPageShown = false;
    browser->goBack();
}

void WebBrowserComponent::goForward()
{
    lastURL.clear();
    browser->goForward();
}

void WebBrowserComponent::refresh()
{
    browser->refresh();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics&)
{
}

void WebBrowserComponent::checkWindowAssociation()
{
    if (isShowing())
    {
        reloadLastURL();

        if (blankPageShown)
            goBack();
    }
    else
    {
        if (unloadPageWhenBrowserIsHidden && ! blankPageShown)
        {
            // when the component becomes invisible, some stuff like flash
            // carries on playing audio, so we need to force it onto a blank
            // page to avoid this, (and send it back when it's made visible again).

            blankPageShown = true;
            browser->goToURL ("about:blank", nullptr, nullptr);
        }
    }
}

void WebBrowserComponent::reloadLastURL()
{
    if (lastURL.isNotEmpty())
    {
        goToURL (lastURL, &lastHeaders, &lastPostData);
        lastURL.clear();
    }
}

void WebBrowserComponent::parentHierarchyChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::resized()
{
    browser->setSize (getWidth(), getHeight());
}

void WebBrowserComponent::visibilityChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::focusGained (FocusChangeType)
{
}

void WebBrowserComponent::clearCookies()
{
    NSHTTPCookieStorage* storage = [NSHTTPCookieStorage sharedHTTPCookieStorage];

    if (NSArray* cookies = [storage cookies])
    {
        const NSUInteger n = [cookies count];

        for (NSUInteger i = 0; i < n; ++i)
            [storage deleteCookie: [cookies objectAtIndex: i]];
    }

    [[NSUserDefaults standardUserDefaults] synchronize];
}

} // namespace juce
