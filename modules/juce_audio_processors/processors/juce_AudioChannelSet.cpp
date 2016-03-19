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
bool AudioChannelSet::operator<  (const AudioChannelSet& other) const noexcept  { return channels <  other.channels; }

String AudioChannelSet::getChannelTypeName (AudioChannelSet::ChannelType type)
{
    if (type >= discreteChannel0)
        return String ("Discrete ") + String (type - discreteChannel0 + 1);

    switch (type)
    {
        case left:           return NEEDS_TRANS("Left");
        case right:          return NEEDS_TRANS("Right");
        case centre:         return NEEDS_TRANS("Centre");
        case subbass:        return NEEDS_TRANS("Subbass");
        case surroundLeft:   return NEEDS_TRANS("Left Surround");
        case surroundRight:  return NEEDS_TRANS("Right Surround");
        case centreLeft:     return NEEDS_TRANS("Centre Left");
        case centreRight:    return NEEDS_TRANS("Centre Right");
        case surround:       return NEEDS_TRANS("Surround");
        case sideLeft:       return NEEDS_TRANS("Side Left");
        case sideRight:      return NEEDS_TRANS("Side Right");
        case topMiddle:      return NEEDS_TRANS("Top Middle");
        case topFrontLeft:   return NEEDS_TRANS("Top Front Left");
        case topFrontCentre: return NEEDS_TRANS("Top Front Centre");
        case topFrontRight:  return NEEDS_TRANS("Top Front Right");
        case topRearLeft:    return NEEDS_TRANS("Top Rear Left");
        case topRearCentre:  return NEEDS_TRANS("Top Rear Centre");
        case topRearRight:   return NEEDS_TRANS("Top Rear Right");
        case wideLeft:       return NEEDS_TRANS("Wide Left");
        case wideRight:      return NEEDS_TRANS("Wide Right");
        case subbass2:       return NEEDS_TRANS("Subbass 2");
        case ambisonicW:     return NEEDS_TRANS("Ambisonic W");
        case ambisonicX:     return NEEDS_TRANS("Ambisonic X");
        case ambisonicY:     return NEEDS_TRANS("Ambisonic Y");
        case ambisonicZ:     return NEEDS_TRANS("Ambisonic Z");
        default:             break;
    }

    return "Unknown";
}

String AudioChannelSet::getAbbreviatedChannelTypeName (AudioChannelSet::ChannelType type)
{
    if (type >= discreteChannel0)
        return String (type - discreteChannel0 + 1);

    switch (type)
    {
        case left:           return "L";
        case right:          return "R";
        case centre:         return "C";
        case subbass:        return "Lfe";
        case surroundLeft:   return "Ls";
        case surroundRight:  return "Rs";
        case centreLeft:     return "Lc";
        case centreRight:    return "Rc";
        case surround:       return "S";
        case sideLeft:       return "Sl";
        case sideRight:      return "Sr";
        case topMiddle:      return "Tm";
        case topFrontLeft:   return "Tfl";
        case topFrontCentre: return "Tfc";
        case topFrontRight:  return "Tfr";
        case topRearLeft:    return "Trl";
        case topRearCentre:  return "Trc";
        case topRearRight:   return "Trr";
        case wideLeft:       return "Wl";
        case wideRight:      return "Wr";
        case subbass2:       return "Lfe2";
        case ambisonicW:     return "W";
        case ambisonicX:     return "X";
        case ambisonicY:     return "Y";
        case ambisonicZ:     return "Z";
        default:        break;
    }

    return "";
}

String AudioChannelSet::getSpeakerArrangementAsString() const
{
    StringArray speakerTypes;
    Array<AudioChannelSet::ChannelType> speakers = getChannelTypes();

    for (int i = 0; i < speakers.size(); ++i)
    {
        String name = getAbbreviatedChannelTypeName (speakers.getReference (i));

        if (name.isNotEmpty())
            speakerTypes.add (name);
    }

    return speakerTypes.joinIntoString (" ");
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

AudioChannelSet AudioChannelSet::disabled()           { return AudioChannelSet(); }
AudioChannelSet AudioChannelSet::mono()               { return AudioChannelSet (1u << centre); }
AudioChannelSet AudioChannelSet::stereo()             { return AudioChannelSet ((1u << left) | (1u << right)); }
AudioChannelSet AudioChannelSet::createLCR()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre)); }
AudioChannelSet AudioChannelSet::createLCRS()         { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << surround)); }
AudioChannelSet AudioChannelSet::quadraphonic()       { return AudioChannelSet ((1u << left) | (1u << right) | (1u << surroundLeft) | (1u << surroundRight)); }
AudioChannelSet AudioChannelSet::pentagonal()         { return AudioChannelSet ((1u << left) | (1u << right) | (1u << surroundLeft) | (1u << surroundRight) | (1u << centre)); }
AudioChannelSet AudioChannelSet::hexagonal()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << surroundLeft) | (1u << surroundRight) | (1u << centre) | (1u << surround)); }
AudioChannelSet AudioChannelSet::octagonal()          { return AudioChannelSet ((1u << left) | (1u << right) | (1u << surroundLeft) | (1u << surroundRight) | (1u << centre) | (1u << surround) | (1u << wideLeft) | (1u << wideRight)); }
AudioChannelSet AudioChannelSet::ambisonic()          { return AudioChannelSet ((1u << ambisonicW) | (1u << ambisonicX) | (1u << ambisonicY) | (1u << ambisonicZ)); }
AudioChannelSet AudioChannelSet::create5point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << sideLeft) | (1u << sideRight)); }
AudioChannelSet AudioChannelSet::create5point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << sideLeft)  | (1u << sideRight)); }
AudioChannelSet AudioChannelSet::create6point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << sideLeft)  | (1u << sideRight) | (1u << surround)); }
AudioChannelSet AudioChannelSet::create6point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass)  | (1u << sideLeft)  | (1u << sideRight) | (1u << surround)); }
AudioChannelSet AudioChannelSet::create7point0()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << sideLeft)  | (1u << sideRight) | (1u << surroundLeft) | (1u << surroundRight)); }
AudioChannelSet AudioChannelSet::create7point1()      { return AudioChannelSet ((1u << left) | (1u << right) | (1u << centre) | (1u << subbass) | (1u << sideLeft)  | (1u << sideRight) | (1u << surroundLeft) | (1u << surroundRight)); }
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
