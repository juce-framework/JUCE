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

#ifndef __JUCE_TOOLTIPCLIENT_JUCEHEADER__
#define __JUCE_TOOLTIPCLIENT_JUCEHEADER__


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
    virtual const String getTooltip() = 0;
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
    virtual void setTooltip (const String& newTooltip)              { tooltipString = newTooltip; }

    virtual const String getTooltip()                               { return tooltipString; }


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    String tooltipString;
};



#endif   // __JUCE_TOOLTIPCLIENT_JUCEHEADER__
