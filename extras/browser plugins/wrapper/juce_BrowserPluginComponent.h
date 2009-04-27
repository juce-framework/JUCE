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

#ifndef __JUCE_BROWSERPLUGINCOMP_H__
#define __JUCE_BROWSERPLUGINCOMP_H__

#include "../../../../juce/juce_amalgamated.h"


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
    const String getBrowserVersion() const;

    /** The plugin must implement this method to return a variant object whose
        properties and methods can be accessed by javascript in the browser.
     
        If your plugin doesn't need to represent itself, you can just return
        a void var() object here.
    */
    virtual const var getJavascriptObject() = 0;

    //==============================================================================
    juce_UseDebuggingNewOperator
};


//==============================================================================
/**
    This function must be implemented somewhere in your code to create the actual 
    plugin object that you want to use.

    Obviously multiple instances may be used simultaneously, so be VERY cautious
    in your use of static variables!
*/
BrowserPluginComponent* JUCE_CALLTYPE createBrowserPlugin();


#endif
