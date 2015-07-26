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

#ifndef JUCE_TOOLTIPCLIENT_H_INCLUDED
#define JUCE_TOOLTIPCLIENT_H_INCLUDED


//==============================================================================
/**
    Components that want to use pop-up tooltips should implement this interface.

    A TooltipWindow will wait for the mouse to hover over a component that
    implements the TooltipClient interface, and when it finds one, it will display
    the tooltip returned by its getTooltip() method.

    @see TooltipWindow, SettableTooltipClient
*/
class JUCE_API  TooltipClient
{
public:
    /** Destructor. */
    virtual ~TooltipClient()  {}

    /** Returns the string that this object wants to show as its tooltip. */
    virtual String getTooltip() = 0;
};


//==============================================================================
/**
    An implementation of TooltipClient that stores the tooltip string and a method
    for changing it.

    This makes it easy to add a tooltip to a custom component, by simply adding this
    as a base class and calling setTooltip().

    Many of the Juce widgets already use this as a base class to implement their
    tooltips.

    @see TooltipClient, TooltipWindow
*/
class JUCE_API  SettableTooltipClient   : public TooltipClient
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~SettableTooltipClient()                                {}

    //==============================================================================
    /** Assigns a new tooltip to this object. */
    virtual void setTooltip (const String& newTooltip)              { tooltipString = newTooltip; }

    /** Returns the tooltip assigned to this object. */
    virtual String getTooltip()                                     { return tooltipString; }

protected:
    SettableTooltipClient() {}

private:
    String tooltipString;
};


#endif   // JUCE_TOOLTIPCLIENT_H_INCLUDED
