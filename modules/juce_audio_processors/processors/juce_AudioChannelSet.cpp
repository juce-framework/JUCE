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

AudioChannelSet::AudioChannelSet (uint32 c) : channels (c) {}

bool AudioChannelSet::operator== (const AudioChannelSet& other) const noexcept  { return channels == other.channels; }
bool AudioChannelSet::operator!= (const AudioChannelSet& other) const noexcept  { return channels != other.channels; }

const char* AudioChannelSet::getChannelTypeName (AudioChannelSet::ChannelType type) noexcept
{
    switch (type)
    {
        case left:      return "Left";
        case right:     return "Right";
        case centre:    return "Centre";
        default:        break;
    }

    return "Unknown";
}

int AudioChannelSet::size() const noexcept
{
    return channels.countNumberOfSetBits();
}

AudioChannelSet::ChannelType AudioChannelSet::getTypeOfChannel (int index) const noexcept
{
    int bit = channels.findNextSetBit(0);

    for (int i = 0; i < index && bit >= 0; ++i)
        bit = channels.findNextSetBit (bit + 1);

    return static_cast<ChannelType> (bit);
}

Array<AudioChannelSet::ChannelType> AudioChannelSet::getChannelTypes() const
{
    Array<ChannelType> result;

    for (int bit = channels.findNextSetBit(0); bit >= 0; bit = channels.findNextSetBit (bit + 1))
        result.add (static_cast<ChannelType> (bit));

    return result;
}

void AudioChannelSet::addChannel (ChannelType newChannel)
{
    const int bit = static_cast<int> (newChannel);
    jassert (bit >= 0 && bit < 1024);
    channels.setBit (bit);
}

AudioChannelSet AudioChannelSet::mono()               { return AudioChannelSet (1u << centre); }
AudioChannelSet AudioChannelSet::stereo()             { return AudioChannelSet ((1u << left) | (1u << right)); }
AudioChannelSet AudioChannelSet::quadraphonic()       { return AudioChannelSet ((1u << left) | (1u << right) | (1u << sideLeft) | (1u << sideRight)); }
AudioChannelSet AudioChannelSet::pentagonal()         { return AudioChannelSet ((1u << left) | (1u << right) | (1u << sideLeft) | (1u << sideRight) | (1u << centre)); }
AudioChannelSet AudioChannelSet::hexagonal()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << sideLeft) | (1u << sideRight) | (1u << centre) | (1u << surround)); }
AudioChannelSet AudioChannelSet::octagonal()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << sideLeft) | (1u << sideRight) | (1u << centre) | (1u << surround) | (1u << wideLeft) | (1u << wideRight)); }
AudioChannelSet AudioChannelSet::ambisonic()          { return AudioChannelSet ((1u << ambisonicW) | (1u << ambisonicX) | (1u << ambisonicY) | (1u << ambisonicZ)); }
AudioChannelSet AudioChannelSet::create5point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << surroundLeft) | (1u << surroundRight)); }
AudioChannelSet AudioChannelSet::create5point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << surroundLeft)  | (1u << surroundRight)); }
AudioChannelSet AudioChannelSet::create6point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << surroundLeft)  | (1u << surroundRight) | (1u << surround)); }
AudioChannelSet AudioChannelSet::create6point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass)  | (1u << surroundLeft)  | (1u << surroundRight) | (1u << surround)); }
AudioChannelSet AudioChannelSet::create7point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << surroundLeft)  | (1u << surroundRight) | (1u << topRearLeft) | (1u << topRearRight)); }
AudioChannelSet AudioChannelSet::create7point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << surroundLeft)  | (1u << surroundRight) | (1u << topRearLeft) | (1u << topRearRight)); }
AudioChannelSet AudioChannelSet::createFront7point0() { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << surroundLeft)  | (1u << surroundRight) | (1u << centreLeft) | (1u << centreRight)); }
AudioChannelSet AudioChannelSet::createFront7point1() { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << surroundLeft)  | (1u << surroundRight) | (1u << centreLeft) | (1u << centreRight)); }


AudioChannelSet AudioChannelSet::discreteChannels (int numChannels)
{
    AudioChannelSet s;
    s.channels.setRange (discreteChannel0, numChannels, true);
    return s;
}

AudioChannelSet AudioChannelSet::canonicalChannelSet (int numChannels)
{
    if (numChannels == 1) return AudioChannelSet::mono();
    if (numChannels == 2) return AudioChannelSet::stereo();
    if (numChannels == 4) return AudioChannelSet::quadraphonic();

    return discreteChannels (numChannels);
}
