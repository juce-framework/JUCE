/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

    if (headers != nullptr)
        lastHeaders = *headers;
    else
        lastHeaders.clear();

    if (postData != nullptr)
        lastPostData = *postData;
    else
        lastPostData.reset();

    blankPageShown = false;
}

void WebBrowserComponent::stop()
{
}

void WebBrowserComponent::goBack()
{
    lastURL.clear();
    blankPageShown = false;

}

void WebBrowserComponent::goForward()
{
    lastURL.clear();

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
        lastURL.clear();
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

void WebBrowserComponent::focusGained (FocusChangeType)
{
}
