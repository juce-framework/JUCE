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

static NSURL* appendParametersToFileURL (const URL& url, NSURL* fileUrl)
{
    if (@available (macOS 10.10, *))
    {
        const auto parameterNames = url.getParameterNames();
        const auto parameterValues = url.getParameterValues();

        jassert (parameterNames.size() == parameterValues.size());

        if (parameterNames.isEmpty())
            return fileUrl;

        NSUniquePtr<NSURLComponents> components ([[NSURLComponents alloc] initWithURL: fileUrl resolvingAgainstBaseURL: NO]);
        NSUniquePtr<NSMutableArray> queryItems ([[NSMutableArray alloc] init]);

        for (int i = 0; i < parameterNames.size(); ++i)
            [queryItems.get() addObject: [NSURLQueryItem queryItemWithName: juceStringToNS (parameterNames[i])
                                                                     value: juceStringToNS (parameterValues[i])]];

        [components.get() setQueryItems: queryItems.get()];

        return [components.get() URL];
    }

    const auto queryString = url.getQueryString();

    if (queryString.isNotEmpty())
        if (NSString* fileUrlString = [fileUrl absoluteString])
            return [NSURL URLWithString: [fileUrlString stringByAppendingString: juceStringToNS (queryString)]];

    return fileUrl;
}

