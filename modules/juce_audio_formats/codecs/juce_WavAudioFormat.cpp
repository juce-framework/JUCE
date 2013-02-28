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

static const char* const wavFormatName = "WAV file";
static const char* const wavExtensions[] = { ".wav", ".bwf", 0 };

//==============================================================================
const char* const WavAudioFormat::bwavDescription      = "bwav description";
const char* const WavAudioFormat::bwavOriginator       = "bwav originator";
const char* const WavAudioFormat::bwavOriginatorRef    = "bwav originator ref";
const char* const WavAudioFormat::bwavOriginationDate  = "bwav origination date";
const char* const WavAudioFormat::bwavOriginationTime  = "bwav origination time";
const char* const WavAudioFormat::bwavTimeReference    = "bwav time reference";
const char* const WavAudioFormat::bwavCodingHistory    = "bwav coding history";

StringPairArray WavAudioFormat::createBWAVMetadata (const String& description,
                                                    const String& originator,
                                                    const String& originatorRef,
                                                    const Time& date,
                                                    const int64 timeReferenceSamples,
                                                    const String& codingHistory)
{
    StringPairArray m;

    m.set (bwavDescription, description);
    m.set (bwavOriginator, originator);
    m.set (bwavOriginatorRef, originatorRef);
    m.set (bwavOriginationDate, date.formatted ("%Y-%m-%d"));
    m.set (bwavOriginationTime, date.formatted ("%H:%M:%S"));
    m.set (bwavTimeReference, String (timeReferenceSamples));
    m.set (bwavCodingHistory, codingHistory);

    return m;
}


//==============================================================================
namespace WavFileHelpers
{
    inline int chunkName (const char* const name) noexcept   { return (int) ByteOrder::littleEndianInt (name); }

    #if JUCE_MSVC
     #pragma pack (push, 1)
    #endif

    struct BWAVChunk
    {
        char description [256];
        char originator [32];
        char originatorRef [32];
        char originationDate [10];
        char originationTime [8];
        uint32 timeRefLow;
        uint32 timeRefHigh;
        uint16 version;
        uint8 umid[64];
        uint8 reserved[190];
        char codingHistory[1];

        void copyTo (StringPairArray& values, const int totalSize) const
        {
            values.set (WavAudioFormat::bwavDescription,     String::fromUTF8 (description,     sizeof (description)));
            values.set (WavAudioFormat::bwavOriginator,      String::fromUTF8 (originator,      sizeof (originator)));
            values.set (WavAudioFormat::bwavOriginatorRef,   String::fromUTF8 (originatorRef,   sizeof (originatorRef)));
            values.set (WavAudioFormat::bwavOriginationDate, String::fromUTF8 (originationDate, sizeof (originationDate)));
            values.set (WavAudioFormat::bwavOriginationTime, String::fromUTF8 (originationTime, sizeof (originationTime)));

            const uint32 timeLow  = ByteOrder::swapIfBigEndian (timeRefLow);
            const uint32 timeHigh = ByteOrder::swapIfBigEndian (timeRefHigh);
            const int64 time = (((int64)timeHigh) << 32) + timeLow;

            values.set (WavAudioFormat::bwavTimeReference, String (time));
            values.set (WavAudioFormat::bwavCodingHistory,
                        String::fromUTF8 (codingHistory, totalSize - (int) offsetof (BWAVChunk, codingHistory)));
        }

        static MemoryBlock createFrom (const StringPairArray& values)
        {
            const size_t sizeNeeded = sizeof (BWAVChunk) + values [WavAudioFormat::bwavCodingHistory].getNumBytesAsUTF8();
            MemoryBlock data ((sizeNeeded + 3) & ~3);
            data.fillWith (0);

            BWAVChunk* b = (BWAVChunk*) data.getData();

            // Allow these calls to overwrite an extra byte at the end, which is fine as long
            // as they get called in the right order..
            values [WavAudioFormat::bwavDescription]    .copyToUTF8 (b->description, 257);
            values [WavAudioFormat::bwavOriginator]     .copyToUTF8 (b->originator, 33);
            values [WavAudioFormat::bwavOriginatorRef]  .copyToUTF8 (b->originatorRef, 33);
            values [WavAudioFormat::bwavOriginationDate].copyToUTF8 (b->originationDate, 11);
            values [WavAudioFormat::bwavOriginationTime].copyToUTF8 (b->originationTime, 9);

            const int64 time = values [WavAudioFormat::bwavTimeReference].getLargeIntValue();
            b->timeRefLow = ByteOrder::swapIfBigEndian ((uint32) (time & 0xffffffff));
            b->timeRefHigh = ByteOrder::swapIfBigEndian ((uint32) (time >> 32));

            values [WavAudioFormat::bwavCodingHistory].copyToUTF8 (b->codingHistory, 0x7fffffff);

            if (b->description[0] != 0
                || b->originator[0] != 0
                || b->originationDate[0] != 0
                || b->originationTime[0] != 0
                || b->codingHistory[0] != 0
                || time != 0)
            {
                return data;
            }

            return MemoryBlock();
        }

    } JUCE_PACKED;


    //==============================================================================
    struct SMPLChunk
    {
        struct SampleLoop
        {
            uint32 identifier;
            uint32 type; // these are different in AIFF and WAV
            uint32 start;
            uint32 end;
            uint32 fraction;
            uint32 playCount;
        } JUCE_PACKED;

        uint32 manufacturer;
        uint32 product;
        uint32 samplePeriod;
        uint32 midiUnityNote;
        uint32 midiPitchFraction;
        uint32 smpteFormat;
        uint32 smpteOffset;
        uint32 numSampleLoops;
        uint32 samplerData;
        SampleLoop loops[1];

