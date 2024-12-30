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

static NSURL* appendParametersToFileURL (const URL& url, NSURL* fileUrl)
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

static NSMutableURLRequest* getRequestForURL (const String& url, const StringArray* headers, const MemoryBlock* postData)
{
    NSString* urlString = juceStringToNS (url);

     urlString = [urlString stringByAddingPercentEncodingWithAllowedCharacters: [NSCharacterSet URLQueryAllowedCharacterSet]];

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

static var fromObject (id object)
{
    // An undefined var serialises to 'undefined' i.e. an expression not returning a value
    if (object == nil)
        return var::undefined();

    if ([object isKindOfClass:[NSNumber class]])
    {
        // The object returned by evaluateJavaScript is a __NSCFBoolean*, which is a private class
        // to the framework, but a handle to this class can be obtained through @YES. When cast to
        // an NSNumber this object would have the wrong type encoding, so [number objCType]; would
        // return 'c' instead of 'B', hence that approach wouldn't work.
        if ([object isKindOfClass: [@YES class]])
            return static_cast<NSNumber*> (object).boolValue == YES ? true : false;

        return static_cast<NSNumber*> (object).doubleValue;
    }

    if ([object isKindOfClass:[NSString class]])
        return nsStringToJuce (object);

    if ([object isKindOfClass:[NSArray class]])
    {
        Array<var> result;

        auto* array = static_cast<NSArray*> (object);

        for (id elem in array)
            result.add (fromObject (elem));

        return result;
    }

    if ([object isKindOfClass:[NSDictionary class]])
    {
        const auto* dict = static_cast<NSDictionary*> (object);

        DynamicObject::Ptr result (new DynamicObject());

        for (id key in dict)
            result->setProperty (nsStringToJuce (key), fromObject ([dict objectForKey:key]));

        return result.get();
    }

    if ([object isKindOfClass:[NSDate class]])
    {
        JUCE_AUTORELEASEPOOL
        {
            auto* date = static_cast<NSDate*> (object);
            auto* formatter = [[NSDateFormatter alloc] init];
            const auto javascriptDateFormatString = @"yyyy'-'MM'-'dd'T'HH':'mm':'ss.SSS'Z'";
            [formatter setDateFormat: javascriptDateFormatString];
            [formatter setTimeZone: [NSTimeZone timeZoneWithName: @"UTC"]];
            NSString* dateString = [formatter stringFromDate: date];
            return nsStringToJuce (dateString);
        }
    }

    // Returning a Void var, which serialises to 'null'
    if ([object isKindOfClass:[NSNull class]])
        return {};

    jassertfalse;
    return {};
}

using LastFocusChange = std::optional<Component::FocusChangeDirection>;

static const char* lastFocusChangeMemberName = "lastFocusChangeHandle";

[[maybe_unused]] static void setLastFocusChangeHandle (id instance, LastFocusChange* object)
{
    object_setInstanceVariable (instance, lastFocusChangeMemberName, object);
}

[[maybe_unused]] static LastFocusChange* getLastFocusChangeHandle (id instance)
{
    return getIvar<LastFocusChange*> (instance, lastFocusChangeMemberName);
}

#if JUCE_MAC
template <class WebViewClass>
struct WebViewKeyEquivalentResponder final : public ObjCClass<WebViewClass>
{
    using Base = ObjCClass<WebViewClass>;

    explicit WebViewKeyEquivalentResponder (bool acceptsFirstMouse)
        : Base ("WebViewKeyEquivalentResponder_")
    {
        this->template addIvar<LastFocusChange*> (lastFocusChangeMemberName);

        this->addMethod (@selector (performKeyEquivalent:),
                         [] (id self, SEL selector, NSEvent* event)
                         {
                             const auto isCommandDown = [event]
                             {
                                 const auto modifierFlags = [event modifierFlags];

                                 if (@available (macOS 10.12, *))
                                     return (modifierFlags & NSEventModifierFlagDeviceIndependentFlagsMask) == NSEventModifierFlagCommand;

                                 JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
                                 return (modifierFlags & NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask;
                                 JUCE_END_IGNORE_DEPRECATION_WARNINGS
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

                             return Base::template sendSuperclassMessage<BOOL> (self, selector, event);
                         });

        this->addMethod (@selector (resignFirstResponder),
                         [] (id self, SEL selector)
                         {
                            const auto result = Base::template sendSuperclassMessage<BOOL> (self, selector);

                            auto* focusChangeTypeHandle = getLastFocusChangeHandle (self);
                            jassert (focusChangeTypeHandle != nullptr); // Forgot to call setLastFocusChangeHandle?
                            focusChangeTypeHandle->emplace (Component::FocusChangeDirection::unknown);

                            auto* currentEvent = [NSApp currentEvent];

                            if (currentEvent == nullptr)
                                return result;

                            const auto eventType = [currentEvent type];

                            if (   eventType != NSEventTypeKeyUp
                                && eventType != NSEventTypeKeyDown
                                && eventType != NSEventTypeFlagsChanged)
                            {
                                return result;
                            }

                            // Device independent key numbers should be compared with Carbon
                            // constants, but we define the one we need here to avoid importing
                            // Carbon.h
                            static constexpr unsigned short carbonTabKeycode = 0x30;

                            if ([currentEvent keyCode] != carbonTabKeycode)
                                return result;

                            const auto shiftKeyDown = ([currentEvent modifierFlags] & NSEventModifierFlagShift) != 0;

                            focusChangeTypeHandle->emplace (shiftKeyDown ? Component::FocusChangeDirection::backward
                                                                         : Component::FocusChangeDirection::forward);

                            return result;
                         });

        if (acceptsFirstMouse)
            this->addMethod (@selector (acceptsFirstMouse:), [] (id, SEL, NSEvent*) { return YES; });

        this->registerClass();
    }
};

JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
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
JUCE_END_IGNORE_DEPRECATION_WARNINGS
#endif

// Connects the delegate to the rest of the implementation without making WebViewDelegateClass
// a nested class as well.
class DelegateConnector
{
public:
    DelegateConnector (WebBrowserComponent& browserIn,
                       std::function<void (const var&)> handleNativeEventFnIn,
                       std::function<std::optional<WebBrowserComponent::Resource> (const String&)> handleResourceRequestFnIn,
                       const WebBrowserComponent::Options& optionsIn)
        : browser (browserIn),
          handleNativeEventFn (std::move (handleNativeEventFnIn)),
          handleResourceRequestFn (std::move (handleResourceRequestFnIn)),
          options (optionsIn)
    {
    }

    auto& getBrowser() { return browser; }

    void handleNativeEvent (const var& message)
    {
        handleNativeEventFn (message);
    }

    auto handleResourceRequest (const String& url)
    {
        return handleResourceRequestFn (url);
    }

    [[nodiscard]] const auto& getOptions() const
    {
        return options;
    }

private:
    WebBrowserComponent& browser;
    std::function<void (const var&)> handleNativeEventFn;
    std::function<std::optional<WebBrowserComponent::Resource> (const String&)> handleResourceRequestFn;
    WebBrowserComponent::Options options;
};

struct WebViewDelegateClass final : public ObjCClass<NSObject>
{
    WebViewDelegateClass()  : ObjCClass ("JUCEWebViewDelegate_")
    {
        addIvar<DelegateConnector*> ("connector");

        addMethod (@selector (webView:decidePolicyForNavigationAction:decisionHandler:),
                   [] (id self, SEL, WKWebView*, WKNavigationAction* navigationAction, void (^decisionHandler) (WKNavigationActionPolicy))
                   {
                       if (auto* connector = getConnector (self))
                       {
                           if (connector->getBrowser().pageAboutToLoad (nsStringToJuce ([[[navigationAction request] URL] absoluteString])))
                               decisionHandler (WKNavigationActionPolicyAllow);
                           else
                               decisionHandler (WKNavigationActionPolicyCancel);
                       }
                   });

        addMethod (@selector (webView:didFinishNavigation:),
                   [] (id self, SEL, WKWebView* webview, WKNavigation*)
                   {
                       if (auto* connector = getConnector (self))
                           connector->getBrowser().pageFinishedLoading (nsStringToJuce ([[webview URL] absoluteString]));
                   });

        addMethod (@selector (webView:didFailNavigation:withError:),
                   [] (id self, SEL, WKWebView*, WKNavigation*, NSError* error)
                   {
                       if (auto* connector = getConnector (self))
                           displayError (&connector->getBrowser(), error);
                   });

        addMethod (@selector (webView:didFailProvisionalNavigation:withError:),
                   [] (id self, SEL, WKWebView*, WKNavigation*, NSError* error)
                   {
                       if (auto* connector = getConnector (self))
                           displayError (&connector->getBrowser(), error);
                   });

        addMethod (@selector (webViewDidClose:),
                   [] (id self, SEL, WKWebView*)
                   {
                       if (auto* connector = getConnector (self))
                           connector->getBrowser().windowCloseRequest();
                   });

        addMethod (@selector (webView:createWebViewWithConfiguration:forNavigationAction:windowFeatures:),
                   [] (id self, SEL, WKWebView*, WKWebViewConfiguration*, WKNavigationAction* navigationAction, WKWindowFeatures*)
                   {
                       if (auto* connector = getConnector (self))
                           connector->getBrowser().newWindowAttemptingToLoad (nsStringToJuce ([[[navigationAction request] URL] absoluteString]));

                       return nil;
                   });

        addMethod (@selector (userContentController:didReceiveScriptMessage:),
                   [] (id self, SEL, id, id message)
                   {
                       if (auto* connector = getConnector (self))
                       {
                           const auto object = fromObject ([message body]);

                           if (! object.isString())
                           {
                               jassertfalse;
                               return;
                           }

                           connector->handleNativeEvent (JSON::fromString (object.toString()));
                       }
                   });

        addMethod (@selector (webView:startURLSchemeTask:),
                   [] (id self, SEL, id, id urlSchemeTask)
                   {
                       auto* connector = getConnector (self);

                       if (connector == nullptr)
                       {
                           [urlSchemeTask didFailWithError: [NSError errorWithDomain:NSURLErrorDomain
                                                                                code:NSURLErrorCancelled
                                                                            userInfo: nil]];
                           return;
                       }

                       const auto request = [urlSchemeTask request];

                       auto* url = [&]
                       {
                           auto r = [request URL];

                           return r == nil ? [NSURL URLWithString:@""] : (NSURL* _Nonnull) r;
                       }();

                       const auto path = nsStringToJuce ([url path]);
                       const auto resource = connector->handleResourceRequest (path);

                       JUCE_AUTORELEASEPOOL
                       {
                           const auto makeResponse = [&url] (auto responseCode, id headers=nil)
                           {
                               auto response = [[NSHTTPURLResponse alloc] initWithURL:url
                                                                           statusCode:responseCode
                                                                          HTTPVersion:@"HTTP/1.1"
                                                                         headerFields:headers];

                               if (response == nil)
                                   return [[NSHTTPURLResponse alloc] autorelease];

                               return (NSHTTPURLResponse* _Nonnull) [response autorelease];
                           };

                           if (resource.has_value())
                           {
                               NSMutableDictionary* headers = [@ {
                                   @"Content-Length" : juceStringToNS (String { resource->data.size() }),
                                   @"Content-Type" : juceStringToNS (resource->mimeType),
                               } mutableCopy];

                               if (auto allowedOrigin = connector->getOptions().getAllowedOrigin())
                               {
                                   [headers setObject:juceStringToNS (*allowedOrigin)
                                               forKey:@"Access-Control-Allow-Origin"];
                               }

                               auto response = makeResponse (200, headers);

                               [urlSchemeTask didReceiveResponse:response];
                               [urlSchemeTask didReceiveData:[NSData dataWithBytes:resource->data.data()
                                                                            length:resource->data.size()]];
                           }
                           else
                           {
                               [urlSchemeTask didReceiveResponse:makeResponse (404)];
                           }

                           [urlSchemeTask didFinish];
                       }
                   });

        addMethod (@selector (webView:stopURLSchemeTask:),
                   [] (id, SEL, id, id)
                   {
                   });

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        if (@available (macOS 10.12, *))
        {
            addMethod (@selector (webView:runOpenPanelWithParameters:initiatedByFrame:completionHandler:),
                       [] (id self, SEL, WKWebView*, WKOpenPanelParameters* parameters, WKFrameInfo*, void (^completionHandler)(NSArray<NSURL*>*))
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

                           if (getConnector (self) == nullptr)
                               return;

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

    static void setConnector (id self, DelegateConnector* connector)
    {
        object_setInstanceVariable (self, "connector", connector);
    }

    static DelegateConnector* getConnector (id self)
    {
        return getIvar<DelegateConnector*> (self, "connector");
    }

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
struct WebBrowserComponent::Impl::Platform
{
    class WKWebViewImpl;
    class WebViewImpl;
};

static constexpr const char* platformSpecificIntegrationScript = R"(
window.__JUCE__ = {
  postMessage: function (object) {
    window.webkit.messageHandlers.__JUCE__.postMessage(object);
  },
};
)";

//==============================================================================
#if JUCE_MAC
JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
class WebBrowserComponent::Impl::Platform::WebViewImpl  : public WebBrowserComponent::Impl::PlatformInterface,
                                                         #if JUCE_MAC
                                                          public NSViewComponent
                                                         #else
                                                          public UIViewComponent
                                                         #endif
{
public:
    WebViewImpl (WebBrowserComponent::Impl& implIn, const String& userAgent) : browser (implIn.owner)
    {
        static WebViewKeyEquivalentResponder<WebView> webviewClass { false };

        webView.reset ([webviewClass.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                                          frameName: nsEmptyString()
                                                          groupName: nsEmptyString()]);

        setLastFocusChangeHandle (webView.get(), &lastFocusChange);

        webView.get().customUserAgent = juceStringToNS (userAgent);

        static DownloadClickDetectorClass cls;
        clickListener.reset ([cls.createInstance() init]);
        DownloadClickDetectorClass::setOwner (clickListener.get(), &browser);

        [webView.get() setPolicyDelegate:    clickListener.get()];
        [webView.get() setFrameLoadDelegate: clickListener.get()];
        [webView.get() setUIDelegate:        clickListener.get()];

        setView (webView.get());
        browser.addAndMakeVisible (this);
    }

    ~WebViewImpl() override
    {
        setView (nil);

        [webView.get() setPolicyDelegate:    nil];
        [webView.get() setFrameLoadDelegate: nil];
        [webView.get() setUIDelegate:        nil];
    }

    void setWebViewSize (int width, int height) override
    {
        setSize (width, height);
    }

    void checkWindowAssociation() override
    {
        if (browser.isShowing())
        {
            browser.reloadLastURL();

            if (browser.blankPageShown)
                browser.goBack();
        }
        else
        {
            if (browser.unloadPageWhenHidden && ! browser.blankPageShown)
            {
                // when the component becomes invisible, some stuff like flash
                // carries on playing audio, so we need to force it onto a blank
                // page to avoid this, (and send it back when it's made visible again).

                browser.blankPageShown = true;
                goToURL ("about:blank", nullptr, nullptr);
            }
        }
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

    void mouseMove (const MouseEvent&) override
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        // WebKit doesn't capture mouse-moves itself, so it seems the only way to make
        // them work is to push them via this non-public method..
        if ([webView.get() respondsToSelector: @selector (_updateMouseoverWithFakeEvent)])
            [webView.get() performSelector:    @selector (_updateMouseoverWithFakeEvent)];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    void evaluateJavascript (const String&, WebBrowserComponent::EvaluationCallback) override
    {
        // This feature is only available on MacOS 10.11 and above
        jassertfalse;
    }

private:
    WebBrowserComponent& browser;
    LastFocusChange lastFocusChange;
    ObjCObjectHandle<WebView*> webView;
    ObjCObjectHandle<id> clickListener;
};
JUCE_END_IGNORE_DEPRECATION_WARNINGS
#endif

class WebBrowserComponent::Impl::Platform::WKWebViewImpl : public WebBrowserComponent::Impl::PlatformInterface,
                                                          #if JUCE_MAC
                                                           public NSViewComponent
                                                          #else
                                                           public UIViewComponent
                                                          #endif
{
public:
    WKWebViewImpl (WebBrowserComponent::Impl& implIn,
                   const WebBrowserComponent::Options& browserOptions,
                   const StringArray& userScripts)
        : owner (implIn),
          delegateConnector (implIn.owner,
                             [this] (const auto& m) { owner.handleNativeEvent (m); },
                             [this] (const auto& r) { return owner.handleResourceRequest (r); },
                             browserOptions),
          allowAccessToEnclosingDirectory (browserOptions.getAppleWkWebViewOptions()
                                                         .getAllowAccessToEnclosingDirectory())
    {
        ObjCObjectHandle<WKWebViewConfiguration*> config { [WKWebViewConfiguration new] };
        id preferences = [config.get() preferences];

        [preferences setValue:@(true) forKey:@"fullScreenEnabled"];
        [preferences setValue:@(true) forKey:@"DOMPasteAllowed"];
        [preferences setValue:@(true) forKey:@"javaScriptCanAccessClipboard"];

        static WebViewDelegateClass cls;
        webViewDelegate.reset ([cls.createInstance() init]);
        WebViewDelegateClass::setConnector (webViewDelegate.get(), &delegateConnector);

        if (browserOptions.getNativeIntegrationsEnabled())
        {
            [[config.get() userContentController] addScriptMessageHandler:webViewDelegate.get()
                                                                     name:@"__JUCE__"];
        }

        // It isn't necessary to concatenate all scripts and add them as one. They will still work
        // when added separately. But in the latter case sometimes only the first one is visible in
        // the WebView developer console, so concatenating them helps with debugging.
        auto allUserScripts = userScripts;
        allUserScripts.insert (0, platformSpecificIntegrationScript);

        NSUniquePtr<WKUserScript> script { [[WKUserScript alloc]
              initWithSource:juceStringToNS (allUserScripts.joinIntoString ("\n"))
               injectionTime:WKUserScriptInjectionTimeAtDocumentStart
            forMainFrameOnly:YES] };

        [[config.get() userContentController] addUserScript:script.get()];

        if (@available (macOS 10.13, *))
        {
            if (browserOptions.getResourceProvider() != nullptr)
                [config.get() setURLSchemeHandler:webViewDelegate.get() forURLScheme:@"juce"];
        }

       #if JUCE_DEBUG
        [preferences setValue: @(true) forKey: @"developerExtrasEnabled"];
       #endif

       #if JUCE_MAC
        auto& webviewClass = [&]() -> auto&
        {
            if (browserOptions.getAppleWkWebViewOptions().getAcceptsFirstMouse())
            {
                static WebViewKeyEquivalentResponder<WKWebView> juceWebviewClass { true };
                return juceWebviewClass;
            }
            else
            {
                static WebViewKeyEquivalentResponder<WKWebView> juceWebviewClass { false };
                return juceWebviewClass;
            }
        }();

        webView.reset ([webviewClass.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                                      configuration: config.get()]);

        setLastFocusChangeHandle (webView.get(), &lastFocusChange);
       #else
        webView.reset ([[WKWebView alloc] initWithFrame: CGRectMake (0, 0, 100.0f, 100.0f)
                                          configuration: config.get()]);
       #endif

        if (const auto userAgent = browserOptions.getUserAgent(); userAgent.isNotEmpty())
            webView.get().customUserAgent = juceStringToNS (userAgent);

        [webView.get() setNavigationDelegate: webViewDelegate.get()];
        [webView.get() setUIDelegate:         webViewDelegate.get()];

        setView (webView.get());
        owner.owner.addAndMakeVisible (this);
    }

    ~WKWebViewImpl() override
    {
        WebViewDelegateClass::setConnector (webViewDelegate.get(), nullptr);

        setView (nil);
        [webView.get() setNavigationDelegate: nil];
        [webView.get() setUIDelegate:         nil];
    }

    void setWebViewSize (int width, int height) override
    {
        setSize (width, height);
    }

    void checkWindowAssociation() override
    {
        auto& browser = owner.owner;

        if (browser.isShowing())
        {
            browser.reloadLastURL();

            if (browser.blankPageShown)
                browser.goBack();
        }
        else
        {
            if (browser.unloadPageWhenHidden && ! browser.blankPageShown)
            {
                // when the component becomes invisible, some stuff like flash
                // carries on playing audio, so we need to force it onto a blank
                // page to avoid this, (and send it back when it's made visible again).

                browser.blankPageShown = true;
                goToURL ("about:blank", nullptr, nullptr);
            }
        }
    }

    void focusGainedWithDirection (FocusChangeType, FocusChangeDirection) override
    {
        const auto webViewFocusLossDirection = std::exchange (lastFocusChange, std::nullopt);

        // We didn't receive the focus from the WebView, so we need to pass it onto it
        if (! webViewFocusLossDirection.has_value())
        {
           #if JUCE_MAC
            [[webView.get() window] makeFirstResponder: webView.get()];
           #endif
            return;
        }

        auto* comp = [&]() -> Component*
        {
            auto* c = owner.owner.getParentComponent();

            if (c == nullptr)
                return nullptr;

            const auto traverser = c->createFocusTraverser();

            if (*webViewFocusLossDirection == FocusChangeDirection::forward)
            {
                if (auto* next = traverser->getNextComponent (this); next != nullptr)
                    return next;

                return traverser->getDefaultComponent (c);
            }

            if (*webViewFocusLossDirection == FocusChangeDirection::backward)
            {
                if (auto* previous = traverser->getPreviousComponent (&owner.owner); previous != nullptr)
                    return previous;

                if (auto all = traverser->getAllComponents (c); ! all.empty())
                    return all.back();
            }

            return nullptr;
        }();

        if (comp != nullptr)
            comp->getAccessibilityHandler()->grabFocus();
        else
            giveAwayKeyboardFocus();
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

            NSURL* nsUrl = [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())];

            auto* accessPath = [&]
            {
                if (! allowAccessToEnclosingDirectory)
                    return nsUrl;

                auto* parentUrl = [NSURL fileURLWithPath: juceStringToNS (file.getParentDirectory().getFullPathName())];

                if (parentUrl == nullptr)
                    return nsUrl;

                return parentUrl;
            }();

            if (nsUrl != nullptr)
                [webView.get() loadFileURL: appendParametersToFileURL (url, nsUrl) allowingReadAccessToURL: accessPath];
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

    void evaluateJavascript (const String& script, WebBrowserComponent::EvaluationCallback callback) override
    {
        [webView.get() evaluateJavaScript: juceStringToNS (script)
                        completionHandler: ^(id obj, NSError* error)
                                          {
                                            if (callback == nullptr)
                                                return;

                                            if (error != nil)
                                            {
                                                const auto resultError = [&]() -> WebBrowserComponent::EvaluationResult::Error
                                                {
                                                    const auto errorCode = [error code];

                                                    if (errorCode == 4)
                                                    {
                                                        String errorMsgTemplate { "JAVASCRIPT_ERROR at (EVALUATION_SOURCE:LINE_NUMBER:COLUMN_NUMBER)" };

                                                        if (id m = [error.userInfo objectForKey:@"WKJavaScriptExceptionMessage"]; m != nil)
                                                            errorMsgTemplate = errorMsgTemplate.replace ("JAVASCRIPT_ERROR", nsStringToJuce (m));

                                                        if (id m = [error.userInfo objectForKey:@"WKJavaScriptExceptionSourceURL"]; m != nil)
                                                            errorMsgTemplate = errorMsgTemplate.replace ("EVALUATION_SOURCE", nsStringToJuce ([m absoluteString]));

                                                        if (id m = [error.userInfo objectForKey:@"WKJavaScriptExceptionLineNumber"]; m != nil)
                                                            errorMsgTemplate = errorMsgTemplate.replace ("LINE_NUMBER", String { [m intValue] });

                                                        if (id m = [error.userInfo objectForKey:@"WKJavaScriptExceptionColumnNumber"]; m != nil)
                                                            errorMsgTemplate = errorMsgTemplate.replace ("COLUMN_NUMBER", String { [m intValue] });

                                                        return { WebBrowserComponent::EvaluationResult::Error::Type::javascriptException,
                                                                 errorMsgTemplate };
                                                    }
                                                    else if (errorCode == 5)
                                                    {
                                                        String errorMessage;

                                                        if (id m = [[error userInfo] objectForKey:@"NSLocalizedDescription"]; m != nil)
                                                            errorMessage = nsStringToJuce (m);

                                                        return { WebBrowserComponent::EvaluationResult::Error::Type::unsupportedReturnType,
                                                                 errorMessage };
                                                    }

                                                    return { WebBrowserComponent::EvaluationResult::Error::Type::unknown, "Unknown error" };
                                                }();

                                                callback (EvaluationResult { resultError });
                                            }
                                            else
                                            {
                                                callback (EvaluationResult { fromObject (obj) });
                                            }
                                          }];
    }

private:
    WebBrowserComponent::Impl& owner;
    DelegateConnector delegateConnector;
    bool allowAccessToEnclosingDirectory = false;
    LastFocusChange lastFocusChange;
    ObjCObjectHandle<WKWebView*> webView;
    ObjCObjectHandle<id> webViewDelegate;
};

//==============================================================================
auto WebBrowserComponent::Impl::createAndInitPlatformDependentPart (WebBrowserComponent::Impl& impl,
                                                                    const WebBrowserComponent::Options& options,
                                                                    const StringArray& userScripts)
    -> std::unique_ptr<PlatformInterface>
{
    return std::make_unique<Platform::WKWebViewImpl> (impl, options, userScripts);
}

//==============================================================================
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
