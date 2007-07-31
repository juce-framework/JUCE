/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MidiFile.h"
#include "../../../juce_core/io/streams/juce_MemoryOutputStream.h"


//==============================================================================
struct TempoInfo
{
    double bpm, timestamp;
};

struct TimeSigInfo
{
    int numerator, denominator;
    double timestamp;
};

MidiFile::MidiFile() throw()
   : numTracks (0),
     timeFormat ((short)(unsigned short)0xe728)
{
}

MidiFile::~MidiFile() throw()
{
    clear();
}

void MidiFile::clear() throw()
{
    while (numTracks > 0)
        delete tracks [--numTracks];
}

//==============================================================================
int MidiFile::getNumTracks() const throw()
{
    return numTracks;
}

const MidiMessageSequence* MidiFile::getTrack (const int index) const throw()
{
    return (index >= 0 && index < numTracks) ? tracks[index] : 0;
}

void MidiFile::addTrack (const MidiMessageSequence& trackSequence) throw()
{
    jassert (numTracks < numElementsInArray (tracks));

    if (numTracks < numElementsInArray (tracks))
        tracks [numTracks++] = new MidiMessageSequence (trackSequence);
}

//==============================================================================
short MidiFile::getTimeFormat() const throw()
{
    return timeFormat;
}

void MidiFile::setTicksPerQuarterNote (const int ticks) throw()
{
    timeFormat = (short)ticks;
}

void MidiFile::setSmpteTimeFormat (const int framesPerSecond,
                                   const int subframeResolution) throw()
{
    timeFormat = (short) (((-framesPerSecond) << 8) | subframeResolution);
}

//==============================================================================
void MidiFile::findAllTempoEvents (MidiMessageSequence& tempoChangeEvents) const
{
    for (int i = numTracks; --i >= 0;)
    {
        const int numEvents = tracks[i]->getNumEvents();

        for (int j = 0; j < numEvents; ++j)
        {
            const MidiMessage& m = tracks[i]->getEventPointer (j)->message;

            if (m.isTempoMetaEvent())
                tempoChangeEvents.addEvent (m);
        }
    }
}

void MidiFile::findAllTimeSigEvents (MidiMessageSequence& timeSigEvents) const
{
    for (int i = numTracks; --i >= 0;)
    {
        const int numEvents = tracks[i]->getNumEvents();

        for (int j = 0; j < numEvents; ++j)
        {
            const MidiMessage& m = tracks[i]->getEventPointer (j)->message;

            if (m.isTimeSignatureMetaEvent())
                timeSigEvents.addEvent (m);
        }
    }
}

double MidiFile::getLastTimestamp() const
{
    double t = 0.0;

    for (int i = numTracks; --i >= 0;)
        t = jmax (t, tracks[i]->getEndTime());

    return t;
}

//==============================================================================
static bool parseMidiHeader (const char* &data,
                             short& timeFormat,
                             short& fileType,
                             short& numberOfTracks)
{
    unsigned int ch = (int) bigEndianInt (data);
    data += 4;

    if (ch != bigEndianInt ("MThd"))
    {
        bool ok = false;

        if (ch == bigEndianInt ("RIFF"))
        {
            for (int i = 0; i < 8; ++i)
            {
                ch = bigEndianInt (data);
                data += 4;

                if (ch == bigEndianInt ("MThd"))
                {
                    ok = true;
                    break;
                }
            }
        }

        if (! ok)
            return false;
    }

    unsigned int bytesRemaining = bigEndianInt (data);
    data += 4;
    fileType = (short)bigEndianShort (data);
    data += 2;
    numberOfTracks = (short)bigEndianShort (data);
    data += 2;
    timeFormat = (short)bigEndianShort (data);
    data += 2;
    bytesRemaining -= 6;
    data += bytesRemaining;

    return true;
}

bool MidiFile::readFrom (InputStream& sourceStream)
{
    clear();
    MemoryBlock data;

    const int maxSensibleMidiFileSize = 2 * 1024 * 1024;

    // (put a sanity-check on the file size, as midi files are generally small)
    if (sourceStream.readIntoMemoryBlock (data, maxSensibleMidiFileSize))
    {
        int size = data.getSize();
        const char* d = (char*) data.getData();
        short fileType, expectedTracks;

        if (size > 16 && parseMidiHeader (d, timeFormat, fileType, expectedTracks))
        {
            size -= (int) (d - (char*) data.getData());

            int track = 0;

            while (size > 0 && track < expectedTracks)
            {
                const int chunkType = (int)bigEndianInt (d);
                d += 4;
                const int chunkSize = (int)bigEndianInt (d);
                d += 4;

                if (chunkSize <= 0)
                    break;

                if (size < 0)
                    return false;

                if (chunkType == (int)bigEndianInt ("MTrk"))
                {
                    readNextTrack (d, chunkSize);
                }

                size -= chunkSize + 8;
                d += chunkSize;
                ++track;
            }

            return true;
        }
    }

    return false;
}

// a comparator that puts all the note-offs before note-ons that have the same time
int MidiFile::compareElements (const MidiMessageSequence::MidiEventHolder* const first,
                               const MidiMessageSequence::MidiEventHolder* const second) throw()
{
    const double diff = (first->message.getTimeStamp() - second->message.getTimeStamp());

    if (diff == 0)
    {
        if (first->message.isNoteOff() && second->message.isNoteOn())
            return -1;
        else if (first->message.isNoteOn() && second->message.isNoteOff())
            return 1;
        else
            return 0;
    }
    else
    {
        return (diff > 0) ? 1 : -1;
    }
}

