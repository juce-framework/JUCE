/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

WebBrowserComponent::WebBrowserComponent (const bool unloadPageWhenBrowserIsHidden_)
    : browser (nullptr),
      blankPageShown (false),
      unloadPageWhenBrowserIsHidden (unloadPageWhenBrowserIsHidden_)
{
    // Unfortunately, WebBrowserComponent is not implemented for Android yet!
    // This is just a stub implementation without any useful functionality.
    jassertfalse;

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

void WebBrowserComponent::clearCookies()
{
}

} // namespace juce
