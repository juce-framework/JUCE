/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_MIDIDATACONCATENATOR_JUCEHEADER__
#define __JUCE_MIDIDATACONCATENATOR_JUCEHEADER__

//==============================================================================
/**
    Helper class that takes chunks of incoming midi bytes, packages them into
    messages, and dispatches them to a midi callback.
*/
class MidiDataConcatenator
{
public:
    //==============================================================================
    MidiDataConcatenator (const int initialBufferSize)
        : pendingData ((size_t) initialBufferSize),
          pendingBytes (0), pendingDataTime (0)
    {
    }

    void reset()
    {
        pendingBytes = 0;
        pendingDataTime = 0;
    }

    void pushMidiData (const void* data, int numBytes, double time,
                       MidiInput* input, MidiInputCallback& callback)
    {
        const uint8* d = static_cast <const uint8*> (data);

        while (numBytes > 0)
        {
            if (pendingBytes > 0 || d[0] == 0xf0)
            {
                processSysex (d, numBytes, time, input, callback);
            }
            else
            {
                int used = 0;
                const MidiMessage m (d, numBytes, used, 0, time);

                if (used <= 0)
                    break; // malformed message..

                callback.handleIncomingMidiMessage (input, m);
                numBytes -= used;
                d += used;
            }
        }
    }

private:
    void processSysex (const uint8*& d, int& numBytes, double time,
                       MidiInput* input, MidiInputCallback& callback)
    {
        if (*d == 0xf0)
        {
            pendingBytes = 0;
            pendingDataTime = time;
        }

        pendingData.ensureSize ((size_t) (pendingBytes + numBytes), false);
        uint8* totalMessage = static_cast<uint8*> (pendingData.getData());
        uint8* dest = totalMessage + pendingBytes;

        do
        {
            if (pendingBytes > 0 && *d >= 0x80)
            {
                if (*d >= 0xfa || *d == 0xf8)
                {
                    callback.handleIncomingMidiMessage (input, MidiMessage (*d, time));
                    ++d;
                    --numBytes;
                }
                else
                {
                    if (*d == 0xf7)
                    {
                        *dest++ = *d++;
                        pendingBytes++;
                        --numBytes;
                    }

                    break;
                }
            }
            else
            {
                *dest++ = *d++;
                pendingBytes++;
                --numBytes;
            }
        }
        while (numBytes > 0);

        if (pendingBytes > 0)
        {
            if (totalMessage [pendingBytes - 1] == 0xf7)
            {
                callback.handleIncomingMidiMessage (input, MidiMessage (totalMessage, pendingBytes, pendingDataTime));
                pendingBytes = 0;
            }
            else
            {
                callback.handlePartialSysexMessage (input, totalMessage, pendingBytes, pendingDataTime);
            }
        }
    }

    MemoryBlock pendingData;
    int pendingBytes;
    double pendingDataTime;

    JUCE_DECLARE_NON_COPYABLE (MidiDataConcatenator);
};

#endif   // __JUCE_MIDIDATACONCATENATOR_JUCEHEADER__
