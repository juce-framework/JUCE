/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    Represents an LED strip on a device.
*/
class LEDRow
{
public:
    LEDRow (Block&);

    /** Destructor. */
    virtual ~LEDRow();

    //==============================================================================
    /** Return the number of LEDs in the row. */
    virtual int getNumLEDs() const = 0;

    /** Sets the colour of the corresponding LED. */
    virtual void setLEDColour (int index, LEDColour newColour) = 0;

    /** Overlays all LEDs with a single colour.

        Whilst the overlay is set subsequent calls to setLEDColour will happen
        *behind* the overlay, and will be invisible to the user until the
        overlay is removed.
    */
    virtual void setOverlayColour (LEDColour newColour) = 0;

    /* Removes an overlay colour. */
    virtual void resetOverlayColour() = 0;

    /** The device that these lights belong to. */
    Block& block;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDRow)
};

} // namespace juce
