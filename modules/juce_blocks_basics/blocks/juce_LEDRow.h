/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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