        template <typename NameType>
        static void setValue (StringPairArray& values, NameType name, uint32 val)
        {
            values.set (name, String (ByteOrder::swapIfBigEndian (val)));
        }

        static void setValue (StringPairArray& values, int prefix, const char* name, uint32 val)
        {
            setValue (values, "Loop" + String (prefix) + name, val);
        }

        void copyTo (StringPairArray& values, const int totalSize) const
        {
            setValue (values, "Manufacturer",      manufacturer);
            setValue (values, "Product",           product);
            setValue (values, "SamplePeriod",      samplePeriod);
            setValue (values, "MidiUnityNote",     midiUnityNote);
            setValue (values, "MidiPitchFraction", midiPitchFraction);
            setValue (values, "SmpteFormat",       smpteFormat);
            setValue (values, "SmpteOffset",       smpteOffset);
            setValue (values, "NumSampleLoops",    numSampleLoops);
            setValue (values, "SamplerData",       samplerData);

            for (int i = 0; i < (int) numSampleLoops; ++i)
            {
                if ((uint8*) (loops + (i + 1)) > ((uint8*) this) + totalSize)
                    break;

                setValue (values, i, "Identifier", loops[i].identifier);
                setValue (values, i, "Type",       loops[i].type);
                setValue (values, i, "Start",      loops[i].start);
                setValue (values, i, "End",        loops[i].end);
                setValue (values, i, "Fraction",   loops[i].fraction);
                setValue (values, i, "PlayCount",  loops[i].playCount);
            }
        }

        template <typename NameType>
        static uint32 getValue (const StringPairArray& values, NameType name, const char* def)
        {
            return ByteOrder::swapIfBigEndian ((uint32) values.getValue (name, def).getIntValue());
        }

        static uint32 getValue (const StringPairArray& values, int prefix, const char* name, const char* def)
        {
            return getValue (values, "Loop" + String (prefix) + name, def);
        }

        static MemoryBlock createFrom (const StringPairArray& values)
        {
            MemoryBlock data;
            const int numLoops = jmin (64, values.getValue ("NumSampleLoops", "0").getIntValue());

            if (numLoops > 0)
            {
                const size_t sizeNeeded = sizeof (SMPLChunk) + (size_t) (numLoops - 1) * sizeof (SampleLoop);
                data.setSize ((sizeNeeded + 3) & ~3, true);

                SMPLChunk* const s = static_cast <SMPLChunk*> (data.getData());

                s->manufacturer      = getValue (values, "Manufacturer", "0");
                s->product           = getValue (values, "Product", "0");
                s->samplePeriod      = getValue (values, "SamplePeriod", "0");
                s->midiUnityNote     = getValue (values, "MidiUnityNote", "60");
                s->midiPitchFraction = getValue (values, "MidiPitchFraction", "0");
                s->smpteFormat       = getValue (values, "SmpteFormat", "0");
                s->smpteOffset       = getValue (values, "SmpteOffset", "0");
                s->numSampleLoops    = ByteOrder::swapIfBigEndian ((uint32) numLoops);
                s->samplerData       = getValue (values, "SamplerData", "0");

                for (int i = 0; i < numLoops; ++i)
                {
                    s->loops[i].identifier = getValue (values, i, "Identifier", "0");
                    s->loops[i].type       = getValue (values, i, "Type", "0");
                    s->loops[i].start      = getValue (values, i, "Start", "0");
                    s->loops[i].end        = getValue (values, i, "End", "0");
                    s->loops[i].fraction   = getValue (values, i, "Fraction", "0");
                    s->loops[i].playCount  = getValue (values, i, "PlayCount", "0");
                }
            }

            return data;
        }
    } JUCE_PACKED;

    //==============================================================================
    struct InstChunk
    {
        int8 baseNote;
        int8 detune;
        int8 gain;
        int8 lowNote;
        int8 highNote;
        int8 lowVelocity;
        int8 highVelocity;

        static void setValue (StringPairArray& values, const char* name, int val)
        {
            values.set (name, String (val));
        }

        void copyTo (StringPairArray& values) const
        {
            setValue (values, "MidiUnityNote",  baseNote);
            setValue (values, "Detune",         detune);
            setValue (values, "Gain",           gain);
            setValue (values, "LowNote",        lowNote);
            setValue (values, "HighNote",       highNote);
            setValue (values, "LowVelocity",    lowVelocity);
            setValue (values, "HighVelocity",   highVelocity);
        }

        static int8 getValue (const StringPairArray& values, const char* name, const char* def)
        {
            return (int8) values.getValue (name, def).getIntValue();
        }

        static MemoryBlock createFrom (const StringPairArray& values)
        {
            MemoryBlock data;
            const StringArray& keys = values.getAllKeys();

            if (keys.contains ("LowNote", true) && keys.contains ("HighNote", true))
            {
                data.setSize (8, true);
                InstChunk* const inst = static_cast <InstChunk*> (data.getData());

                inst->baseNote      = getValue (values, "MidiUnityNote", "60");
                inst->detune        = getValue (values, "Detune", "0");
                inst->gain          = getValue (values, "Gain", "0");
                inst->lowNote       = getValue (values, "LowNote", "0");
                inst->highNote      = getValue (values, "HighNote", "127");
                inst->lowVelocity   = getValue (values, "LowVelocity", "1");
                inst->highVelocity  = getValue (values, "HighVelocity", "127");
            }

            return data;
        }
    } JUCE_PACKED;

