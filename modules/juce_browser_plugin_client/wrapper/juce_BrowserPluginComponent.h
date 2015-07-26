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

#ifndef JUCE_BROWSERPLUGINCOMPONENT_H_INCLUDED
#define JUCE_BROWSERPLUGINCOMPONENT_H_INCLUDED

//==============================================================================
/**
    Base class for a browser plugin object.

    You need to implement a createBrowserPlugin() function that the host will call
    when it needs a new instance of your BrowserPluginComponent subclass. The host will
    delete the BrowserPluginComponent later when the user navigates away from the
    page.
*/
class BrowserPluginComponent    : public Component
{
public:
    //==============================================================================
    /**
        Creates a browser plugin object.
        @see createBrowserPlugin
    */
    BrowserPluginComponent();

    /** Destructor. */
    ~BrowserPluginComponent();

    //==============================================================================
    /** Returns a string describing the host browser version.
    */
    String getBrowserVersion() const;

    /** Returns the URL that the browser is currently showing.
    */
    String getBrowserURL() const;

    /** The plugin must implement this method to return a variant object whose
        properties and methods can be accessed by javascript in the browser.

        If your plugin doesn't need to represent itself, you can just return
        a void var() object here.
    */
    virtual var getJavascriptObject() = 0;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrowserPluginComponent)
};

#endif   // JUCE_BROWSERPLUGINCOMPONENT_H_INCLUDED
