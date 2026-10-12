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

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             MultiTouchDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases multi-touch features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, vs2026, linux_make, androidstudio,
                   xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        MultiTouchDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class MultiTouchDemo final : public Component,
                             private ComponentMovementWatcher
{
public:
    class InputSource
    {
    public:
        explicit InputSource (const MouseInputSource& src)
            : source (src)
        {}

        Colour getColour() const
        {
            return colour;
        }

        const MouseInputSource& getSource() const
        {
            return source;
        }

        Point<float> getPosition() const
        {
            return position;
        }

        void setPosition (Point<float> pos)
        {
            position = pos;
        }

        const Path& getTrail() const
        {
            return trail;
        }

        void addPointToTrail (Point<float> point, float pressure)
        {
            if (! lastTrailPoint.has_value())
            {
                lastTrailPoint = point;
                return;
            }

            if (point.getDistanceFrom (*lastTrailPoint) > 5.0f)
            {
                Path newSegment;
                newSegment.startNewSubPath (*lastTrailPoint);
                newSegment.lineTo (point);

                auto diameter = 20.0f * (pressure > 0 && pressure < 1.0f ? pressure : 1.0f);

                PathStrokeType (diameter, PathStrokeType::curved, PathStrokeType::rounded).createStrokedPath (newSegment, newSegment);
                newSegment.closeSubPath();

                trail.addPath (newSegment);

                lastTrailPoint = point;
            }
        }

        void clearTrail()
        {
            trail.clear();
            lastTrailPoint.reset();
        }

    private:
        MouseInputSource source;
        Point<float> position;
        std::optional<Point<float>> lastTrailPoint;
        Path trail;
        Colour colour { getRandomBrightColour().withAlpha (0.6f) };
    };

    MultiTouchDemo()
        : ComponentMovementWatcher (this)
    {
        setOpaque (true);

        initSize();
    }

    ~MultiTouchDemo() override
    {
        setUsingWindowsMultiTouch (false);
    }

    void componentMovedOrResized (bool, bool) override {}

    void setUsingWindowsMultiTouch (bool shouldUseMultiTouch)
    {
        auto* peer = getPeer();

        if (peer == nullptr)
            return;

        auto* window = dynamic_cast<TopLevelWindow*> (&peer->getComponent());

        if (window == nullptr)
            return;

        window->setUsingWindowsMultiTouch (shouldUseMultiTouch);
    }

    void componentPeerChanged() override
    {
        setUsingWindowsMultiTouch (true);
        initSize();
    }

    void componentVisibilityChanged() override {}

    using ComponentMovementWatcher::componentVisibilityChanged;
    using ComponentMovementWatcher::componentMovedOrResized;

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colour::greyLevel (0.4f)));

        g.setColour (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::defaultText,
                                             Colours::lightgrey));
        g.setFont (14.0f);
        g.drawFittedText ("Drag here with as many fingers as you have!",
                          getLocalBounds().reduced (30), Justification::centred, 4);

        for (const auto& source : sources)
        {
            g.setColour (source->getColour());

            const auto& trail = source->getTrail();

            if (! trail.isEmpty())
            {
                g.fillPath (trail);

                const auto radius = 40.0f;
                const auto lastPathPoint = trail.getCurrentPosition();

                g.fillEllipse (lastPathPoint.x - radius,
                               lastPathPoint.y - radius,
                               radius * 2.0f,
                               radius * 2.0f);
            }

            const auto pos = source->getPosition();
            const auto radius = 30.0f;
            g.drawEllipse (pos.x - radius,
                           pos.y - radius,
                           radius * 2.0f,
                           radius * 2.0f,
                           2.0f);

            g.setFont (14.0f);

            const auto& mouseSource = source->getSource();

            auto desc = std::invoke ([&mouseSource]() -> String
            {
                switch (mouseSource.getType())
                {
                    case (MouseInputSource::InputSourceType::mouse): return "Mouse";
                    case (MouseInputSource::InputSourceType::touch): return "Touch";
                    case (MouseInputSource::InputSourceType::pen):   return "Pen";
                }

                jassertfalse;
                return "Unknown";
            });

            desc << " #" << mouseSource.getIndex();

            auto pressure = mouseSource.getCurrentPressure();

            if (pressure > 0.0f && pressure < 1.0f)
                desc << "  (pressure: " << (int) (pressure * 100.0f) << "%)";

            const auto modifierKeys = mouseSource.getCurrentModifiers();

            if (modifierKeys.isCommandDown()) desc << " (CMD)";
            if (modifierKeys.isShiftDown())   desc << " (SHIFT)";
            if (modifierKeys.isCtrlDown())    desc << " (CTRL)";
            if (modifierKeys.isAltDown())     desc << " (ALT)";

            const auto labelPos = trail.isEmpty() ? pos : trail.getCurrentPosition();

            g.drawText (desc,
                        Rectangle<int> ((int) labelPos.x - 200,
                                        (int) labelPos.y - 60,
                                        400, 20),
                        Justification::centredTop, false);
        }
    }

    void mouseEnter (const MouseEvent& e) override
    {
        auto& source = getInputSource (e);
        source.setPosition (e.position);
        repaint();
    }

    void mouseExit (const MouseEvent& e) override
    {
        removeInputSource (e);
        repaint();
    }

    void mouseMove (const MouseEvent& e) override
    {
        auto& source = getInputSource (e);
        source.setPosition (e.position);
        repaint();
    }

    void mouseDrag (const MouseEvent& e) override
    {
        auto& source = getInputSource (e);
        source.addPointToTrail (e.position, e.pressure);
        repaint();
    }

    void mouseUp (const MouseEvent& e) override
    {
        auto& source = getInputSource (e);
        source.setPosition (e.position);
        source.clearTrail();
        repaint();
    }

    static auto sourceMatches (const MouseInputSource& source)
    {
        return [&source] (const auto& x) { return x->getSource() == source; };
    }

    InputSource& getInputSource (const MouseEvent& e)
    {
        const auto it = std::find_if (sources.begin(), sources.end(), sourceMatches (e.source));
        return it != sources.end() ? **it : *sources.emplace_back (std::make_unique<InputSource> (e.source));
    }

    void removeInputSource (const MouseEvent& e)
    {
        sources.erase (std::remove_if (sources.begin(), sources.end(), sourceMatches (e.source)),
                       sources.end());
    }

private:
    void initSize()
    {
        setSize (500, 500);
    }

    std::vector<std::unique_ptr<InputSource>> sources;
};