    //==============================================================================
    struct CueChunk
    {
        struct Cue
        {
            uint32 identifier;
            uint32 order;
            uint32 chunkID;
            uint32 chunkStart;
            uint32 blockStart;
            uint32 offset;
        } JUCE_PACKED;

        uint32 numCues;
        Cue cues[1];

        static void setValue (StringPairArray& values, int prefix, const char* name, uint32 val)
        {
            values.set ("Cue" + String (prefix) + name, String (ByteOrder::swapIfBigEndian (val)));
        }

        void copyTo (StringPairArray& values, const int totalSize) const
        {
            values.set ("NumCuePoints", String (ByteOrder::swapIfBigEndian (numCues)));

            for (int i = 0; i < (int) numCues; ++i)
            {
                if ((uint8*) (cues + (i + 1)) > ((uint8*) this) + totalSize)
                    break;

                setValue (values, i, "Identifier",  cues[i].identifier);
                setValue (values, i, "Order",       cues[i].order);
                setValue (values, i, "ChunkID",     cues[i].chunkID);
                setValue (values, i, "ChunkStart",  cues[i].chunkStart);
                setValue (values, i, "BlockStart",  cues[i].blockStart);
                setValue (values, i, "Offset",      cues[i].offset);
            }
        }

        static void create (MemoryBlock& data, const StringPairArray& values)
        {
            const int numCues = values.getValue ("NumCuePoints", "0").getIntValue();

            if (numCues > 0)
            {
                const size_t sizeNeeded = sizeof (CueChunk) + (size_t) (numCues - 1) * sizeof (Cue);
                data.setSize ((sizeNeeded + 3) & ~3, true);

                CueChunk* const c = static_cast <CueChunk*> (data.getData());

                c->numCues = ByteOrder::swapIfBigEndian ((uint32) numCues);

                const String dataChunkID (chunkName ("data"));
                int nextOrder = 0;

               #if JUCE_DEBUG
                Array<uint32> identifiers;
               #endif

                for (int i = 0; i < numCues; ++i)
                {
                    const String prefix ("Cue" + String (i));

                    uint32 identifier = (uint32) values.getValue (prefix + "Identifier", "0").getIntValue();

                   #if JUCE_DEBUG
                    jassert (! identifiers.contains (identifier));
                    identifiers.add (identifier);
                   #endif

                    c->cues[i].identifier   = ByteOrder::swapIfBigEndian ((uint32) identifier);

                    const int order = values.getValue (prefix + "Order", String (nextOrder)).getIntValue();
                    nextOrder = jmax (nextOrder, order) + 1;

                    c->cues[i].order        = ByteOrder::swapIfBigEndian ((uint32) order);
                    c->cues[i].chunkID      = ByteOrder::swapIfBigEndian ((uint32) values.getValue (prefix + "ChunkID", dataChunkID).getIntValue());
                    c->cues[i].chunkStart   = ByteOrder::swapIfBigEndian ((uint32) values.getValue (prefix + "ChunkStart", "0").getIntValue());
                    c->cues[i].blockStart   = ByteOrder::swapIfBigEndian ((uint32) values.getValue (prefix + "BlockStart", "0").getIntValue());
                    c->cues[i].offset       = ByteOrder::swapIfBigEndian ((uint32) values.getValue (prefix + "Offset", "0").getIntValue());
                }
            }
        }

    } JUCE_PACKED;

    //==============================================================================
    namespace ListChunk
    {
        static int getValue (const StringPairArray& values, const String& name)
        {
            return values.getValue (name, "0").getIntValue();
        }

        static int getValue (const StringPairArray& values, const String& prefix, const char* name)
        {
            return getValue (values, prefix + name);
        }

        static void appendLabelOrNoteChunk (const StringPairArray& values, const String& prefix,
                                            const int chunkType, MemoryOutputStream& out)
        {
            const String label (values.getValue (prefix + "Text", prefix));
            const int labelLength = (int) label.getNumBytesAsUTF8() + 1;
            const int chunkLength = 4 + labelLength + (labelLength & 1);

            out.writeInt (chunkType);
            out.writeInt (chunkLength);
            out.writeInt (getValue (values, prefix, "Identifier"));
            out.write (label.toUTF8(), labelLength);

            if ((out.getDataSize() & 1) != 0)
                out.writeByte (0);
        }

        static void appendExtraChunk (const StringPairArray& values, const String& prefix, MemoryOutputStream& out)
        {
            const String text (values.getValue (prefix + "Text", prefix));

            const int textLength = (int) text.getNumBytesAsUTF8() + 1; // include null terminator
            int chunkLength = textLength + 20 + (textLength & 1);

            out.writeInt (chunkName ("ltxt"));
            out.writeInt (chunkLength);
            out.writeInt (getValue (values, prefix, "Identifier"));
            out.writeInt (getValue (values, prefix, "SampleLength"));
            out.writeInt (getValue (values, prefix, "Purpose"));
            out.writeShort ((short) getValue (values, prefix, "Country"));
            out.writeShort ((short) getValue (values, prefix, "Language"));
            out.writeShort ((short) getValue (values, prefix, "Dialect"));
            out.writeShort ((short) getValue (values, prefix, "CodePage"));
            out.write (text.toUTF8(), textLength);

            if ((out.getDataSize() & 1) != 0)
                out.writeByte (0);
        }

