/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#ifndef DOXYGEN

class PointerState
{
    auto tie() const noexcept
    {
        return std::tie (position, pressure, orientation, rotation, tiltX, tiltY);
    }

public:
    PointerState() = default;

    bool operator== (const PointerState& other) const noexcept   { return tie() == other.tie(); }
    bool operator!= (const PointerState& other) const noexcept   { return tie() != other.tie(); }

    JUCE_NODISCARD PointerState withPositionOffset (Point<float> x)        const noexcept { return with (&PointerState::position, position + x); }
    JUCE_NODISCARD PointerState withPosition (Point<float> x)              const noexcept { return with (&PointerState::position, x); }
    JUCE_NODISCARD PointerState withPressure (float x)                     const noexcept { return with (&PointerState::pressure, x); }
    JUCE_NODISCARD PointerState withOrientation (float x)                  const noexcept { return with (&PointerState::orientation, x); }
    JUCE_NODISCARD PointerState withRotation (float x)                     const noexcept { return with (&PointerState::rotation, x); }
    JUCE_NODISCARD PointerState withTiltX (float x)                        const noexcept { return with (&PointerState::tiltX, x); }
    JUCE_NODISCARD PointerState withTiltY (float x)                        const noexcept { return with (&PointerState::tiltY, x); }

    Point<float> position;
    float pressure    = MouseInputSource::defaultPressure;
    float orientation = MouseInputSource::defaultOrientation;
    float rotation    = MouseInputSource::defaultRotation;
    float tiltX       = MouseInputSource::defaultTiltX;
    float tiltY       = MouseInputSource::defaultTiltY;

    bool isPressureValid()      const noexcept        { return 0.0f <= pressure && pressure <= 1.0f; }
    bool isOrientationValid()   const noexcept        { return 0.0f <= orientation && orientation <= MathConstants<float>::twoPi; }
    bool isRotationValid()      const noexcept        { return 0.0f <= rotation && rotation <= MathConstants<float>::twoPi; }
    bool isTiltValid (bool isX) const noexcept
    {
        return isX ? (-1.0f <= tiltX && tiltX <= 1.0f)
                   : (-1.0f <= tiltY && tiltY <= 1.0f);
    }

private:
    template <typename Value>
    PointerState with (Value PointerState::* member, Value item) const
    {
        auto copy = *this;
        copy.*member = std::move (item);
        return copy;
    }
};

inline auto makeMouseEvent (MouseInputSource source,
                            const PointerState& ps,
                            ModifierKeys modifiers,
                            Component* eventComponent,
                            Component* originator,
                            Time eventTime,
                            Point<float> mouseDownPos,
                            Time mouseDownTime,
                            int numberOfClicks,
                            bool mouseWasDragged)
{
    return MouseEvent (source,
                       ps.position,
                       modifiers,
                       ps.pressure,
                       ps.orientation,
                       ps.rotation,
                       ps.tiltX,
                       ps.tiltY,
                       eventComponent,
                       originator,
                       eventTime,
                       mouseDownPos,
                       mouseDownTime,
                       numberOfClicks,
                       mouseWasDragged);
}


#endif

} // namespace juce
