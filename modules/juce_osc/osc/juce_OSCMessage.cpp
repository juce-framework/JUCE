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

//==============================================================================
OSCMessage::OSCMessage (const OSCAddressPattern& ap) noexcept  : addressPattern (ap)
{
}

//==============================================================================
void OSCMessage::setAddressPattern (const OSCAddressPattern& ap) noexcept
{
    addressPattern = ap;
}

OSCAddressPattern OSCMessage::getAddressPattern() const noexcept
{
    return addressPattern;
}

//==============================================================================
int OSCMessage::size() const noexcept
{
    return arguments.size();
}

bool OSCMessage::isEmpty() const noexcept
{
    return arguments.isEmpty();
}

OSCArgument& OSCMessage::operator[] (const int i) const noexcept
{
    return arguments.getReference (i);
}

OSCArgument* OSCMessage::begin() const noexcept
{
    return arguments.begin();
}

OSCArgument* OSCMessage::end() const noexcept
{
    return arguments.end();
}

void OSCMessage::clear()
{
    arguments.clear();
}

//==============================================================================
void OSCMessage::addInt32 (int32 value)             { arguments.add (OSCArgument (value)); }
void OSCMessage::addFloat32 (float value)           { arguments.add (OSCArgument (value)); }
void OSCMessage::addString (const String& value)    { arguments.add (OSCArgument (value)); }
void OSCMessage::addBlob (const MemoryBlock& blob)  { arguments.add (OSCArgument (blob)); }
void OSCMessage::addArgument (OSCArgument arg)      { arguments.add (arg); }

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class OSCMessageTests  : public UnitTest
{
public:
    OSCMessageTests() : UnitTest ("OSCMessage class") {}

    void runTest()
    {
        beginTest ("Basic usage");
        {
            OSCMessage msg ("/test/param0");
            expectEquals (msg.size(), 0);
            expect (msg.getAddressPattern().toString() == "/test/param0");

            const int numTestArgs = 4;

            const int testInt = 42;
            const float testFloat = 3.14159f;
            const String testString = "Hello, World!";

            const uint8 testBlobData[5] = { 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };
            const MemoryBlock testBlob (testBlobData,  sizeof (testBlobData));

            msg.addInt32 (testInt);
            msg.addFloat32 (testFloat);
            msg.addString (testString);
            msg.addBlob (testBlob);

            expectEquals (msg.size(), numTestArgs);

            expectEquals (msg[0].getType(), OSCTypes::int32);
            expectEquals (msg[1].getType(), OSCTypes::float32);
            expectEquals (msg[2].getType(), OSCTypes::string);
            expectEquals (msg[3].getType(), OSCTypes::blob);

            expect (msg[0].isInt32());
            expect (msg[1].isFloat32());
            expect (msg[2].isString());
            expect (msg[3].isBlob());

            expectEquals (msg[0].getInt32(), testInt);
            expectEquals (msg[1].getFloat32(), testFloat);
            expectEquals (msg[2].getString(), testString);
            expect (msg[3].getBlob() == testBlob);

            expect (msg.begin() + numTestArgs == msg.end());

            OSCArgument* arg = msg.begin();
            expect (arg->isInt32());
            expectEquals (arg->getInt32(), testInt);
            ++arg;
            expect (arg->isFloat32());
            expectEquals (arg->getFloat32(), testFloat);
            ++arg;
            expect (arg->isString());
            expectEquals (arg->getString(), testString);
            ++arg;
            expect (arg->isBlob());
            expect(arg->getBlob() == testBlob);
            ++arg;
            expect (arg == msg.end());
        }


       #if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
        beginTest ("Initialisation with argument list (C++11 only)");
        {
            int testInt = 42;
            float testFloat = 5.5;
            String testString = "Hello, World!";

            {
                OSCMessage msg ("/test", testInt);
                expect (msg.getAddressPattern().toString() == String ("/test"));
                expectEquals (msg.size(), 1);
                expect (msg[0].isInt32());
                expectEquals (msg[0].getInt32(), testInt);
            }
            {
                OSCMessage msg ("/test", testFloat);
                expect (msg.getAddressPattern().toString() == String ("/test"));
                expectEquals (msg.size(), 1);
                expect (msg[0].isFloat32());
                expectEquals (msg[0].getFloat32(), testFloat);
            }
            {
                OSCMessage msg ("/test", testString);
                expect (msg.getAddressPattern().toString() == String ("/test"));
                expectEquals (msg.size(), 1);
                expect (msg[0].isString());
                expectEquals (msg[0].getString(), testString);
            }
            {
                OSCMessage msg ("/test", testInt, testFloat, testString, testFloat, testInt);
                expect (msg.getAddressPattern().toString() == String ("/test"));
                expectEquals (msg.size(), 5);
                expect (msg[0].isInt32());
                expect (msg[1].isFloat32());
                expect (msg[2].isString());
                expect (msg[3].isFloat32());
                expect (msg[4].isInt32());

                expectEquals (msg[0].getInt32(), testInt);
                expectEquals (msg[1].getFloat32(), testFloat);
                expectEquals (msg[2].getString(), testString);
                expectEquals (msg[3].getFloat32(), testFloat);
                expectEquals (msg[4].getInt32(), testInt);
            }
        }
       #endif
    }
};

static OSCMessageTests OSCMessageUnitTests;

#endif // JUCE_UNIT_TESTS
