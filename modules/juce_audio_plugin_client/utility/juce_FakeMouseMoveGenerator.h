/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_FAKEMOUSEMOVEGENERATOR_H_INCLUDED
#define JUCE_FAKEMOUSEMOVEGENERATOR_H_INCLUDED

#if JUCE_MAC

//==============================================================================
// Helper class to workaround windows not getting mouse-moves...
class FakeMouseMoveGenerator  : private Timer
{
public:
    FakeMouseMoveGenerator()
    {
        startTimer (1000 / 30);
    }

    void timerCallback() override
    {
        // Workaround for windows not getting mouse-moves...
        const Point<float> screenPos (Desktop::getInstance().getMainMouseSource().getScreenPosition());

        if (screenPos != lastScreenPos)
        {
            lastScreenPos = screenPos;
            const ModifierKeys mods (ModifierKeys::getCurrentModifiers());

            if (! mods.isAnyMouseButtonDown())
                if (Component* const comp = Desktop::getInstance().findComponentAt (screenPos.roundToInt()))
                    if (ComponentPeer* const peer = comp->getPeer())
                        if (! peer->isFocused())
                            peer->handleMouseEvent (0, peer->globalToLocal (screenPos), mods,
                                                    MouseInputSource::invalidPressure, Time::currentTimeMillis());
        }
    }

private:
    Point<float> lastScreenPos;
};

#else
struct FakeMouseMoveGenerator {};
#endif

#endif   // JUCE_FAKEMOUSEMOVEGENERATOR_H_INCLUDED
