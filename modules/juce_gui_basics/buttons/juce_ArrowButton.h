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
    A button with an arrow in it.

    @see Button

    @tags{GUI}
*/
class JUCE_API  ArrowButton  : public Button
{
public:
    //==============================================================================
    /** Creates an ArrowButton.

        @param buttonName       the name to give the button
        @param arrowDirection   the direction the arrow should point in, where 0.0 is
                                pointing right, 0.25 is down, 0.5 is left, 0.75 is up
        @param arrowColour      the colour to use for the arrow
    */
    ArrowButton (const String& buttonName,
                 float arrowDirection,
                 Colour arrowColour);

    /** Destructor. */
    ~ArrowButton() override;

    /** @internal */
    void paintButton (Graphics&, bool, bool) override;

private:
    Colour colour;
    Path path;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrowButton)
};

} // namespace juce