        static void create (MemoryBlock& block, const StringPairArray& values)
        {
            const int numCueLabels  = getValue (values, "NumCueLabels");
            const int numCueNotes   = getValue (values, "NumCueNotes");
            const int numCueRegions = getValue (values, "NumCueRegions");

            if (numCueLabels > 0 || numCueNotes > 0 || numCueRegions > 0)
            {
                MemoryOutputStream out (block, false);

                for (int i = 0; i < numCueLabels; ++i)
                    appendLabelOrNoteChunk (values, "CueLabel" + String (i), chunkName ("labl"), out);

                for (int i = 0; i < numCueNotes; ++i)
                    appendLabelOrNoteChunk (values, "CueNote" + String (i), chunkName ("note"), out);

                for (int i = 0; i < numCueRegions; ++i)
                    appendExtraChunk (values, "CueRegion" + String (i), out);
            }
        }
    }

    //==============================================================================
    struct ExtensibleWavSubFormat
    {
        uint32 data1;
        uint16 data2;
        uint16 data3;
        uint8  data4[8];
    } JUCE_PACKED;

    static const ExtensibleWavSubFormat pcmFormat       = { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
    static const ExtensibleWavSubFormat IEEEFloatFormat = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
    static const ExtensibleWavSubFormat ambisonicFormat = { 0x00000001, 0x0721, 0x11d3, { 0x86, 0x44, 0xC8, 0xC1, 0xCA, 0x00, 0x00, 0x00 } };

    struct DataSize64Chunk   // chunk ID = 'ds64' if data size > 0xffffffff, 'JUNK' otherwise
    {
        uint32 riffSizeLow;     // low 4 byte size of RF64 block
        uint32 riffSizeHigh;    // high 4 byte size of RF64 block
        uint32 dataSizeLow;     // low 4 byte size of data chunk
        uint32 dataSizeHigh;    // high 4 byte size of data chunk
        uint32 sampleCountLow;  // low 4 byte sample count of fact chunk
        uint32 sampleCountHigh; // high 4 byte sample count of fact chunk
        uint32 tableLength;     // number of valid entries in array 'table'
    } JUCE_PACKED;

    #if JUCE_MSVC
     #pragma pack (pop)
    #endif
}

//==============================================================================
class WavAudioFormatReader  : public AudioFormatReader
{
public:
    WavAudioFormatReader (InputStream* const in)
        : AudioFormatReader (in, TRANS (wavFormatName)),
          bwavChunkStart (0),
          bwavSize (0),
          dataLength (0),
          isRF64 (false)
    {
        using namespace WavFileHelpers;
        uint64 len = 0;
        uint64 end = 0;
        int cueNoteIndex = 0;
        int cueLabelIndex = 0;
        int cueRegionIndex = 0;

        const int firstChunkType = input->readInt();

        if (firstChunkType == chunkName ("RF64"))
        {
            input->skipNextBytes (4); // size is -1 for RF64
            isRF64 = true;
        }
        else if (firstChunkType == chunkName ("RIFF"))
        {
            len = (uint64) (uint32) input->readInt();
            end = len + (uint64) input->getPosition();
        }
        else
        {
            return;
        }

        const int64 startOfRIFFChunk = input->getPosition();

        if (input->readInt() == chunkName ("WAVE"))
        {
            if (isRF64 && input->readInt() == chunkName ("ds64"))
            {
                const uint32 length = (uint32) input->readInt();

                if (length < 28)
                    return;

                const int64 chunkEnd = input->getPosition() + length + (length & 1);
                len = (uint64) input->readInt64();
                end = len + (uint64) startOfRIFFChunk;
                dataLength = input->readInt64();
                input->setPosition (chunkEnd);
            }

            while ((uint64) input->getPosition() < end && ! input->isExhausted())
            {
                const int chunkType = input->readInt();
                uint32 length = (uint32) input->readInt();
                const int64 chunkEnd = input->getPosition() + length + (length & 1);

                if (chunkType == chunkName ("fmt "))
                {
                    // read the format chunk
                    const unsigned short format = (unsigned short) input->readShort();
                    numChannels = (unsigned int) input->readShort();
                    sampleRate = input->readInt();
                    const int bytesPerSec = input->readInt();
                    input->skipNextBytes (2);
                    bitsPerSample = (unsigned int) (int) input->readShort();

                    if (bitsPerSample > 64)
                    {
                        bytesPerFrame = bytesPerSec / (int) sampleRate;
                        bitsPerSample = 8 * (unsigned int) bytesPerFrame / numChannels;
                    }
                    else
                    {
                        bytesPerFrame = numChannels * bitsPerSample / 8;
                    }

                    if (format == 3)
                    {
                        usesFloatingPointData = true;
                    }
                    else if (format == 0xfffe /*WAVE_FORMAT_EXTENSIBLE*/)
                    {
                        if (length < 40) // too short
                        {
                            bytesPerFrame = 0;
                        }
                        else
                        {
                            input->skipNextBytes (4); // skip over size and bitsPerSample
                            metadataValues.set ("ChannelMask", String (input->readInt()));

                            ExtensibleWavSubFormat subFormat;
                            subFormat.data1 = (uint32) input->readInt();
                            subFormat.data2 = (uint16) input->readShort();
                            subFormat.data3 = (uint16) input->readShort();
                            input->read (subFormat.data4, sizeof (subFormat.data4));

                            if (memcmp (&subFormat, &pcmFormat, sizeof (subFormat)) != 0
                                 && memcmp (&subFormat, &IEEEFloatFormat, sizeof (subFormat)) != 0
                                 && memcmp (&subFormat, &ambisonicFormat, sizeof (subFormat)) != 0)
                                bytesPerFrame = 0;
                        }
                    }
                    else if (format != 1)
                    {
                        bytesPerFrame = 0;
                    }
                }
                else if (chunkType == chunkName ("data"))
                {
                    if (! isRF64) // data size is expected to be -1, actual data size is in ds64 chunk
                        dataLength = length;

                    dataChunkStart = input->getPosition();
                    lengthInSamples = (bytesPerFrame > 0) ? (dataLength / bytesPerFrame) : 0;
                }
                else if (chunkType == chunkName ("bext"))
                {
                    bwavChunkStart = input->getPosition();
                    bwavSize = length;

                    HeapBlock <BWAVChunk> bwav;
                    bwav.calloc (jmax ((size_t) length + 1, sizeof (BWAVChunk)), 1);
                    input->read (bwav, (int) length);
                    bwav->copyTo (metadataValues, (int) length);
                }
                else if (chunkType == chunkName ("smpl"))
                {
                    HeapBlock <SMPLChunk> smpl;
                    smpl.calloc (jmax ((size_t) length + 1, sizeof (SMPLChunk)), 1);
                    input->read (smpl, (int) length);
                    smpl->copyTo (metadataValues, (int) length);
                }
                else if (chunkType == chunkName ("inst") || chunkType == chunkName ("INST")) // need to check which...
                {
                    HeapBlock <InstChunk> inst;
                    inst.calloc (jmax ((size_t) length + 1, sizeof (InstChunk)), 1);
                    input->read (inst, (int) length);
                    inst->copyTo (metadataValues);
                }
                else if (chunkType == chunkName ("cue "))
                {
                    HeapBlock <CueChunk> cue;
                    cue.calloc (jmax ((size_t) length + 1, sizeof (CueChunk)), 1);
                    input->read (cue, (int) length);
                    cue->copyTo (metadataValues, (int) length);
                }
                else if (chunkType == chunkName ("LIST"))
                {
                    if (input->readInt() == chunkName ("adtl"))
                    {
                        while (input->getPosition() < chunkEnd)
                        {
                            const int adtlChunkType = input->readInt();
                            const uint32 adtlLength = (uint32) input->readInt();
                            const int64 adtlChunkEnd = input->getPosition() + (adtlLength + (adtlLength & 1));

                            if (adtlChunkType == chunkName ("labl") || adtlChunkType == chunkName ("note"))
                            {
                                String prefix;

                                if (adtlChunkType == chunkName ("labl"))
                                    prefix << "CueLabel" << cueLabelIndex++;
                                else if (adtlChunkType == chunkName ("note"))
                                    prefix << "CueNote" << cueNoteIndex++;

                                const uint32 identifier = (uint32) input->readInt();
                                const int stringLength = (int) adtlLength - 4;

                                MemoryBlock textBlock;
                                input->readIntoMemoryBlock (textBlock, stringLength);

                                metadataValues.set (prefix + "Identifier", String (identifier));
                                metadataValues.set (prefix + "Text", textBlock.toString());
                            }
                            else if (adtlChunkType == chunkName ("ltxt"))
                            {
                                const String prefix ("CueRegion" + String (cueRegionIndex++));
                                const uint32 identifier     = (uint32) input->readInt();
                                const uint32 sampleLength   = (uint32) input->readInt();
                                const uint32 purpose        = (uint32) input->readInt();
                                const uint16 country        = (uint16) input->readInt();
                                const uint16 language       = (uint16) input->readInt();
                                const uint16 dialect        = (uint16) input->readInt();
                                const uint16 codePage       = (uint16) input->readInt();
                                const uint32 stringLength   = adtlLength - 20;

                                MemoryBlock textBlock;
                                input->readIntoMemoryBlock (textBlock, (int) stringLength);

                                metadataValues.set (prefix + "Identifier",   String (identifier));
                                metadataValues.set (prefix + "SampleLength", String (sampleLength));
                                metadataValues.set (prefix + "Purpose",      String (purpose));
                                metadataValues.set (prefix + "Country",      String (country));
                                metadataValues.set (prefix + "Language",     String (language));
                                metadataValues.set (prefix + "Dialect",      String (dialect));
                                metadataValues.set (prefix + "CodePage",     String (codePage));
                                metadataValues.set (prefix + "Text",         textBlock.toString());
                            }

                            input->setPosition (adtlChunkEnd);
                        }
                    }
                }
                else if (chunkEnd <= input->getPosition())
                {
                    break;
                }

                input->setPosition (chunkEnd);
            }
        }

        if (cueLabelIndex > 0)          metadataValues.set ("NumCueLabels",    String (cueLabelIndex));
        if (cueNoteIndex > 0)           metadataValues.set ("NumCueNotes",     String (cueNoteIndex));
        if (cueRegionIndex > 0)         metadataValues.set ("NumCueRegions",   String (cueRegionIndex));
        if (metadataValues.size() > 0)  metadataValues.set ("MetaDataSource",  "WAV");
    }