static NSMutableURLRequest* getRequestForURL (const String& url, const StringArray* headers, const MemoryBlock* postData)
{
    NSString* urlString = juceStringToNS (url);

     if (@available (macOS 10.9, *))
     {
         urlString = [urlString stringByAddingPercentEncodingWithAllowedCharacters: [NSCharacterSet URLQueryAllowedCharacterSet]];
     }
     else
     {
         JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
         urlString = [urlString stringByAddingPercentEscapesUsingEncoding: NSUTF8StringEncoding];
         JUCE_END_IGNORE_WARNINGS_GCC_LIKE
     }

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

#if JUCE_MAC
template <class WebViewClass>
struct WebViewKeyEquivalentResponder final : public ObjCClass<WebViewClass>
{
    WebViewKeyEquivalentResponder()
        : ObjCClass<WebViewClass> ("WebViewKeyEquivalentResponder_")
    {
        this->addMethod (@selector (performKeyEquivalent:),
                         [] (id self, SEL selector, NSEvent* event)
                         {
                             const auto isCommandDown = [event]
                             {
                                 const auto modifierFlags = [event modifierFlags];

                                 if (@available (macOS 10.12, *))
                                     return (modifierFlags & NSEventModifierFlagDeviceIndependentFlagsMask) == NSEventModifierFlagCommand;

                                 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
                                 return (modifierFlags & NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask;
                                 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
                             }();

                             if (isCommandDown)
                             {
                                 auto sendAction = [&] (SEL actionSelector) -> BOOL
                                 {
                                     return [NSApp sendAction:actionSelector
                                                           to:[[self window] firstResponder]
                                                         from:self];
                                 };

                                 if ([[event charactersIgnoringModifiers] isEqualToString:@"x"])
                                     return sendAction (@selector (cut:));
                                 if ([[event charactersIgnoringModifiers] isEqualToString:@"c"])
                                     return sendAction (@selector (copy:));
                                 if ([[event charactersIgnoringModifiers] isEqualToString:@"v"])
                                     return sendAction (@selector (paste:));
                                 if ([[event charactersIgnoringModifiers] isEqualToString:@"a"])
                                     return sendAction (@selector (selectAll:));
                             }

                             return ObjCClass<WebViewClass>::template sendSuperclassMessage<BOOL> (self, selector, event);
                         });
        this->registerClass();
    }
};

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
struct DownloadClickDetectorClass final : public ObjCClass<NSObject>
{
    DownloadClickDetectorClass()  : ObjCClass ("JUCEWebClickDetector_")
    {
        addIvar<WebBrowserComponent*> ("owner");

        addMethod (@selector (webView:didFailLoadWithError:forFrame:),            didFailLoadWithError);
        addMethod (@selector (webView:didFailProvisionalLoadWithError:forFrame:), didFailLoadWithError);

        addMethod (@selector (webView:decidePolicyForNavigationAction:request:frame:decisionListener:),
                   [] (id self, SEL, WebView*, NSDictionary* actionInformation, NSURLRequest*, WebFrame*, id<WebPolicyDecisionListener> listener)
                   {
                       if (getOwner (self)->pageAboutToLoad (getOriginalURL (actionInformation)))
                           [listener use];
                       else
                           [listener ignore];
                   });

        addMethod (@selector (webView:decidePolicyForNewWindowAction:request:newFrameName:decisionListener:),
                   [] (id self, SEL, WebView*, NSDictionary* actionInformation, NSURLRequest*, NSString*, id<WebPolicyDecisionListener> listener)
                   {
                       getOwner (self)->newWindowAttemptingToLoad (getOriginalURL (actionInformation));
                       [listener ignore];
                   });

        addMethod (@selector (webView:didFinishLoadForFrame:),
                   [] (id self, SEL, WebView* sender, WebFrame* frame)
                   {
                       if ([frame isEqual:[sender mainFrame]])
                       {
                           NSURL* url = [[[frame dataSource] request] URL];
                           getOwner (self)->pageFinishedLoading (nsStringToJuce ([url absoluteString]));
                       }
                   });

        addMethod (@selector (webView:willCloseFrame:),
                   [] (id self, SEL, WebView*, WebFrame*)
                   {
                       getOwner (self)->windowCloseRequest();
                   });

        addMethod (@selector (webView:runOpenPanelForFileButtonWithResultListener:allowMultipleFiles:),
                   [] (id, SEL, WebView*, id<WebOpenPanelResultListener> resultListener, BOOL allowMultipleFiles)
                   {
                       struct DeletedFileChooserWrapper final : private DeletedAtShutdown
                       {
                           DeletedFileChooserWrapper (std::unique_ptr<FileChooser> fc, id<WebOpenPanelResultListener> rl)
                               : chooser (std::move (fc)), listener (rl)
                           {
                               [listener.get() retain];
                           }

                           std::unique_ptr<FileChooser> chooser;
                           ObjCObjectHandle<id<WebOpenPanelResultListener>> listener;
                       };

                       auto chooser = std::make_unique<FileChooser> (TRANS ("Select the file you want to upload..."),
                                                                     File::getSpecialLocation (File::userHomeDirectory),
                                                                     "*");
                       auto* wrapper = new DeletedFileChooserWrapper (std::move (chooser), resultListener);

                       auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles
                                    | (allowMultipleFiles ? FileBrowserComponent::canSelectMultipleItems : 0);

                       wrapper->chooser->launchAsync (flags, [wrapper] (const FileChooser&)
                       {
                           for (auto& f : wrapper->chooser->getResults())
                               [wrapper->listener.get() chooseFilename: juceStringToNS (f.getFullPathName())];

                           delete wrapper;
                       });
                   });

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
};
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

struct API_AVAILABLE (macos (10.10)) WebViewDelegateClass final : public ObjCClass<NSObject>
{
    WebViewDelegateClass()  : ObjCClass ("JUCEWebViewDelegate_")
    {
        addIvar<WebBrowserComponent*> ("owner");

        addMethod (@selector (webView:decidePolicyForNavigationAction:decisionHandler:),
                   [] (id self, SEL, WKWebView*, WKNavigationAction* navigationAction, void (^decisionHandler) (WKNavigationActionPolicy))
                   {
                       if (getOwner (self)->pageAboutToLoad (nsStringToJuce ([[[navigationAction request] URL] absoluteString])))
                           decisionHandler (WKNavigationActionPolicyAllow);
                       else
                           decisionHandler (WKNavigationActionPolicyCancel);
                   });

        addMethod (@selector (webView:didFinishNavigation:),
                   [] (id self, SEL, WKWebView* webview, WKNavigation*)
                   {
                       getOwner (self)->pageFinishedLoading (nsStringToJuce ([[webview URL] absoluteString]));
                   });

        addMethod (@selector (webView:didFailNavigation:withError:),
                   [] (id self, SEL, WKWebView*, WKNavigation*, NSError* error)
                   {
                       displayError (getOwner (self), error);
                   });

        addMethod (@selector (webView:didFailProvisionalNavigation:withError:),
                   [] (id self, SEL, WKWebView*, WKNavigation*, NSError* error)
                   {
                       displayError (getOwner (self), error);
                   });

        addMethod (@selector (webViewDidClose:),
                   [] (id self, SEL, WKWebView*)
                   {
                       getOwner (self)->windowCloseRequest();
                   });

        addMethod (@selector (webView:createWebViewWithConfiguration:forNavigationAction:windowFeatures:),
                   [] (id self, SEL, WKWebView*, WKWebViewConfiguration*, WKNavigationAction* navigationAction, WKWindowFeatures*)
                   {
                       getOwner (self)->newWindowAttemptingToLoad (nsStringToJuce ([[[navigationAction request] URL] absoluteString]));
                       return nil;
                   });

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        if (@available (macOS 10.12, *))
        {
            addMethod (@selector (webView:runOpenPanelWithParameters:initiatedByFrame:completionHandler:),
                       [] (id, SEL, WKWebView*, WKOpenPanelParameters* parameters, WKFrameInfo*, void (^completionHandler)(NSArray<NSURL*>*))
                       {
                           using CompletionHandlerType = decltype (completionHandler);

                           class DeletedFileChooserWrapper final : private DeletedAtShutdown
                           {
                           public:
                               DeletedFileChooserWrapper (std::unique_ptr<FileChooser> fc, CompletionHandlerType h)
                                   : chooser (std::move (fc)), handler (h)
                               {
                                   [handler.get() retain];
                               }

                               ~DeletedFileChooserWrapper()
                               {
                                   callHandler (nullptr);
                               }

                               void callHandler (NSArray<NSURL*>* urls)
                               {
                                   if (handlerCalled)
                                       return;

                                   handler.get() (urls);
                                   handlerCalled = true;
                               }

                               std::unique_ptr<FileChooser> chooser;

                           private:
                               ObjCObjectHandle<CompletionHandlerType> handler;
                               bool handlerCalled = false;
                           };

                           auto chooser = std::make_unique<FileChooser> (TRANS ("Select the file you want to upload..."),
                                                                         File::getSpecialLocation (File::userHomeDirectory), "*");
                           auto* wrapper = new DeletedFileChooserWrapper (std::move (chooser), completionHandler);

                           auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles
                                        | ([parameters allowsMultipleSelection] ? FileBrowserComponent::canSelectMultipleItems : 0);

                          #if JUCE_MAC
                           if (@available (macOS 10.14, *))
                           {
                               if ([parameters allowsDirectories])
                                   flags |= FileBrowserComponent::canSelectDirectories;
                           }
                          #endif

                           wrapper->chooser->launchAsync (flags, [wrapper] (const FileChooser&)
                           {
                               auto results = wrapper->chooser->getResults();
                               auto urls = [NSMutableArray arrayWithCapacity: (NSUInteger) results.size()];

                               for (auto& f : results)
                                   [urls addObject: [NSURL fileURLWithPath: juceStringToNS (f.getFullPathName())]];

                               wrapper->callHandler (urls);
                               delete wrapper;
                           });
                       });
        }
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        registerClass();
    }

    static void setOwner (id self, WebBrowserComponent* owner)   { object_setInstanceVariable (self, "owner", owner); }
    static WebBrowserComponent* getOwner (id self)               { return getIvar<WebBrowserComponent*> (self, "owner"); }

private:
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
};

//==============================================================================
struct WebViewBase
{
    virtual ~WebViewBase() = default;

    virtual void goToURL (const String&, const StringArray*, const MemoryBlock*) = 0;
    virtual void goBack() = 0;
    virtual void goForward() = 0;
    virtual void stop() = 0;
    virtual void refresh() = 0;

    virtual id getWebView() = 0;
};

#if JUCE_MAC
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
class WebViewImpl  : public WebViewBase
{
public:
    WebViewImpl (WebBrowserComponent* owner, const String& userAgent)
    {
        static WebViewKeyEquivalentResponder<WebView> webviewClass;

        webView.reset ([webviewClass.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                                          frameName: nsEmptyString()
                                                          groupName: nsEmptyString()]);

        webView.get().customUserAgent = juceStringToNS (userAgent);

        static DownloadClickDetectorClass cls;
        clickListener.reset ([cls.createInstance() init]);
        DownloadClickDetectorClass::setOwner (clickListener.get(), owner);

        [webView.get() setPolicyDelegate:    clickListener.get()];
        [webView.get() setFrameLoadDelegate: clickListener.get()];
        [webView.get() setUIDelegate:        clickListener.get()];
    }

    ~WebViewImpl() override
    {
        [webView.get() setPolicyDelegate:    nil];
        [webView.get() setFrameLoadDelegate: nil];
        [webView.get() setUIDelegate:        nil];
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData) override
    {
        if (url.trimStart().startsWithIgnoreCase ("javascript:"))
        {
            [webView.get() stringByEvaluatingJavaScriptFromString: juceStringToNS (url.fromFirstOccurrenceOf (":", false, false))];
            return;
        }

        stop();

        auto getRequest = [&]() -> NSMutableURLRequest*
        {
            if (url.trimStart().startsWithIgnoreCase ("file:"))
            {
                auto file = URL (url).getLocalFile();

                if (NSURL* nsUrl = [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())])
                    return [NSMutableURLRequest requestWithURL: appendParametersToFileURL (url, nsUrl)
                                                   cachePolicy: NSURLRequestUseProtocolCachePolicy
                                               timeoutInterval: 30.0];

                return nullptr;
            }

            return getRequestForURL (url, headers, postData);
        };

        if (NSMutableURLRequest* request = getRequest())
            [[webView.get() mainFrame] loadRequest: request];
    }

    void goBack() override      { [webView.get() goBack]; }
    void goForward() override   { [webView.get() goForward]; }

    void stop() override        { [webView.get() stopLoading: nil]; }
    void refresh() override     { [webView.get() reload: nil]; }

    id getWebView() override    { return webView.get(); }

    void mouseMove (const MouseEvent&)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        // WebKit doesn't capture mouse-moves itself, so it seems the only way to make
        // them work is to push them via this non-public method..
        if ([webView.get() respondsToSelector: @selector (_updateMouseoverWithFakeEvent)])
            [webView.get() performSelector:    @selector (_updateMouseoverWithFakeEvent)];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

private:
    ObjCObjectHandle<WebView*> webView;
    ObjCObjectHandle<id> clickListener;
};
JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

class API_AVAILABLE (macos (10.11)) WKWebViewImpl : public WebViewBase
{
public:
    WKWebViewImpl (WebBrowserComponent* owner, const String& userAgent)
    {
       #if JUCE_MAC
        static WebViewKeyEquivalentResponder<WKWebView> webviewClass;

        webView.reset ([webviewClass.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)]);
       #else
        webView.reset ([[WKWebView alloc] initWithFrame: CGRectMake (0, 0, 100.0f, 100.0f)]);
       #endif

        if (userAgent.isNotEmpty())
            webView.get().customUserAgent = juceStringToNS (userAgent);

        static WebViewDelegateClass cls;
        webViewDelegate.reset ([cls.createInstance() init]);
        WebViewDelegateClass::setOwner (webViewDelegate.get(), owner);

        [webView.get() setNavigationDelegate: webViewDelegate.get()];
        [webView.get() setUIDelegate:         webViewDelegate.get()];

       #if JUCE_DEBUG
        [[[webView.get() configuration] preferences] setValue: @(true) forKey: @"developerExtrasEnabled"];
       #endif
    }

    ~WKWebViewImpl() override
    {
        [webView.get() setNavigationDelegate: nil];
        [webView.get() setUIDelegate:         nil];
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData) override
    {
        auto trimmed = url.trimStart();

        if (trimmed.startsWithIgnoreCase ("javascript:"))
        {
            [webView.get() evaluateJavaScript: juceStringToNS (url.fromFirstOccurrenceOf (":", false, false))
                            completionHandler: nil];

            return;
        }

        stop();

        if (trimmed.startsWithIgnoreCase ("file:"))
        {
            auto file = URL (url).getLocalFile();

            if (NSURL* nsUrl = [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())])
                [webView.get() loadFileURL: appendParametersToFileURL (url, nsUrl) allowingReadAccessToURL: nsUrl];
        }
        else if (NSMutableURLRequest* request = getRequestForURL (url, headers, postData))
        {
            [webView.get() loadRequest: request];
        }
    }

    void goBack() override      { [webView.get() goBack]; }
    void goForward() override   { [webView.get() goForward]; }

    void stop() override        { [webView.get() stopLoading]; }
    void refresh() override     { [webView.get() reload]; }

    id getWebView() override    { return webView.get(); }

private:
    ObjCObjectHandle<WKWebView*> webView;
    ObjCObjectHandle<id> webViewDelegate;
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
    Pimpl (WebBrowserComponent* owner, const String& userAgent)
    {
        if (@available (macOS 10.11, *))
            webView = std::make_unique<WKWebViewImpl> (owner, userAgent);
       #if JUCE_MAC
        else
            webView = std::make_unique<WebViewImpl> (owner, userAgent);
       #endif

        setView (webView->getWebView());
    }

    ~Pimpl() override
    {
        webView = nullptr;
        setView (nil);
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        webView->goToURL (url, headers, postData);
    }

    void goBack()      { webView->goBack(); }
    void goForward()   { webView->goForward(); }

    void stop()        { webView->stop(); }
    void refresh()     { webView->refresh(); }

private:
    std::unique_ptr<WebViewBase> webView;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const Options& options)
    : unloadPageWhenHidden (! options.keepsPageLoadedWhenBrowserIsHidden())
{
    setOpaque (true);
    browser.reset (new Pimpl (this, options.getUserAgent()));
    addAndMakeVisible (browser.get());
}

WebBrowserComponent::~WebBrowserComponent() = default;

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
        if (unloadPageWhenHidden && ! blankPageShown)
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

void WebBrowserComponent::focusGainedWithDirection (FocusChangeType, FocusChangeDirection)
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

//==============================================================================
bool WebBrowserComponent::areOptionsSupported (const Options& options)
{
    return (options.getBackend() == Options::Backend::defaultBackend);
}

} // namespace juce
