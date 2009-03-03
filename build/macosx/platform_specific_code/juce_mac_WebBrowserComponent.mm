/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_WEB_BROWSER

//==============================================================================
END_JUCE_NAMESPACE

#define DownloadClickDetector MakeObjCClassName(DownloadClickDetector)


@interface DownloadClickDetector   : NSObject
{
    JUCE_NAMESPACE::WebBrowserComponent* ownerComponent;
}

- (DownloadClickDetector*) initWithWebBrowserOwner: (JUCE_NAMESPACE::WebBrowserComponent*) ownerComponent;

- (void) webView: (WebView*) webView decidePolicyForNavigationAction: (NSDictionary*) actionInformation
                                                             request: (NSURLRequest*) request
                                                               frame: (WebFrame*) frame
                                                    decisionListener: (id<WebPolicyDecisionListener>) listener;
@end

//==============================================================================
@implementation DownloadClickDetector

- (DownloadClickDetector*) initWithWebBrowserOwner: (JUCE_NAMESPACE::WebBrowserComponent*) ownerComponent_
{
    [super init];
    ownerComponent = ownerComponent_;
    return self;
}

- (void) webView: (WebView*) sender decidePolicyForNavigationAction: (NSDictionary*) actionInformation
                                                            request: (NSURLRequest*) request
                                                              frame: (WebFrame*) frame
                                                   decisionListener: (id <WebPolicyDecisionListener>) listener
{
    NSURL* url = [actionInformation valueForKey: @"WebActionOriginalURLKey"];

    const MessageManagerLock mml;

    if (ownerComponent->pageAboutToLoad (nsStringToJuce ([url absoluteString])))
        [listener use];
    else
        [listener ignore];
}

@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
class WebBrowserComponentInternal  : public NSViewComponent
{
public:
    WebBrowserComponentInternal (WebBrowserComponent* owner)
    {
        webView = [[WebView alloc] initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)
                                       frameName: @""
                                       groupName: @""];
        setView (webView);

        clickListener = [[DownloadClickDetector alloc] initWithWebBrowserOwner: owner];
        [webView setPolicyDelegate: clickListener];
    }

    ~WebBrowserComponentInternal()
    {
        [webView setPolicyDelegate: nil];
        [clickListener release];
        setView (0);
    }

    void goToURL (const String& url,
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        NSMutableURLRequest* r
            = [NSMutableURLRequest requestWithURL: [NSURL URLWithString: juceStringToNS (url)]
                                      cachePolicy: NSURLRequestUseProtocolCachePolicy
                                  timeoutInterval: 30.0];

        if (postData != 0 && postData->getSize() > 0)
        {
            [r setHTTPMethod: @"POST"];
            [r setHTTPBody: [NSData dataWithBytes: postData->getData()
                                           length: postData->getSize()]];
        }

        if (headers != 0)
        {
            for (int i = 0; i < headers->size(); ++i)
            {
                const String headerName ((*headers)[i].upToFirstOccurrenceOf (T(":"), false, false).trim());
                const String headerValue ((*headers)[i].fromFirstOccurrenceOf (T(":"), false, false).trim());

                [r setValue: juceStringToNS (headerValue)
                   forHTTPHeaderField: juceStringToNS (headerName)];
            }
        }

        stop();
        [[webView mainFrame] loadRequest: r];
    }

    void goBack()
    {
        [webView goBack];
    }

    void goForward()
    {
        [webView goForward];
    }

    void stop()
    {
        [webView stopLoading: nil];
    }

    void refresh()
    {
        [webView reload: nil];
    }

private:
    WebView* webView;
    DownloadClickDetector* clickListener;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent()
    : browser (0),
      blankPageShown (false)
{
    setOpaque (true);

    addAndMakeVisible (browser = new WebBrowserComponentInternal (this));
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

    lastHeaders.clear();
    if (headers != 0)
        lastHeaders = *headers;

    lastPostData.setSize (0);
    if (postData != 0)
        lastPostData = *postData;

    blankPageShown = false;

    browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    browser->stop();
}

void WebBrowserComponent::goBack()
{
    lastURL = String::empty;
    blankPageShown = false;
    browser->goBack();
}

void WebBrowserComponent::goForward()
{
    lastURL = String::empty;
    browser->goForward();
}

void WebBrowserComponent::refresh()
{
    browser->refresh();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
}

void WebBrowserComponent::checkWindowAssociation()
{
    // when the component becomes invisible, some stuff like flash
    // carries on playing audio, so we need to force it onto a blank
    // page to avoid this, (and send it back when it's made visible again).

    if (isShowing())
    {
        if (blankPageShown)
            goBack();
    }
    else
    {
        if (! blankPageShown)
        {
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
        lastURL = String::empty;
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

bool WebBrowserComponent::pageAboutToLoad (const String& url)
{
    return true;
}

#endif
