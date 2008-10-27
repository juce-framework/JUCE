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

#ifndef __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
#define __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__

#include "../juce_Component.h"

#if JUCE_WEB_BROWSER

class WebBrowserComponentInternal;


//==============================================================================
/**
    A component that displays an embedded web browser.

    The browser itself will be platform-dependent. On the Mac, probably Safari, on
    Windows, probably IE.

*/
class WebBrowserComponent      : public Component
{
public:
    //==============================================================================
    /** Creates a WebBrowserComponent.

        Once it's created and visible, send the browser to a URL using goToURL().
    */
    WebBrowserComponent();

    /** Destructor. */
    ~WebBrowserComponent();

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
                  const StringArray* headers = 0,
                  const MemoryBlock* postData = 0);

    /** Stops the current page loading.
    */
    void stop();

    /** Sends the browser back one page.
    */
    void goBack();

    /** Sends the browser forward one page.
    */
    void goForward();


    //==============================================================================
    /** This callback is called when the browser is about to navigate
        to a new location.

        You can override this method to perform some action when the user
        tries to go to a particular URL. To allow the operation to carry on,
        return true, or return false to stop the navigation happening.
    */
    virtual bool pageAboutToLoad (const String& newURL);

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    void parentHierarchyChanged();
    /** @internal */
    void visibilityChanged();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    WebBrowserComponentInternal* browser;
    bool blankPageShown;

    String lastURL;
    StringArray lastHeaders;
    MemoryBlock lastPostData;

    void reloadLastURL();
    void checkWindowAssociation();

    WebBrowserComponent (const WebBrowserComponent&);
    const WebBrowserComponent& operator= (const WebBrowserComponent&);
};


#endif
#endif   // __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