void MidiFile::readNextTrack (const char* data, int size)
{
    double time = 0;
    char lastStatusByte = 0;

    MidiMessageSequence result;

    while (size > 0)
    {
        int bytesUsed;
        const int delay = MidiMessage::readVariableLengthVal ((const uint8*) data, bytesUsed);
        data += bytesUsed;
        size -= bytesUsed;
        time += delay;

        int messSize = 0;
        const MidiMessage mm ((const uint8*) data, size, messSize, lastStatusByte, time);
        size -= messSize;
        data += messSize;

        result.addEvent (mm);

        const char firstByte = *(mm.getRawData());
        if ((firstByte & 0xf0) != 0xf0)
            lastStatusByte = firstByte;
    }

    // use a sort that puts all the note-offs before note-ons that have the same time
    result.list.sort (*this, true);

    result.updateMatchedPairs();

    addTrack (result);
}

//==============================================================================
static double convertTicksToSeconds (const double time,
                                     const MidiMessageSequence& tempoEvents,
                                     const int timeFormat)
{
    if (timeFormat > 0)
    {
        int numer = 4, denom = 4;
        double tempoTime = 0.0, correctedTempoTime = 0.0;
        const double tickLen = 1.0 / (timeFormat & 0x7fff);
        double secsPerTick = 0.5 * tickLen;
        const int numEvents = tempoEvents.getNumEvents();

        for (int i = 0; i < numEvents; ++i)
        {
            const MidiMessage& m = tempoEvents.getEventPointer(i)->message;

            if (time <= m.getTimeStamp())
                break;

            if (timeFormat > 0)
            {
                correctedTempoTime = correctedTempoTime
                                        + (m.getTimeStamp() - tempoTime) * secsPerTick;
            }
            else
            {
                correctedTempoTime = tickLen * m.getTimeStamp() / (((timeFormat & 0x7fff) >> 8) * (timeFormat & 0xff));
            }

            tempoTime = m.getTimeStamp();

            if (m.isTempoMetaEvent())
                secsPerTick = tickLen * m.getTempoSecondsPerQuarterNote();
            else if (m.isTimeSignatureMetaEvent())
                m.getTimeSignatureInfo (numer, denom);

            while (i + 1 < numEvents)
            {
                const MidiMessage& m2 = tempoEvents.getEventPointer(i + 1)->message;
                if (m2.getTimeStamp() == tempoTime)
                {
                    ++i;

                    if (m2.isTempoMetaEvent())
                        secsPerTick = tickLen * m2.getTempoSecondsPerQuarterNote();
                    else if (m2.isTimeSignatureMetaEvent())
                        m2.getTimeSignatureInfo (numer, denom);
                }
                else
                {
                    break;
                }
            }

        }

        return correctedTempoTime + (time - tempoTime) * secsPerTick;
    }
    else
    {
        return time / (((timeFormat & 0x7fff) >> 8) * (timeFormat & 0xff));
    }
}

void MidiFile::convertTimestampTicksToSeconds()
{
    MidiMessageSequence tempoEvents;
    findAllTempoEvents (tempoEvents);
    findAllTimeSigEvents (tempoEvents);

    for (int i = 0; i < numTracks; ++i)
    {
        MidiMessageSequence& ms = *tracks[i];

        for (int j = ms.getNumEvents(); --j >= 0;)
        {
            MidiMessage& m = ms.getEventPointer(j)->message;

            m.setTimeStamp (convertTicksToSeconds (m.getTimeStamp(),
                                                   tempoEvents,
                                                   timeFormat));
        }
    }
}

//==============================================================================
static void writeVariableLengthInt (OutputStream& out, unsigned int v)
{
    unsigned int buffer = v & 0x7F;

    while ((v >>= 7) != 0)
    {
        buffer <<= 8;
        buffer |= ((v & 0x7F) | 0x80);
    }

    for (;;)
    {
        out.writeByte ((char) buffer);

        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}

bool MidiFile::writeTo (OutputStream& out)
{
    out.writeIntBigEndian ((int) bigEndianInt ("MThd"));
    out.writeIntBigEndian (6);
    out.writeShortBigEndian (1); // type
    out.writeShortBigEndian (numTracks);
    out.writeShortBigEndian (timeFormat);

    for (int i = 0; i < numTracks; ++i)
        writeTrack (out, i);

    out.flush();

    return true;
}


//==============================================================================
void MidiFile::writeTrack (OutputStream& mainOut,
                           const int trackNum)
{
    MemoryOutputStream out;

    const MidiMessageSequence& ms = *tracks[trackNum];

    int lastTick = 0;
    char lastStatusByte = 0;

    for (int i = 0; i < ms.getNumEvents(); ++i)
    {
        const MidiMessage& mm = ms.getEventPointer(i)->message;

        const int tick = roundDoubleToInt (mm.getTimeStamp());
        const int delta = jmax (0, tick - lastTick);
        writeVariableLengthInt (out, delta);
        lastTick = tick;

        const char statusByte = *(mm.getRawData());

        if ((statusByte == lastStatusByte)
             && ((statusByte & 0xf0) != 0xf0)
             && i > 0
             && mm.getRawDataSize() > 1)
        {
            out.write (mm.getRawData() + 1, mm.getRawDataSize() - 1);
        }
        else
        {
            out.write (mm.getRawData(), mm.getRawDataSize());
        }

        lastStatusByte = statusByte;
    }

    out.writeByte (0);
    const MidiMessage m (MidiMessage::endOfTrack());
    out.write (m.getRawData(),
               m.getRawDataSize());

    mainOut.writeIntBigEndian ((int)bigEndianInt ("MTrk"));
    mainOut.writeIntBigEndian (out.getDataSize());
    mainOut.write (out.getData(), out.getDataSize());
}

END_JUCE_NAMESPACE