    //==============================================================================
    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples)
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (numSamples <= 0)
            return true;

        input->setPosition (dataChunkStart + startSampleInFile * bytesPerFrame);

        while (numSamples > 0)
        {
            const int tempBufSize = 480 * 3 * 4; // (keep this a multiple of 3)
            char tempBuffer [tempBufSize];

            const int numThisTime = jmin (tempBufSize / bytesPerFrame, numSamples);
            const int bytesRead = input->read (tempBuffer, numThisTime * bytesPerFrame);

            if (bytesRead < numThisTime * bytesPerFrame)
            {
                jassert (bytesRead >= 0);
                zeromem (tempBuffer + bytesRead, (size_t) (numThisTime * bytesPerFrame - bytesRead));
            }

            copySampleData (bitsPerSample, usesFloatingPointData,
                            destSamples, startOffsetInDestBuffer, numDestChannels,
                            tempBuffer, (int) numChannels, numThisTime);

            startOffsetInDestBuffer += numThisTime;
            numSamples -= numThisTime;
        }

        return true;
    }

    static void copySampleData (unsigned int bitsPerSample, const bool usesFloatingPointData,
                                int* const* destSamples, int startOffsetInDestBuffer, int numDestChannels,
                                const void* sourceData, int numChannels, int numSamples) noexcept
    {
        switch (bitsPerSample)
        {
            case 8:     ReadHelper<AudioData::Int32, AudioData::UInt8, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numChannels, numSamples); break;
            case 16:    ReadHelper<AudioData::Int32, AudioData::Int16, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numChannels, numSamples); break;
            case 24:    ReadHelper<AudioData::Int32, AudioData::Int24, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numChannels, numSamples); break;
            case 32:    if (usesFloatingPointData) ReadHelper<AudioData::Float32, AudioData::Float32, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numChannels, numSamples);
                        else                       ReadHelper<AudioData::Int32, AudioData::Int32, AudioData::LittleEndian>::read (destSamples, startOffsetInDestBuffer, numDestChannels, sourceData, numChannels, numSamples); break;
            default:    jassertfalse; break;
        }
    }

    int64 bwavChunkStart, bwavSize;
    int64 dataChunkStart, dataLength;
    int bytesPerFrame;
    bool isRF64;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormatReader)
};

