/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
#define __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__

#if JUCE_WEB_BROWSER || DOXYGEN

//==============================================================================
/**
    A component that displays an embedded web browser.

    The browser itself will be platform-dependent. On the Mac, probably Safari, on
    Windows, probably IE.

*/
class JUCE_API  WebBrowserComponent      : public Component
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
                  const StringArray* headers = nullptr,
                  const MemoryBlock* postData = nullptr);

    /** Stops the current page loading.
    */
    void stop();

    /** Sends the browser back one page.
    */
    void goBack();

    /** Sends the browser forward one page.
    */
    void goForward();

    /** Refreshes the browser.
    */
    void refresh();

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

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    void parentHierarchyChanged();
    /** @internal */
    void visibilityChanged();

private:
    //==============================================================================
    class Pimpl;
    Pimpl* browser;
    bool blankPageShown, unloadPageWhenBrowserIsHidden;
    String lastURL;
    StringArray lastHeaders;
    MemoryBlock lastPostData;

    void reloadLastURL();
    void checkWindowAssociation();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserComponent)
};


#endif
#endif   // __JUCE_WEBBROWSERCOMPONENT_JUCEHEADER__
