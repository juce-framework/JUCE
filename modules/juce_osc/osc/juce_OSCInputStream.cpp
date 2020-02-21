/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce{

OSCInputStream::OSCInputStream (const void* sourceData, size_t sourceDataSize)
    : input (sourceData, sourceDataSize, false)
{}

//==============================================================================
int32 OSCInputStream::readInt32()
{
    checkBytesAvailable (4, "OSC input stream exhausted while reading int32");
    return input.readIntBigEndian();
}

uint64 OSCInputStream::readUint64()
{
    checkBytesAvailable (8, "OSC input stream exhausted while reading uint64");
    return (uint64) input.readInt64BigEndian();
}

float OSCInputStream::readFloat32()
{
    checkBytesAvailable (4, "OSC input stream exhausted while reading float");
    return input.readFloatBigEndian();
}

String OSCInputStream::readString()
{
    checkBytesAvailable (4, "OSC input stream exhausted while reading string");

    auto posBegin = (size_t) getPosition();
    auto s = input.readString();
    auto posEnd = (size_t) getPosition();

    if (static_cast<const char*> (getData()) [posEnd - 1] != '\0')
        throw OSCFormatError ("OSC input stream exhausted before finding null terminator of string");

    size_t bytesRead = posEnd - posBegin;
    readPaddingZeros (bytesRead);

    return s;
}

MemoryBlock OSCInputStream::readBlob()
{
    checkBytesAvailable (4, "OSC input stream exhausted while reading blob");

    auto blobDataSize = input.readIntBigEndian();
    checkBytesAvailable ((blobDataSize + 3) % 4, "OSC input stream exhausted before reaching end of blob");

    MemoryBlock blob;
    auto bytesRead = input.readIntoMemoryBlock (blob, (ssize_t) blobDataSize);
    readPaddingZeros (bytesRead);

    return blob;
}

OSCColour OSCInputStream::readColour()
{
    checkBytesAvailable (4, "OSC input stream exhausted while reading colour");
    return OSCColour::fromInt32 ((uint32) input.readIntBigEndian());
}

OSCTimeTag OSCInputStream::readTimeTag()
{
    checkBytesAvailable (8, "OSC input stream exhausted while reading time tag");
    return OSCTimeTag (uint64 (input.readInt64BigEndian()));
}

OSCAddress OSCInputStream::readAddress()
{
    return OSCAddress (readString());
}

OSCAddressPattern OSCInputStream::readAddressPattern()
{
    return OSCAddressPattern (readString());
}

//==============================================================================
OSCTypeList OSCInputStream::readTypeTagString()
{
    OSCTypeList typeList;

    checkBytesAvailable (4, "OSC input stream exhausted while reading type tag string");

    if (input.readByte() != ',')
        throw OSCFormatError ("OSC input stream format error: expected type tag string");

    for (;;)
    {
        if (isExhausted())
            throw OSCFormatError ("OSC input stream exhausted while reading type tag string");

        const OSCType type = input.readByte();

        if (type == 0)
            break;  // encountered null terminator. list is complete.

        if (! OSCTypes::isSupportedType (type))
            throw OSCFormatError ("OSC input stream format error: encountered unsupported type tag");

        typeList.add (type);
    }

    auto bytesRead = (size_t) typeList.size() + 2;
    readPaddingZeros (bytesRead);

    return typeList;
}

//==============================================================================
OSCArgument OSCInputStream::readArgument (OSCType type)
{
    switch (type)
    {
        case OSCTypes::int32:       return OSCArgument (readInt32());
        case OSCTypes::float32:     return OSCArgument (readFloat32());
        case OSCTypes::string:      return OSCArgument (readString());
        case OSCTypes::blob:        return OSCArgument (readBlob());
        case OSCTypes::colour:      return OSCArgument (readColour());

        default:
            // You supplied an invalid OSCType when calling readArgument! This should never happen.
            jassertfalse;
            throw OSCInternalError ("OSC input stream: internal error while reading message argument");
    }
}

//==============================================================================
OSCMessage OSCInputStream::readMessage()
{
    auto ap = readAddressPattern();
    auto types = readTypeTagString();

    OSCMessage msg (ap);

    for (auto& type : types)
        msg.addArgument (readArgument (type));

    return msg;
}

//==============================================================================
OSCBundle OSCInputStream::readBundle (size_t maxBytesToRead)
{
    // maxBytesToRead is only passed in here in case this bundle is a nested
    // bundle, so we know when to consider the next element *not* part of this
    // bundle anymore (but part of the outer bundle) and return.

    checkBytesAvailable (16, "OSC input stream exhausted while reading bundle");

    if (readString() != "#bundle")
        throw OSCFormatError ("OSC input stream format error: bundle does not start with string '#bundle'");

    OSCBundle bundle (readTimeTag());

    size_t bytesRead = 16; // already read "#bundle" and timeTag
    auto pos = getPosition();

    while (! isExhausted() && bytesRead < maxBytesToRead)
    {
        bundle.addElement (readElement());

        auto newPos = getPosition();
        bytesRead += (size_t) (newPos - pos);
        pos = newPos;
    }

    return bundle;
}

//==============================================================================
OSCBundle::Element OSCInputStream::readElement()
{
    checkBytesAvailable (4, "OSC input stream exhausted while reading bundle element size");

    auto elementSize = (size_t) readInt32();

    if (elementSize < 4)
        throw OSCFormatError ("OSC input stream format error: invalid bundle element size");

    return readElementWithKnownSize (elementSize);
}

//==============================================================================
OSCBundle::Element OSCInputStream::readElementWithKnownSize (size_t elementSize)
{
    checkBytesAvailable ((int64) elementSize, "OSC input stream exhausted while reading bundle element content");

    auto firstContentChar = static_cast<const char*> (getData()) [getPosition()];

    if (firstContentChar == '/')  return OSCBundle::Element (readMessageWithCheckedSize (elementSize));
    if (firstContentChar == '#')  return OSCBundle::Element (readBundleWithCheckedSize (elementSize));

    throw OSCFormatError ("OSC input stream: invalid bundle element content");
}

//==============================================================================
void OSCInputStream::readPaddingZeros (size_t bytesRead)
{
    size_t numZeros = ~(bytesRead - 1) & 0x03;

    while (numZeros > 0)
    {
        if (isExhausted() || input.readByte() != 0)
            throw OSCFormatError ("OSC input stream format error: missing padding zeros");

        --numZeros;
    }
}

OSCBundle OSCInputStream::readBundleWithCheckedSize (size_t size)
{
    auto begin = (size_t) getPosition();
    auto maxBytesToRead = size - 4; // we've already read 4 bytes (the bundle size)

    OSCBundle bundle (readBundle (maxBytesToRead));

    if (getPosition() - begin != size)
        throw OSCFormatError ("OSC input stream format error: wrong element content size encountered while reading");

    return bundle;
}

OSCMessage OSCInputStream::readMessageWithCheckedSize (size_t size)
{
    auto begin = (size_t) getPosition();
    auto message = readMessage();

    if (getPosition() - begin != size)
        throw OSCFormatError ("OSC input stream format error: wrong element content size encountered while reading");

    return message;
}

void OSCInputStream::checkBytesAvailable (int64 requiredBytes, const char* message)
{
    if (input.getNumBytesRemaining() < requiredBytes)
        throw OSCFormatError (message);
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class OSCInputStreamTests  : public UnitTest
{
public:
    OSCInputStreamTests()
        : UnitTest ("OSCInputStream class", UnitTestCategories::osc)
    {}

    void runTest()
    {
        beginTest ("reading OSC addresses");
        {
            const char buffer[16] = {
                '/', 't', 'e', 's', 't', '/', 'f', 'a',
                'd', 'e', 'r', '7', '\0', '\0', '\0', '\0' };

            // reading a valid osc address:
            {
                OSCInputStream inStream (buffer, sizeof (buffer));
                OSCAddress address = inStream.readAddress();

                expect (inStream.getPosition() == sizeof (buffer));
                expectEquals (address.toString(), String ("/test/fader7"));
            }

            // check various possible failures:
            {
                // zero padding is present, but size is not modulo 4:
                OSCInputStream inStream (buffer, 15);
                expectThrowsType (inStream.readAddress(), OSCFormatError)
            }
            {
                // zero padding is missing:
                OSCInputStream inStream (buffer, 12);
                expectThrowsType (inStream.readAddress(), OSCFormatError)
            }
            {
                // pattern does not start with a forward slash:
                OSCInputStream inStream (buffer + 4, 12);
                expectThrowsType (inStream.readAddress(), OSCFormatError)
            }
        }

        beginTest ("reading OSC address patterns");
        {
            const char buffer[20] = {
                '/', '*', '/', '*', 'p', 'u', 't', '/',
                'f', 'a', 'd', 'e', 'r', '[', '0', '-',
                '9', ']', '\0', '\0' };

            // reading a valid osc address pattern:
            {
                OSCInputStream inStream (buffer, sizeof (buffer));
                expectDoesNotThrow (inStream.readAddressPattern());
            }
            {
                OSCInputStream inStream (buffer, sizeof (buffer));
                OSCAddressPattern ap = inStream.readAddressPattern();

                expect (inStream.getPosition() == sizeof (buffer));
                expectEquals (ap.toString(), String ("/*/*put/fader[0-9]"));
                expect (ap.containsWildcards());
            }

            // check various possible failures:
            {
                // zero padding is present, but size is not modulo 4:
                OSCInputStream inStream (buffer, 19);
                expectThrowsType (inStream.readAddressPattern(), OSCFormatError)
            }
            {
                // zero padding is missing:
                OSCInputStream inStream (buffer, 16);
                expectThrowsType (inStream.readAddressPattern(), OSCFormatError)
            }
            {
                // pattern does not start with a forward slash:
                OSCInputStream inStream (buffer + 4, 16);
                expectThrowsType (inStream.readAddressPattern(), OSCFormatError)
            }
        }

        beginTest ("reading OSC time tags");

        {
            char buffer[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
            OSCInputStream inStream (buffer, sizeof (buffer));

            OSCTimeTag tag = inStream.readTimeTag();
            expect (tag.isImmediately());
        }
        {
            char buffer[8] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
            OSCInputStream inStream (buffer, sizeof (buffer));

            OSCTimeTag tag = inStream.readTimeTag();
            expect (! tag.isImmediately());
        }

        beginTest ("reading OSC arguments");

        {
            // test data:
            int testInt = -2015;
            const uint8 testIntRepresentation[] =  { 0xFF, 0xFF, 0xF8, 0x21 }; // big endian two's complement

            float testFloat = 345.6125f;
            const uint8 testFloatRepresentation[] = { 0x43, 0xAC, 0xCE, 0x66 }; // big endian IEEE 754

            String testString = "Hello, World!";
            const char testStringRepresentation[] = {
                'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                'o', 'r', 'l', 'd', '!', '\0', '\0', '\0' }; // padded to size % 4 == 0

            const uint8 testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
            const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));
            const uint8 testBlobRepresentation[] = {
                0x00, 0x00, 0x00, 0x05,
                0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00 }; // size prefixed + padded to size % 4 == 0

            // read:
            {
                {
                    // int32:
                    OSCInputStream inStream (testIntRepresentation, sizeof (testIntRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::int32);

                    expect (inStream.getPosition() == 4);
                    expect (arg.isInt32());
                    expectEquals (arg.getInt32(), testInt);
                }
                {
                    // float32:
                    OSCInputStream inStream (testFloatRepresentation, sizeof (testFloatRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::float32);

                    expect (inStream.getPosition() == 4);
                    expect (arg.isFloat32());
                    expectEquals (arg.getFloat32(), testFloat);
                }
                {
                    // string:
                    OSCInputStream inStream (testStringRepresentation, sizeof (testStringRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::string);

                    expect (inStream.getPosition() == sizeof (testStringRepresentation));
                    expect (arg.isString());
                    expectEquals (arg.getString(), testString);
                }
                {
                    // blob:
                    OSCInputStream inStream (testBlobRepresentation, sizeof (testBlobRepresentation));
                    OSCArgument arg = inStream.readArgument (OSCTypes::blob);

                    expect (inStream.getPosition() == sizeof (testBlobRepresentation));
                    expect (arg.isBlob());
                    expect (arg.getBlob() == testBlob);
                }
            }

            // read invalid representations:

            {
                // not enough bytes
                {
                    const uint8 rawData[] = { 0xF8, 0x21 };

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::int32), OSCFormatError);
                    expectThrowsType (inStream.readArgument (OSCTypes::float32), OSCFormatError);
                }

                // test string not being padded to multiple of 4 bytes:
                {
                    const char rawData[] = {
                        'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                        'o', 'r', 'l', 'd', '!', '\0' }; // padding missing

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::string), OSCFormatError);
                }
                {
                    const char rawData[] = {
                        'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                        'o', 'r', 'l', 'd', '!', '\0', 'x', 'x' }; // padding with non-zero chars

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::string), OSCFormatError);
                }

                // test blob not being padded to multiple of 4 bytes:
                {
                    const uint8 rawData[] = { 0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }; // padding missing

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::blob), OSCFormatError);
                }

                // test blob having wrong size
                {
                    const uint8 rawData[] = { 0x00, 0x00, 0x00, 0x12, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

                    OSCInputStream inStream (rawData, sizeof (rawData));

                    expectThrowsType (inStream.readArgument (OSCTypes::blob), OSCFormatError);
                }
            }
        }

        beginTest ("reading OSC messages (type tag string)");
        {
            {
                // valid empty message
                const char data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));

                auto msg = inStream.readMessage();
                expect (msg.getAddressPattern().toString() == "/test");
                expect (msg.size() == 0);
            }

            {
                // invalid message: no type tag string
                const char data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W',
                    'o', 'r', 'l', 'd', '!', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                // invalid message: no type tag string and also empty
                const char data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            // invalid message: wrong padding
            {
                const char data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0', ',', '\0', '\0', '\0' };
                OSCInputStream inStream (data, sizeof (data) - 1);

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            // invalid message: says it contains an arg, but doesn't
            {
                const char data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0', ',', 'i', '\0', '\0' };
                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            // invalid message: binary size does not match size deducted from type tag string
            {
                const char data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0', ',', 'i', 'f', '\0' };
                OSCInputStream inStream (data, sizeof (data));

                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }
        }

        beginTest ("reading OSC messages (contents)");
        {
            // valid non-empty message.

            {
                int32 testInt = -2015;
                float testFloat = 345.6125f;
                String testString = "Hello, World!";

                const uint8 testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
                const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));

                uint8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', 'i', 'f', 's', 'b', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));

                auto msg = inStream.readMessage();

                expectEquals (msg.getAddressPattern().toString(), String ("/test"));
                expectEquals (msg.size(), 4);

                expectEquals (msg[0].getType(), OSCTypes::int32);
                expectEquals (msg[1].getType(), OSCTypes::float32);
                expectEquals (msg[2].getType(), OSCTypes::string);
                expectEquals (msg[3].getType(), OSCTypes::blob);

                expectEquals (msg[0].getInt32(), testInt);
                expectEquals (msg[1].getFloat32(), testFloat);
                expectEquals (msg[2].getString(), testString);
                expect (msg[3].getBlob() == testBlob);
            }
        }
        beginTest ("reading OSC messages (handling of corrupted messages)");
        {
            // invalid messages

            {
                OSCInputStream inStream (nullptr, 0);
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                const uint8 data[] = { 0x00 };
                OSCInputStream inStream (data, 0);
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                uint8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', 'i', 'f', 's', 'b',   // type tag string not padded
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                uint8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    ',', 'i', 'f', 's', 'b', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0'  // rest of message cut off
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }
        }

        beginTest ("reading OSC messages (handling messages without type tag strings)");
        {

            {
                uint8 data[] = { '/', 't', 'e', 's', 't', '\0', '\0', '\0' };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }

            {
                uint8 data[] = {
                    '/', 't', 'e', 's', 't', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readMessage(), OSCFormatError);
            }
        }

        beginTest ("reading OSC bundles");
        {
            // valid bundle (empty)
            {
                uint8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
                };

                OSCInputStream inStream (data, sizeof (data));
                OSCBundle bundle = inStream.readBundle();

                expect (bundle.getTimeTag().isImmediately());
                expect (bundle.size() == 0);
            }

            // valid bundle (containing both messages and other bundles)

            {
                int32 testInt = -2015;
                float testFloat = 345.6125f;
                String testString = "Hello, World!";
                const uint8 testBlobData[] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
                const MemoryBlock testBlob (testBlobData, sizeof (testBlobData));

                uint8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,

                    0x00, 0x00, 0x00, 0x34,

                    '/', 't', 'e', 's', 't', '/', '1', '\0',
                    ',', 'i', 'f', 's', 'b', '\0', '\0', '\0',
                    0xFF, 0xFF, 0xF8, 0x21,
                    0x43, 0xAC, 0xCE, 0x66,
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0',
                    0x00, 0x00, 0x00, 0x05, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x00, 0x00,

                    0x00, 0x00, 0x00, 0x0C,

                    '/', 't', 'e', 's', 't', '/', '2', '\0',
                     ',', '\0', '\0', '\0',

                    0x00, 0x00, 0x00, 0x10,

                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
                };

                OSCInputStream inStream (data, sizeof (data));
                OSCBundle bundle = inStream.readBundle();

                expect (bundle.getTimeTag().isImmediately());
                expect (bundle.size() == 3);

                OSCBundle::Element* elements = bundle.begin();

                expect (elements[0].isMessage());
                expect (elements[0].getMessage().getAddressPattern().toString() == "/test/1");
                expect (elements[0].getMessage().size() == 4);
                expect (elements[0].getMessage()[0].isInt32());
                expect (elements[0].getMessage()[1].isFloat32());
                expect (elements[0].getMessage()[2].isString());
                expect (elements[0].getMessage()[3].isBlob());
                expectEquals (elements[0].getMessage()[0].getInt32(), testInt);
                expectEquals (elements[0].getMessage()[1].getFloat32(), testFloat);
                expectEquals (elements[0].getMessage()[2].getString(), testString);
                expect (elements[0].getMessage()[3].getBlob() == testBlob);

                expect (elements[1].isMessage());
                expect (elements[1].getMessage().getAddressPattern().toString() == "/test/2");
                expect (elements[1].getMessage().size() == 0);

                expect (elements[2].isBundle());
                expect (! elements[2].getBundle().getTimeTag().isImmediately());
            }

            // invalid bundles.

            {
                uint8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,

                    0x00, 0x00, 0x00, 0x34,  // wrong bundle element size (too large)

                    '/', 't', 'e', 's', 't', '/', '1', '\0',
                    ',', 's', '\0', '\0',
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0'
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                uint8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,

                    0x00, 0x00, 0x00, 0x08,  // wrong bundle element size (too small)

                    '/', 't', 'e', 's', 't', '/', '1', '\0',
                    ',', 's', '\0', '\0',
                    'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0', '\0', '\0'
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                uint8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', '\0',
                    0x00, 0x00, 0x00, 0x00  // incomplete time tag
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                uint8 data[] = {
                    '#', 'b', 'u', 'n', 'x', 'l', 'e', '\0', // wrong initial string
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }

            {
                uint8 data[] = {
                    '#', 'b', 'u', 'n', 'd', 'l', 'e', // padding missing from string
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };

                OSCInputStream inStream (data, sizeof (data));
                expectThrowsType (inStream.readBundle(), OSCFormatError);
            }
        }
    }
};

static OSCInputStreamTests OSCInputStreamUnitTests;

#endif

} // namespace


