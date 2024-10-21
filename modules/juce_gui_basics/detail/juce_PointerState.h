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

namespace juce::detail
{

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

    [[nodiscard]] PointerState withPositionOffset (Point<float> x)        const noexcept { return with (&PointerState::position, position + x); }
    [[nodiscard]] PointerState withPosition (Point<float> x)              const noexcept { return with (&PointerState::position, x); }
    [[nodiscard]] PointerState withPressure (float x)                     const noexcept { return with (&PointerState::pressure, x); }
    [[nodiscard]] PointerState withOrientation (float x)                  const noexcept { return with (&PointerState::orientation, x); }
    [[nodiscard]] PointerState withRotation (float x)                     const noexcept { return with (&PointerState::rotation, x); }
    [[nodiscard]] PointerState withTiltX (float x)                        const noexcept { return with (&PointerState::tiltX, x); }
    [[nodiscard]] PointerState withTiltY (float x)                        const noexcept { return with (&PointerState::tiltY, x); }

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

} // namespace juce::detail
