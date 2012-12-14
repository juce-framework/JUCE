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

class GZIPCompressorOutputStream::GZIPCompressorHelper
{
public:
    GZIPCompressorHelper (const int compressionLevel, const int windowBits)
        : compLevel ((compressionLevel < 1 || compressionLevel > 9) ? -1 : compressionLevel),
          isFirstDeflate (true),
          streamIsValid (false),
          finished (false)
    {
        using namespace zlibNamespace;
        zerostruct (stream);

        streamIsValid = (deflateInit2 (&stream, compLevel, Z_DEFLATED,
                                       windowBits != 0 ? windowBits : MAX_WBITS,
                                       8, strategy) == Z_OK);
    }

    ~GZIPCompressorHelper()
    {
        if (streamIsValid)
            zlibNamespace::deflateEnd (&stream);
    }

    bool write (const uint8* data, unsigned int dataSize, OutputStream& out)
    {
        // When you call flush() on a gzip stream, the stream is closed, and you can
        // no longer continue to write data to it!
        jassert (! finished);

        while (dataSize > 0)
            if (! doNextBlock (data, dataSize, out, Z_NO_FLUSH))
                return false;

        return true;
    }

    void finish (OutputStream& out)
    {
        const uint8* data = nullptr;
        unsigned int dataSize = 0;

        while (! finished)
            doNextBlock (data, dataSize, out, Z_FINISH);
    }

private:
    enum { strategy = 0 };

    zlibNamespace::z_stream stream;
    const int compLevel;
    bool isFirstDeflate, streamIsValid, finished;
    zlibNamespace::Bytef buffer[32768];

    bool doNextBlock (const uint8*& data, unsigned int& dataSize, OutputStream& out, const int flushMode)
    {
        using namespace zlibNamespace;
        if (streamIsValid)
        {
            stream.next_in   = const_cast <uint8*> (data);
            stream.next_out  = buffer;
            stream.avail_in  = (z_uInt) dataSize;
            stream.avail_out = (z_uInt) sizeof (buffer);

            const int result = isFirstDeflate ? deflateParams (&stream, compLevel, strategy)
                                              : deflate (&stream, flushMode);
            isFirstDeflate = false;

            switch (result)
            {
                case Z_STREAM_END:
                    finished = true;
                    // Deliberate fall-through..
                case Z_OK:
                {
                    data += dataSize - stream.avail_in;
                    dataSize = stream.avail_in;
                    const int bytesDone = ((int) sizeof (buffer)) - (int) stream.avail_out;
                    return bytesDone <= 0 || out.write (buffer, bytesDone);
                }

                default:
                    break;
            }
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE (GZIPCompressorHelper)
};

//==============================================================================
GZIPCompressorOutputStream::GZIPCompressorOutputStream (OutputStream* const out,
                                                        const int compressionLevel,
                                                        const bool deleteDestStream,
                                                        const int windowBits)
    : destStream (out, deleteDestStream),
      helper (new GZIPCompressorHelper (compressionLevel, windowBits))
{
    jassert (out != nullptr);
}

GZIPCompressorOutputStream::~GZIPCompressorOutputStream()
{
    flush();
}

void GZIPCompressorOutputStream::flush()
{
    helper->finish (*destStream);
    destStream->flush();
}

bool GZIPCompressorOutputStream::write (const void* destBuffer, int howMany)
{
    jassert (destBuffer != nullptr && howMany >= 0);

    return helper->write (static_cast <const uint8*> (destBuffer),
                          (unsigned int) howMany, *destStream);
}

int64 GZIPCompressorOutputStream::getPosition()
{
    return destStream->getPosition();
}

bool GZIPCompressorOutputStream::setPosition (int64 /*newPosition*/)
{
    jassertfalse; // can't do it!
    return false;
}

//==============================================================================
#if JUCE_UNIT_TESTS

class GZIPTests  : public UnitTest
{
public:
    GZIPTests()   : UnitTest ("GZIP") {}

    void runTest()
    {
        beginTest ("GZIP");
        Random rng;

        for (int i = 100; --i >= 0;)
        {
            MemoryOutputStream original, compressed, uncompressed;

            {
                GZIPCompressorOutputStream zipper (&compressed, rng.nextInt (10), false);

                for (int j = rng.nextInt (100); --j >= 0;)
                {
                    MemoryBlock data ((unsigned int) (rng.nextInt (2000) + 1));

                    for (int k = (int) data.getSize(); --k >= 0;)
                        data[k] = (char) rng.nextInt (255);

                    original.write (data.getData(), (int) data.getSize());
                    zipper  .write (data.getData(), (int) data.getSize());
                }
            }

            {
                MemoryInputStream compressedInput (compressed.getData(), compressed.getDataSize(), false);
                GZIPDecompressorInputStream unzipper (compressedInput);

                uncompressed << unzipper;
            }

            expectEquals ((int) uncompressed.getDataSize(),
                          (int) original.getDataSize());

            if (original.getDataSize() == uncompressed.getDataSize())
                expect (memcmp (uncompressed.getData(),
                                original.getData(),
                                original.getDataSize()) == 0);
        }
    }
};

static GZIPTests gzipTests;

#endif
