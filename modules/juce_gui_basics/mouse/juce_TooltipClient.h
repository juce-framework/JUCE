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
    ~SettableTooltipClient() {}

    //==============================================================================
    /** Assigns a new tooltip to this object. */
    virtual void setTooltip (const String& newTooltip)              { tooltipString = newTooltip; }

    /** Returns the tooltip assigned to this object. */
    String getTooltip() override                                    { return tooltipString; }

protected:
    SettableTooltipClient() {}

private:
    String tooltipString;
};

} // namespace juce
