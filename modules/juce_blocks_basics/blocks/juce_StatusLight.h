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
    Represents a status LED on a device.
*/
class StatusLight
{
public:
    StatusLight (Block&);

    /** Destructor. */
    virtual ~StatusLight();

    //==============================================================================
    /** Returns a name to describe this light. */
    virtual juce::String getName() const = 0;

    /** Changes the light's colour. */
    virtual bool setColour (LEDColour newColour) = 0;

    /** The device that this LED belongs to. */
    Block& block;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StatusLight)
};
