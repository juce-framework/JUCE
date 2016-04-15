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

OSCArgument::OSCArgument (int32 value) noexcept
    : type (OSCTypes::int32), intValue (value)
{
}

OSCArgument::OSCArgument (float value) noexcept
    : type (OSCTypes::float32), floatValue (value)
{
}

OSCArgument::OSCArgument (const String& value) noexcept
    : type (OSCTypes::string), stringValue (value)
{
}

OSCArgument::OSCArgument (const MemoryBlock& b)
    : type (OSCTypes::blob), blob (b)
{
}

//==============================================================================
String OSCArgument::getString() const noexcept
{
    if (isString())
        return stringValue;

    jassertfalse; // you must check the type of an argument before attempting to get its value!
    return String();
}

int32 OSCArgument::getInt32() const noexcept
{
    if (isInt32())
        return intValue;

    jassertfalse; // you must check the type of an argument before attempting to get its value!
    return 0;
}

float OSCArgument::getFloat32() const noexcept
{
    if (isFloat32())
        return floatValue;

    jassertfalse; // you must check the type of an argument before attempting to get its value!
    return 0.0f;
}

const MemoryBlock& OSCArgument::getBlob() const noexcept
{
    // you must check the type of an argument before attempting to get its value!
    jassert (isBlob());

    return blob;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class OSCArgumentTests  : public UnitTest
{
public:
    OSCArgumentTests() : UnitTest ("OSCArgument class") {}


    MemoryBlock getMemoryBlockWithRandomData (size_t numBytes)
    {
        MemoryBlock block (numBytes);

        Random rng = getRandom();

        for (size_t i = 0; i < numBytes; ++i)
            block[i] = (char) rng.nextInt (256);

        return block;
    }

    void runTest()
    {
        runTestInitialisation();
    }

    void runTestInitialisation()
    {
        beginTest ("Int32");
        {
            int value = 123456789;

            OSCArgument arg (value);

            expect (arg.getType() == OSCTypes::int32);

            expect (arg.isInt32());
            expect (! arg.isFloat32());
            expect (! arg.isString());
            expect (! arg.isBlob());

            expect (arg.getInt32() == value);
        }

        beginTest ("Float32");
        {
            float value = 12345.6789f;

            OSCArgument arg (value);

            expect (arg.getType() == OSCTypes::float32);

            expect (! arg.isInt32());
            expect (arg.isFloat32());
            expect (! arg.isString());
            expect (! arg.isBlob());

            expect (arg.getFloat32() == value);


        }

        beginTest ("String");
        {
            String value = "Hello, World!";
            OSCArgument arg (value);

            expect (arg.getType() == OSCTypes::string);

            expect (! arg.isInt32());
            expect (! arg.isFloat32());
            expect (arg.isString());
            expect (! arg.isBlob());

            expect (arg.getString() == value);
        }

        beginTest ("String (from C string)");
        {
            OSCArgument arg ("Hello, World!");

            expect (arg.getType() == OSCTypes::string);

            expect (! arg.isInt32());
            expect (! arg.isFloat32());
            expect (arg.isString());
            expect (! arg.isBlob());

            expect (arg.getString() == String ("Hello, World!"));
        }

        beginTest ("Blob");
        {
            const size_t numBytes = 412;
            MemoryBlock blob = getMemoryBlockWithRandomData (numBytes);

            OSCArgument arg (blob);

            expect (arg.getType() == OSCTypes::blob);

            expect (! arg.isInt32());
            expect (! arg.isFloat32());
            expect (! arg.isString());
            expect (arg.isBlob());

            expect (arg.getBlob() == blob);
        }

        beginTest ("Copy, move and assignment");
        {
            {
                int value = -42;
                OSCArgument arg (value);

                OSCArgument copy = arg;
                expect (copy.getType() == OSCTypes::int32);
                expect (copy.getInt32() == value);

                OSCArgument assignment ("this will be overwritten!");
                assignment = copy;
                expect (assignment.getType() == OSCTypes::int32);
                expect (assignment.getInt32() == value);
           }
           {
                const size_t numBytes = 412;
                MemoryBlock blob = getMemoryBlockWithRandomData (numBytes);
                OSCArgument arg (blob);

                OSCArgument copy = arg;
                expect (copy.getType() == OSCTypes::blob);
                expect (copy.getBlob() == blob);

                OSCArgument assignment ("this will be overwritten!");
                assignment = copy;
                expect (assignment.getType() == OSCTypes::blob);
                expect (assignment.getBlob() == blob);

               #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
                OSCArgument move = std::move (arg);
                expect (move.getType() == OSCTypes::blob);
                expect (move.getBlob() == blob);

                OSCArgument moveAssignment ("this will be overwritten!");
                moveAssignment = std::move (copy);
                expect (moveAssignment.getType() == OSCTypes::blob);
                expect (moveAssignment.getBlob() == blob);
               #endif
           }
        }
    }
};

static OSCArgumentTests OSCArgumentUnitTests;

#endif // JUCE_UNIT_TESTS
