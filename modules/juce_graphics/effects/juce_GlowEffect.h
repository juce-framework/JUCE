/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component effect that adds a coloured blur around the component's contents.

    (This will only work on non-opaque components).

    @see Component::setComponentEffect, DropShadowEffect

    @tags{Graphics}
*/
class JUCE_API  GlowEffect  : public ImageEffectFilter
{
public:
    //==============================================================================
    /** Creates a default 'glow' effect.
        To customise its appearance, use the setGlowProperties() method.
    */
    GlowEffect();

    /** Destructor. */
    ~GlowEffect() override;

    //==============================================================================
    /** Sets the glow's radius and colour.

        The radius is how large the blur should be, and the colour is
        used to render it (for a less intense glow, lower the colour's
        opacity).
    */
    void setGlowProperties (float newRadius,
                            Colour newColour,
                            Point<int> offset = {});


    //==============================================================================
    /** @internal */
    void applyEffect (Image&, Graphics&, float scaleFactor, float alpha) override;

private:
    //==============================================================================
    float radius = 2.0f;
    Colour colour { Colours::white };
    Point<int> offset;

    JUCE_LEAK_DETECTOR (GlowEffect)
};

} // namespace juce