//==============================================================================
class WavAudioFormatWriter  : public AudioFormatWriter
{
public:
    WavAudioFormatWriter (OutputStream* const out, const double sampleRate_,
                          const unsigned int numChannels_, const unsigned int bits,
                          const StringPairArray& metadataValues)
        : AudioFormatWriter (out, TRANS (wavFormatName), sampleRate_, numChannels_, bits),
          lengthInSamples (0),
          bytesWritten (0),
          writeFailed (false)
    {
        using namespace WavFileHelpers;

        if (metadataValues.size() > 0)
        {
            // The meta data should have been santised for the WAV format.
            // If it was originally sourced from an AIFF file the MetaDataSource
            // key should be removed (or set to "WAV") once this has been done
            jassert (metadataValues.getValue ("MetaDataSource", "None") != "AIFF");

            bwavChunk = BWAVChunk::createFrom (metadataValues);
            smplChunk = SMPLChunk::createFrom (metadataValues);
            instChunk = InstChunk::createFrom (metadataValues);
            CueChunk ::create (cueChunk, metadataValues);
            ListChunk::create (listChunk, metadataValues);
        }

        headerPosition = out->getPosition();
        writeHeader();
    }

    ~WavAudioFormatWriter()
    {
        if ((bytesWritten & 1) != 0) // pad to an even length
        {
            ++bytesWritten;
            output->writeByte (0);
        }

        writeHeader();
    }

    //==============================================================================
    bool write (const int** data, int numSamples)
    {
        jassert (data != nullptr && *data != nullptr); // the input must contain at least one channel!

        if (writeFailed)
            return false;

        const size_t bytes = numChannels * (unsigned int) numSamples * bitsPerSample / 8;
        tempBlock.ensureSize (bytes, false);

        switch (bitsPerSample)
        {
            case 8:     WriteHelper<AudioData::UInt8, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (int) numChannels, data, numSamples); break;
            case 16:    WriteHelper<AudioData::Int16, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (int) numChannels, data, numSamples); break;
            case 24:    WriteHelper<AudioData::Int24, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (int) numChannels, data, numSamples); break;
            case 32:    WriteHelper<AudioData::Int32, AudioData::Int32, AudioData::LittleEndian>::write (tempBlock.getData(), (int) numChannels, data, numSamples); break;
            default:    jassertfalse; break;
        }

        if (! output->write (tempBlock.getData(), bytes))
        {
            // failed to write to disk, so let's try writing the header.
            // If it's just run out of disk space, then if it does manage
            // to write the header, we'll still have a useable file..
            writeHeader();
            writeFailed = true;
            return false;
        }
        else
        {
            bytesWritten += bytes;
            lengthInSamples += numSamples;

            return true;
        }
    }

