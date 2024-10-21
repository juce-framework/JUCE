/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#pragma once

class ADSRComponent final : public Component
{
public:
    ADSRComponent()
        : envelope { *this }
    {
        for (Slider* slider : { &adsrAttack, &adsrDecay, &adsrSustain, &adsrRelease })
        {
            if (slider == &adsrSustain)
            {
                slider->textFromValueFunction = [slider] (double value)
                {
                    String text;

                    text << slider->getName();

                    const auto val = (int) jmap (value, 0.0, 1.0, 0.0, 100.0);
                    text << String::formatted (": %d%%", val);

                    return text;
                };
            }
            else
            {
                slider->textFromValueFunction = [slider] (double value)
                {
                    String text;

                    text << slider->getName();

                    text << ": " << ((value < 0.4f) ? String::formatted ("%dms", (int) std::round (value * 1000))
                                                    : String::formatted ("%0.2lf Sec", value));

                    return text;
                };

                slider->setSkewFactor (0.3);
            }

            slider->setRange (0, 1);
            slider->setTextBoxStyle (Slider::TextBoxBelow, true, 300, 25);
            slider->onValueChange = [this]
            {
                NullCheckedInvocation::invoke (onChange);
                repaint();
            };

            addAndMakeVisible (slider);
        }

        adsrAttack.setName ("Attack");
        adsrDecay.setName ("Decay");
        adsrSustain.setName ("Sustain");
        adsrRelease.setName ("Release");

        adsrAttack.setValue (0.1, dontSendNotification);
        adsrDecay.setValue (0.3, dontSendNotification);
        adsrSustain.setValue (0.3, dontSendNotification);
        adsrRelease.setValue (0.2, dontSendNotification);

        addAndMakeVisible (envelope);
    }

    std::function<void()> onChange;

    ADSR::Parameters getParameters() const
    {
        return
        {
            (float) adsrAttack.getValue(),
            (float) adsrDecay.getValue(),
            (float) adsrSustain.getValue(),
            (float) adsrRelease.getValue(),
        };
    }

    void resized() final
    {
        auto bounds = getLocalBounds();

        const auto knobWidth = bounds.getWidth() / 4;
        auto knobBounds = bounds.removeFromBottom (bounds.getHeight() / 2);
        {
            adsrAttack.setBounds (knobBounds.removeFromLeft (knobWidth));
            adsrDecay.setBounds (knobBounds.removeFromLeft (knobWidth));
            adsrSustain.setBounds (knobBounds.removeFromLeft (knobWidth));
            adsrRelease.setBounds (knobBounds.removeFromLeft (knobWidth));
        }

        envelope.setBounds (bounds);
    }

    Slider adsrAttack  { Slider::RotaryVerticalDrag, Slider::TextBoxBelow };
    Slider adsrDecay   { Slider::RotaryVerticalDrag, Slider::TextBoxBelow };
    Slider adsrSustain { Slider::RotaryVerticalDrag, Slider::TextBoxBelow };
    Slider adsrRelease { Slider::RotaryVerticalDrag, Slider::TextBoxBelow };

private:
    class Envelope final : public Component
    {
    public:
        Envelope (ADSRComponent& adsr) : parent { adsr } {}

        void paint (Graphics& g) final
        {
            const auto env = parent.getParameters();

            // sustain isn't a length but we use a fixed value here to give
            // sustain some visual width in the envelope
            constexpr auto sustainLength = 0.1;

            const auto adsrLength = env.attack
                                  + env.decay
                                  + sustainLength
                                  + env.release;

            auto bounds = getLocalBounds().toFloat();

            const auto attackWidth   = bounds.proportionOfWidth (env.attack    / adsrLength);
            const auto decayWidth    = bounds.proportionOfWidth (env.decay     / adsrLength);
            const auto sustainWidth  = bounds.proportionOfWidth (sustainLength / adsrLength);
            const auto releaseWidth  = bounds.proportionOfWidth (env.release   / adsrLength);
            const auto sustainHeight = bounds.proportionOfHeight (1 - env.sustain);

            const auto attackBounds  = bounds.removeFromLeft (attackWidth);
            const auto decayBounds   = bounds.removeFromLeft (decayWidth);
            const auto sustainBounds = bounds.removeFromLeft (sustainWidth);
            const auto releaseBounds = bounds.removeFromLeft (releaseWidth);

            g.setColour (Colours::black.withAlpha (0.1f));
            g.fillRect (bounds);

            const auto alpha = 0.4f;

            g.setColour (Colour (246, 98, 92).withAlpha (alpha));
            g.fillRect (attackBounds);

            g.setColour (Colour (242, 187, 60).withAlpha (alpha));
            g.fillRect (decayBounds);

            g.setColour (Colour (109, 234, 166).withAlpha (alpha));
            g.fillRect (sustainBounds);

            g.setColour (Colour (131, 61, 183).withAlpha (alpha));
            g.fillRect (releaseBounds);

            Path envelopePath;
            envelopePath.startNewSubPath (attackBounds.getBottomLeft());
            envelopePath.lineTo (decayBounds.getTopLeft());
            envelopePath.lineTo (sustainBounds.getX(), sustainHeight);
            envelopePath.lineTo (releaseBounds.getX(), sustainHeight);
            envelopePath.lineTo (releaseBounds.getBottomRight());

            const auto lineThickness = 4.0f;

            g.setColour (Colours::white);
            g.strokePath (envelopePath, PathStrokeType { lineThickness });
        }

    private:
        ADSRComponent& parent;
    };

    Envelope envelope;
};
