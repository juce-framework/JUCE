/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#if JUCE_MAC

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

        return String();
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
            const Array<File>& files = chooser.getResults();

            for (int i = 0; i < files.size(); ++i)
                [resultListener chooseFilename: juceStringToNS (files.getReference(i).getFullPathName())];
        }
       #else
        (void) resultListener; (void) allowMultipleFiles;
        jassertfalse; // Can't use this without modal loops being enabled!
       #endif
    }
};

#else

} // (juce namespace)

//==============================================================================
@interface WebViewTapDetector  : NSObject<UIGestureRecognizerDelegate>
{
}

- (BOOL) gestureRecognizer: (UIGestureRecognizer*) gestureRecognizer
         shouldRecognizeSimultaneouslyWithGestureRecognizer: (UIGestureRecognizer*) otherGestureRecognizer;
@end

@implementation WebViewTapDetector

- (BOOL) gestureRecognizer: (UIGestureRecognizer*) gestureRecognizer
         shouldRecognizeSimultaneouslyWithGestureRecognizer: (UIGestureRecognizer*) otherGestureRecognizer
{
    (void) gestureRecognizer;
    (void) otherGestureRecognizer;
    return YES;
}

@end

//==============================================================================
@interface WebViewURLChangeDetector : NSObject<UIWebViewDelegate>
{
    juce::WebBrowserComponent* ownerComponent;
}

- (WebViewURLChangeDetector*) initWithWebBrowserOwner: (juce::WebBrowserComponent*) ownerComponent;
- (BOOL) webView: (UIWebView*) webView shouldStartLoadWithRequest: (NSURLRequest*) request
                                                   navigationType: (UIWebViewNavigationType) navigationType;
- (void) webViewDidFinishLoad: (UIWebView*) webView;
@end

@implementation WebViewURLChangeDetector

- (WebViewURLChangeDetector*) initWithWebBrowserOwner: (juce::WebBrowserComponent*) ownerComp
{
    [super init];
    ownerComponent = ownerComp;
    return self;
}

- (BOOL) webView: (UIWebView*) webView shouldStartLoadWithRequest: (NSURLRequest*) request
                                                   navigationType: (UIWebViewNavigationType) navigationType
{
    (void) webView;
    (void) navigationType;
    return ownerComponent->pageAboutToLoad (nsStringToJuce (request.URL.absoluteString));
}

- (void) webViewDidFinishLoad: (UIWebView*) webView
{
    ownerComponent->pageFinishedLoading (nsStringToJuce (webView.request.URL.absoluteString));
}
@end

namespace juce {

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
        webView = [[WebView alloc] initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                       frameName: nsEmptyString()
                                       groupName: nsEmptyString()];
        setView (webView);

        static DownloadClickDetectorClass cls;
        clickListener = [cls.createInstance() init];
        DownloadClickDetectorClass::setOwner (clickListener, owner);
        [webView setPolicyDelegate: clickListener];
        [webView setFrameLoadDelegate: clickListener];
        [webView setUIDelegate: clickListener];
       #else
        webView = [[UIWebView alloc] initWithFrame: CGRectMake (0, 0, 1.0f, 1.0f)];
        setView (webView);

        tapDetector = [[WebViewTapDetector alloc] init];
        urlDetector = [[WebViewURLChangeDetector alloc] initWithWebBrowserOwner: owner];
        gestureRecogniser = nil;
        webView.delegate = urlDetector;
       #endif
    }

    ~Pimpl()
    {
       #if JUCE_MAC
        [webView setPolicyDelegate: nil];
        [webView setFrameLoadDelegate: nil];
        [webView setUIDelegate: nil];
        [clickListener release];
       #else
        webView.delegate = nil;
        [webView removeGestureRecognizer: gestureRecogniser];
        [gestureRecogniser release];
        [tapDetector release];
        [urlDetector release];
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
        else
        {
            NSMutableURLRequest* r
                = [NSMutableURLRequest requestWithURL: [NSURL URLWithString: juceStringToNS (url)]
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
                    const String headerName  ((*headers)[i].upToFirstOccurrenceOf (":", false, false).trim());
                    const String headerValue ((*headers)[i].fromFirstOccurrenceOf (":", false, false).trim());

                    [r setValue: juceStringToNS (headerValue)
                       forHTTPHeaderField: juceStringToNS (headerName)];
                }
            }

           #if JUCE_MAC
            [[webView mainFrame] loadRequest: r];
           #else
            [webView loadRequest: r];
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
    WebView* webView;
    NSObject* clickListener;
   #else
    UIWebView* webView;
    WebViewTapDetector* tapDetector;
    WebViewURLChangeDetector* urlDetector;
    UITapGestureRecognizer* gestureRecogniser;
   #endif
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadWhenHidden)
    : browser (nullptr),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadWhenHidden)
{
    setOpaque (true);

    addAndMakeVisible (browser = new Pimpl (this));
}

WebBrowserComponent::~WebBrowserComponent()
{
    deleteAndZero (browser);
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
            browser->goToURL ("about:blank", 0, 0);
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