private:
    MemoryBlock tempBlock, bwavChunk, smplChunk, instChunk, cueChunk, listChunk;
    uint64 lengthInSamples, bytesWritten;
    int64 headerPosition;
    bool writeFailed;

    static int getChannelMask (const int numChannels) noexcept
    {
        switch (numChannels)
        {
            case 1:   return 0;
            case 2:   return 1 + 2; // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT
            case 5:   return 1 + 2 + 4 + 16 + 32; // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT
            case 6:   return 1 + 2 + 4 + 8 + 16 + 32; // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT
            case 7:   return 1 + 2 + 4  + 16 + 32 + 512 + 1024; // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER  | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT
            case 8:   return 1 + 2 + 4 + 8 + 16 + 32 + 512 + 1024; // SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT
            default:  break;
        }

        return 0;
    }

    void writeHeader()
    {
        using namespace WavFileHelpers;
        const bool seekedOk = output->setPosition (headerPosition);
        (void) seekedOk;

        // if this fails, you've given it an output stream that can't seek! It needs
        // to be able to seek back to write the header
        jassert (seekedOk);

        const size_t bytesPerFrame = numChannels * bitsPerSample / 8;
        uint64 audioDataSize = bytesPerFrame * lengthInSamples;

        const bool isRF64 = (bytesWritten >= literal64bit (0x100000000));
        const bool isWaveFmtEx = isRF64 || (numChannels > 2);

        int64 riffChunkSize = (int64) (4 /* 'RIFF' */ + 8 + 40 /* WAVEFORMATEX */
                                       + 8 + audioDataSize + (audioDataSize & 1)
                                       + (bwavChunk.getSize() > 0 ? (8  + bwavChunk.getSize()) : 0)
                                       + (smplChunk.getSize() > 0 ? (8  + smplChunk.getSize()) : 0)
                                       + (instChunk.getSize() > 0 ? (8  + instChunk.getSize()) : 0)
                                       + (cueChunk .getSize() > 0 ? (8  + cueChunk .getSize()) : 0)
                                       + (listChunk.getSize() > 0 ? (12 + listChunk.getSize()) : 0)
                                       + (8 + 28)); // (ds64 chunk)

        riffChunkSize += (riffChunkSize & 1);

        output->writeInt (chunkName (isRF64 ? "RF64" : "RIFF"));
        output->writeInt (isRF64 ? -1 : (int) riffChunkSize);
        output->writeInt (chunkName ("WAVE"));

        if (! isRF64)
        {
            output->writeInt (chunkName ("JUNK"));
            output->writeInt (28 + (isWaveFmtEx? 0 : 24));
            output->writeRepeatedByte (0, 28 /* ds64 */ + (isWaveFmtEx? 0 : 24));
        }
        else
        {
            // write ds64 chunk
            output->writeInt (chunkName ("ds64"));
            output->writeInt (28);  // chunk size for uncompressed data (no table)
            output->writeInt64 (riffChunkSize);
            output->writeInt64 ((int64) audioDataSize);
            output->writeRepeatedByte (0, 12);
        }

        output->writeInt (chunkName ("fmt "));

        if (isWaveFmtEx)
        {
            output->writeInt (40); // chunk size
            output->writeShort ((short) (uint16) 0xfffe); // WAVE_FORMAT_EXTENSIBLE
        }
        else
        {
            output->writeInt (16); // chunk size
            output->writeShort (bitsPerSample < 32 ? (short) 1 /*WAVE_FORMAT_PCM*/
                                                   : (short) 3 /*WAVE_FORMAT_IEEE_FLOAT*/);
        }

        output->writeShort ((short) numChannels);
        output->writeInt ((int) sampleRate);
        output->writeInt ((int) (bytesPerFrame * sampleRate)); // nAvgBytesPerSec
        output->writeShort ((short) bytesPerFrame); // nBlockAlign
        output->writeShort ((short) bitsPerSample); // wBitsPerSample

        if (isWaveFmtEx)
        {
            output->writeShort (22); // cbSize (size of  the extension)
            output->writeShort ((short) bitsPerSample); // wValidBitsPerSample
            output->writeInt (getChannelMask ((int) numChannels));

            const ExtensibleWavSubFormat& subFormat = bitsPerSample < 32 ? pcmFormat : IEEEFloatFormat;

            output->writeInt ((int) subFormat.data1);
            output->writeShort ((short) subFormat.data2);
            output->writeShort ((short) subFormat.data3);
            output->write (subFormat.data4, sizeof (subFormat.data4));
        }

        if (bwavChunk.getSize() > 0)
        {
            output->writeInt (chunkName ("bext"));
            output->writeInt ((int) bwavChunk.getSize());
            *output << bwavChunk;
        }

        if (smplChunk.getSize() > 0)
        {
            output->writeInt (chunkName ("smpl"));
            output->writeInt ((int) smplChunk.getSize());
            *output << smplChunk;
        }

        if (instChunk.getSize() > 0)
        {
            output->writeInt (chunkName ("inst"));
            output->writeInt (7);
            *output << instChunk;
        }

        if (cueChunk.getSize() > 0)
        {
            output->writeInt (chunkName ("cue "));
            output->writeInt ((int) cueChunk.getSize());
            *output << cueChunk;
        }

        if (listChunk.getSize() > 0)
        {
            output->writeInt (chunkName ("LIST"));
            output->writeInt ((int) listChunk.getSize() + 4);
            output->writeInt (chunkName ("adtl"));
            *output << listChunk;
        }

        output->writeInt (chunkName ("data"));
        output->writeInt (isRF64 ? -1 : (int) (lengthInSamples * bytesPerFrame));

        usesFloatingPointData = (bitsPerSample == 32);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavAudioFormatWriter)
};

//==============================================================================
class MemoryMappedWavReader   : public MemoryMappedAudioFormatReader
{
public:
    MemoryMappedWavReader (const File& file, const WavAudioFormatReader& reader)
        : MemoryMappedAudioFormatReader (file, reader, reader.dataChunkStart,
                                         reader.dataLength, reader.bytesPerFrame)
    {
    }

    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples)
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (map == nullptr || ! mappedSection.contains (Range<int64> (startSampleInFile, startSampleInFile + numSamples)))
        {
            jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.
            return false;
        }

        WavAudioFormatReader::copySampleData (bitsPerSample, usesFloatingPointData,
                                              destSamples, startOffsetInDestBuffer, numDestChannels,
                                              sampleToPointer (startSampleInFile), (int) numChannels, numSamples);
        return true;
    }

    void readMaxLevels (int64 startSampleInFile, int64 numSamples,
                        float& min0, float& max0, float& min1, float& max1)
    {
        if (numSamples <= 0)
        {
            min0 = max0 = min1 = max1 = 0;
            return;
        }

        if (map == nullptr || ! mappedSection.contains (Range<int64> (startSampleInFile, startSampleInFile + numSamples)))
        {
            jassertfalse; // you must make sure that the window contains all the samples you're going to attempt to read.

            min0 = max0 = min1 = max1 = 0;
            return;
        }

        switch (bitsPerSample)
        {
            case 8:     scanMinAndMax<AudioData::UInt8> (startSampleInFile, numSamples, min0, max0, min1, max1); break;
            case 16:    scanMinAndMax<AudioData::Int16> (startSampleInFile, numSamples, min0, max0, min1, max1); break;
            case 24:    scanMinAndMax<AudioData::Int24> (startSampleInFile, numSamples, min0, max0, min1, max1); break;
            case 32:    if (usesFloatingPointData) scanMinAndMax<AudioData::Float32> (startSampleInFile, numSamples, min0, max0, min1, max1);
                        else                       scanMinAndMax<AudioData::Int32>   (startSampleInFile, numSamples, min0, max0, min1, max1); break;
            default:    jassertfalse; break;
        }
    }

