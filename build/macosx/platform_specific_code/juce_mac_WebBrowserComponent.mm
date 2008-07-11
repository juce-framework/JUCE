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

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#include <Cocoa/Cocoa.h>
#include <WebKit/WebKit.h>
#include <WebKit/HIWebView.h>
#include <WebKit/WebPolicyDelegate.h>
#include <WebKit/CarbonUtils.h>

BEGIN_JUCE_NAMESPACE
#include "../../../src/juce_appframework/events/juce_Timer.h"
#include "../../../src/juce_appframework/gui/components/special/juce_WebBrowserComponent.h"
END_JUCE_NAMESPACE

//==============================================================================
@interface DownloadClickDetector   : NSObject
{
    JUCE_NAMESPACE::WebBrowserComponent* ownerComponent;
}

- (DownloadClickDetector*) initWithOwner: (JUCE_NAMESPACE::WebBrowserComponent*) ownerComponent;

- (void) webView: (WebView*) webView decidePolicyForNavigationAction: (NSDictionary*) actionInformation
                                                             request: (NSURLRequest*) request
                                                               frame: (WebFrame*) frame
                                                    decisionListener: (id<WebPolicyDecisionListener>) listener;
@end

//==============================================================================
@implementation DownloadClickDetector

- (DownloadClickDetector*) initWithOwner: (JUCE_NAMESPACE::WebBrowserComponent*) ownerComponent_
{
    [super init];
    ownerComponent = ownerComponent_;
    return self;
}

- (void) webView: (WebView*) sender decidePolicyForNavigationAction: (NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id < WebPolicyDecisionListener >)listener
{
    NSURL* url = [actionInformation valueForKey: @"WebActionOriginalURLKey"];

    if (ownerComponent->pageAboutToLoad (JUCE_NAMESPACE::String::fromUTF8 ((const JUCE_NAMESPACE::uint8*) [[url absoluteString] UTF8String])))
        [listener use];
    else
        [listener ignore];
}

@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
class WebBrowserComponentInternal   : public Timer
{
public:
    WebBrowserComponentInternal (WebBrowserComponent* owner_)
        : owner (owner_),
          view (0),
          webView (0)
    {
        HIWebViewCreate (&view);

        ComponentPeer* const peer = owner_->getPeer();
        jassert (peer != 0);

        if (view != 0 && peer != 0)
        {
            WindowRef parentWindow = (WindowRef) peer->getNativeHandle();

            WindowAttributes attributes;
            GetWindowAttributes (parentWindow, &attributes);

            HIViewRef parentView = 0;

            if ((attributes & kWindowCompositingAttribute) != 0)
            {
                HIViewRef root = HIViewGetRoot (parentWindow);
                HIViewFindByID (root, kHIViewWindowContentID, &parentView);

                if (parentView == 0)
                    parentView = root;
            }
            else
            {
                GetRootControl (parentWindow, (ControlRef*) &parentView);

                if (parentView == 0)
                    CreateRootControl (parentWindow, (ControlRef*) &parentView);
            }

            HIViewAddSubview (parentView, view);
            updateBounds();
            show();

            webView = HIWebViewGetWebView (view);

            clickListener = [[DownloadClickDetector alloc] initWithOwner: owner_];
            [webView setPolicyDelegate: clickListener];
        }

        startTimer (500);
    }

    ~WebBrowserComponentInternal()
    {
        [webView setPolicyDelegate: nil];
        [clickListener release];

        if (view != 0)
            CFRelease (view);
    }

    // Horrific bodge-workaround for the fact that the webview somehow hangs onto key
    // focus when you pop up a new window, no matter what that window does to
    // try to grab focus for itself. This catches such a situation and forces
    // focus away from the webview, then back to the place it should be..
    void timerCallback()
    {
        WindowRef viewWindow = HIViewGetWindow (view);
        WindowRef focusedWindow = GetUserFocusWindow();

        if (focusedWindow != viewWindow)
        {
            if (HIViewSubtreeContainsFocus (view))
            {
                HIViewAdvanceFocus (HIViewGetRoot (viewWindow), 0);
                HIViewAdvanceFocus (HIViewGetRoot (focusedWindow), 0);
            }
        }
    }

    void show()
    {
        HIViewSetVisible (view, true);
    }

    void hide()
    {
        HIViewSetVisible (view, false);
    }

