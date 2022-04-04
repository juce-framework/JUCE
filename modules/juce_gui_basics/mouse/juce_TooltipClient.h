/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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

    @tags{GUI}
*/
class JUCE_API  TooltipClient
{
public:
    /** Destructor. */
    virtual ~TooltipClient() = default;

    /** Returns the string that this object wants to show as its tooltip. */
    virtual String getTooltip() = 0;
};


//==============================================================================
/**
    An implementation of TooltipClient that stores the tooltip string and a method
    for changing it.

    This makes it easy to add a tooltip to a custom component, by simply adding this
    as a base class and calling setTooltip().

    Many of the JUCE widgets already use this as a base class to implement their
    tooltips.

    @see TooltipClient, TooltipWindow

    @tags{GUI}
*/
class JUCE_API  SettableTooltipClient   : public TooltipClient
{
public:
    //==============================================================================
    /** Destructor. */
    ~SettableTooltipClient() override = default;

    //==============================================================================
    /** Assigns a new tooltip to this object. */
    virtual void setTooltip (const String& newTooltip)              { tooltipString = newTooltip; }

    /** Returns the tooltip assigned to this object. */
    String getTooltip() override                                    { return tooltipString; }

protected:
    SettableTooltipClient() = default;

private:
    String tooltipString;
};

} // namespace juce
