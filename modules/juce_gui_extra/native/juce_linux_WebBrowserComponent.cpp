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

/*
    Sorry.. This class isn't implemented on Linux!
*/

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const bool unloadPageWhenBrowserIsHidden_)
    : browser (0),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    setOpaque (true);
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

    lastHeaders.clear();
    if (headers != nullptr)
        lastHeaders = *headers;

    lastPostData.setSize (0);
    if (postData != nullptr)
        lastPostData = *postData;

    blankPageShown = false;


}

void WebBrowserComponent::stop()
{
}

void WebBrowserComponent::goBack()
{
    lastURL = String::empty;
    blankPageShown = false;

}

void WebBrowserComponent::goForward()
{
    lastURL = String::empty;

}

void WebBrowserComponent::refresh()
{
}

//==============================================================================
void WebBrowserComponent::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void WebBrowserComponent::checkWindowAssociation()
{
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
}

void WebBrowserComponent::visibilityChanged()
{
    checkWindowAssociation();
}

bool WebBrowserComponent::pageAboutToLoad (const String&)  { return true; }
void WebBrowserComponent::pageFinishedLoading (const String&) {}