private:
    template <typename SampleType>
    void scanMinAndMax (int64 startSampleInFile, int64 numSamples,
                        float& min0, float& max0, float& min1, float& max1) const
    {
        typedef AudioData::Pointer <SampleType, AudioData::LittleEndian, AudioData::Interleaved, AudioData::Const> SourceType;

        SourceType (sampleToPointer (startSampleInFile), (int) numChannels)
            .findMinAndMax ((size_t) numSamples, min0, max0);

        if (numChannels > 1)
            SourceType (addBytesToPointer (sampleToPointer (startSampleInFile), bitsPerSample / 8), (int) numChannels)
                  .findMinAndMax ((size_t) numSamples, min1, max1);
        else
            min1 = max1 = 0;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedWavReader)
};

//==============================================================================
WavAudioFormat::WavAudioFormat()
    : AudioFormat (TRANS (wavFormatName), StringArray (wavExtensions))
{
}

WavAudioFormat::~WavAudioFormat()
{
}

Array<int> WavAudioFormat::getPossibleSampleRates()
{
    const int rates[] = { 8000, 11025, 12000, 16000, 22050, 32000,
                          44100, 48000, 88200, 96000, 176400, 192000 };

    return Array<int> (rates, numElementsInArray (rates));
}

Array<int> WavAudioFormat::getPossibleBitDepths()
{
    const int depths[] = { 8, 16, 24, 32 };

    return Array<int> (depths, numElementsInArray (depths));
}

bool WavAudioFormat::canDoStereo()  { return true; }
bool WavAudioFormat::canDoMono()    { return true; }

AudioFormatReader* WavAudioFormat::createReaderFor (InputStream* sourceStream,
                                                    const bool deleteStreamIfOpeningFails)
{
    ScopedPointer <WavAudioFormatReader> r (new WavAudioFormatReader (sourceStream));

    if (r->sampleRate > 0 && r->numChannels > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

MemoryMappedAudioFormatReader* WavAudioFormat::createMemoryMappedReader (const File& file)
{
    if (FileInputStream* fin = file.createInputStream())
    {
        WavAudioFormatReader reader (fin);

        if (reader.lengthInSamples > 0)
            return new MemoryMappedWavReader (file, reader);
    }

    return nullptr;
}

AudioFormatWriter* WavAudioFormat::createWriterFor (OutputStream* out, double sampleRate,
                                                    unsigned int numChannels, int bitsPerSample,
                                                    const StringPairArray& metadataValues, int /*qualityOptionIndex*/)
{
    if (getPossibleBitDepths().contains (bitsPerSample))
        return new WavAudioFormatWriter (out, sampleRate, (unsigned int) numChannels,
                                         (unsigned int) bitsPerSample, metadataValues);

    return nullptr;
}

namespace WavFileHelpers
{
    static bool slowCopyWavFileWithNewMetadata (const File& file, const StringPairArray& metadata)
    {
        TemporaryFile tempFile (file);

        WavAudioFormat wav;
        ScopedPointer <AudioFormatReader> reader (wav.createReaderFor (file.createInputStream(), true));

        if (reader != nullptr)
        {
            ScopedPointer <OutputStream> outStream (tempFile.getFile().createOutputStream());

            if (outStream != nullptr)
            {
                ScopedPointer <AudioFormatWriter> writer (wav.createWriterFor (outStream, reader->sampleRate,
                                                                               reader->numChannels, (int) reader->bitsPerSample,
                                                                               metadata, 0));

                if (writer != nullptr)
                {
                    outStream.release();

                    bool ok = writer->writeFromAudioReader (*reader, 0, -1);
                    writer = nullptr;
                    reader = nullptr;

                    return ok && tempFile.overwriteTargetFileWithTemporary();
                }
            }
        }

        return false;
    }
}

bool WavAudioFormat::replaceMetadataInFile (const File& wavFile, const StringPairArray& newMetadata)
{
    using namespace WavFileHelpers;
    ScopedPointer <WavAudioFormatReader> reader (static_cast <WavAudioFormatReader*> (createReaderFor (wavFile.createInputStream(), true)));

    if (reader != nullptr)
    {
        const int64 bwavPos  = reader->bwavChunkStart;
        const int64 bwavSize = reader->bwavSize;
        reader = nullptr;

        if (bwavSize > 0)
        {
            MemoryBlock chunk (BWAVChunk::createFrom (newMetadata));

            if (chunk.getSize() <= (size_t) bwavSize)
            {
                // the new one will fit in the space available, so write it directly..
                const int64 oldSize = wavFile.getSize();

                {
                    FileOutputStream out (wavFile);

                    if (! out.failedToOpen())
                    {
                        out.setPosition (bwavPos);
                        out << chunk;
                        out.setPosition (oldSize);
                    }
                }

                jassert (wavFile.getSize() == oldSize);

                return true;
            }
        }
    }

    return slowCopyWavFileWithNewMetadata (wavFile, newMetadata);
}