    void goToURL (const String& url, 
                  const StringArray* headers,
                  const MemoryBlock* postData)
    {
        char** headerNamesAsChars = 0;
        char** headerValuesAsChars = 0;
        int numHeaders = 0;

        if (headers != 0)
        {
            numHeaders = headers->size();

            headerNamesAsChars = (char**) juce_malloc (sizeof (char*) * numHeaders);
            headerValuesAsChars = (char**) juce_malloc (sizeof (char*) * numHeaders);

            int i;
            for (i = 0; i < numHeaders; ++i)
            {
                const String headerName ((*headers)[i].upToFirstOccurrenceOf (T(":"), false, false).trim());
                headerNamesAsChars[i] = (char*) juce_calloc (headerName.copyToUTF8 (0));
                headerName.copyToUTF8 ((JUCE_NAMESPACE::uint8*) headerNamesAsChars[i]);

                const String headerValue ((*headers)[i].fromFirstOccurrenceOf (T(":"), false, false).trim());
                headerValuesAsChars[i] = (char*) juce_calloc (headerValue.copyToUTF8 (0));
                headerValue.copyToUTF8 ((JUCE_NAMESPACE::uint8*) headerValuesAsChars[i]);
            }
        }

        sendWebViewToURL ((const char*) url.toUTF8(),
                          (const char**) headerNamesAsChars, 
                          (const char**) headerValuesAsChars, 
                          numHeaders,
                          postData != 0 ? (const char*) postData->getData() : 0,
                          postData != 0 ? postData->getSize() : 0);

        for (int i = 0; i < numHeaders; ++i)
        {
            juce_free (headerNamesAsChars[i]);
            juce_free (headerValuesAsChars[i]);
        }

        juce_free (headerNamesAsChars);
        juce_free (headerValuesAsChars);
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

    void updateBounds()
    {
        HIRect r;
        r.origin.x = (float) owner->getScreenX() - owner->getTopLevelComponent()->getScreenX();
        r.origin.y = (float) owner->getScreenY() - owner->getTopLevelComponent()->getScreenY();
        r.size.width = (float) owner->getWidth();
        r.size.height = (float) owner->getHeight();
        HIViewSetFrame (view, &r);
    }

private:
    WebBrowserComponent* const owner;
    HIViewRef view;
    WebView* webView;
    DownloadClickDetector* clickListener;
    
    void sendWebViewToURL (const char* utf8URL,
                           const char** headerNames, 
                           const char** headerValues, 
                           int numHeaders,
                           const char* postData, 
                           int postDataSize)
    {
        NSMutableURLRequest* r = [NSMutableURLRequest 
                                    requestWithURL: [NSURL URLWithString: [NSString stringWithUTF8String: utf8URL]]
                                    cachePolicy: NSURLRequestUseProtocolCachePolicy
                                    timeoutInterval: 30.0];

        if (postDataSize > 0)
        {
            [ r setHTTPMethod: @"POST"];
            [ r setHTTPBody: [NSData dataWithBytes: postData length: postDataSize]];
        }

        int i;
        for (i = 0; i < numHeaders; ++i)
        {
            [ r setValue: [NSString stringWithUTF8String: headerValues[i]]
                forHTTPHeaderField: [NSString stringWithUTF8String: headerNames[i]]];
        }

        [[webView mainFrame] stopLoading ];
        [[webView mainFrame] loadRequest: r];
    }
    
    WebBrowserComponentInternal (const WebBrowserComponentInternal&);
    const WebBrowserComponentInternal& operator= (const WebBrowserComponentInternal&);
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent()
    : browser (0),
      associatedWindow (0),
      blankPageShown (false)
{
    setOpaque (true);
}

WebBrowserComponent::~WebBrowserComponent()
{
    deleteBrowser();
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

    if (browser != 0)
        browser->goToURL (url, headers, postData);
}

void WebBrowserComponent::stop()
{
    if (browser != 0)
        browser->stop();
}

void WebBrowserComponent::goBack()
{
    lastURL = String::empty;
    blankPageShown = false;

    if (browser != 0)
        browser->goBack();
}

void WebBrowserComponent::goForward()
{
    lastURL = String::empty;

    if (browser != 0)
        browser->goForward();
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
    if (browser == 0)
        g.fillAll (Colours::white);
}

void WebBrowserComponent::checkWindowAssociation()
{
    void* const window = getWindowHandle();

    if (window != associatedWindow 
         || (browser == 0 && window != 0))
    {
        associatedWindow = window;

        deleteBrowser();
        createBrowser();
    }

    if (browser != 0)
    {
        if (associatedWindow != 0 && isShowing())
        {
            browser->show();

            if (blankPageShown)
                goBack();
        }
        else
        {
            if (! blankPageShown)
            {
                // when the component becomes invisible, some stuff like flash 
                // carries on playing audio, so we need to force it onto a blank
                // page to avoid this..

                blankPageShown = true;
                browser->goToURL ("about:blank", 0, 0);
            }

            browser->hide();
        }
    }
}

void WebBrowserComponent::createBrowser()
{
    deleteBrowser();

    if (isShowing())
    {
        WebInitForCarbon();
        browser = new WebBrowserComponentInternal (this);
        reloadLastURL();
    }
}

void WebBrowserComponent::deleteBrowser()
{
    deleteAndZero (browser);
}

void WebBrowserComponent::reloadLastURL()
{
    if (lastURL.isNotEmpty())
    {
        goToURL (lastURL, &lastHeaders, &lastPostData);
        lastURL = String::empty;
    }
}

void WebBrowserComponent::updateBrowserPosition()
{
    if (getPeer() != 0 && browser != 0)
        browser->updateBounds();
}

void WebBrowserComponent::parentHierarchyChanged()
{
    checkWindowAssociation();
}

void WebBrowserComponent::moved()
{
    updateBrowserPosition();
}

void WebBrowserComponent::resized()
{
    updateBrowserPosition();
}

void WebBrowserComponent::visibilityChanged()
{
    checkWindowAssociation();
}

bool WebBrowserComponent::pageAboutToLoad (const String& url)
{
    return true;
}

END_JUCE_NAMESPACE
