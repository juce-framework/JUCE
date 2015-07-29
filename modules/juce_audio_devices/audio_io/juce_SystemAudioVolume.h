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

#ifndef JUCE_SYSTEMAUDIOVOLUME_H_INCLUDED
#define JUCE_SYSTEMAUDIOVOLUME_H_INCLUDED


//==============================================================================
/**
    Contains functions to control the system's master volume.
*/
class JUCE_API  SystemAudioVolume
{
public:
    //==============================================================================
    /** Returns the operating system's current volume level in the range 0 to 1.0 */
    static float JUCE_CALLTYPE getGain();

    /** Attempts to set the operating system's current volume level.
        @param newGain  the level, between 0 and 1.0
        @returns true if the operation succeeds
    */
    static bool JUCE_CALLTYPE setGain (float newGain);

    /** Returns true if the system's audio output is currently muted. */
    static bool JUCE_CALLTYPE isMuted();

    /** Attempts to mute the operating system's audio output.
        @param shouldBeMuted    true if you want it to be muted
        @returns true if the operation succeeds
    */
    static bool JUCE_CALLTYPE setMuted (bool shouldBeMuted);

private:
    SystemAudioVolume(); // Don't instantiate this class, just call its static fns.
    JUCE_DECLARE_NON_COPYABLE (SystemAudioVolume)
};


#endif   // JUCE_SYSTEMAUDIOVOLUME_H_INCLUDED
