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

#ifndef JUCE_OSCARGUMENT_H_INCLUDED
#define JUCE_OSCARGUMENT_H_INCLUDED


//==============================================================================
/**
    An OSC argument.

    An OSC argument is a value of one of the following types: int32, float32, string,
    or blob (raw binary data).

    OSCMessage objects are essentially arrays of OSCArgument objects.
*/
class JUCE_API  OSCArgument
{
public:
    /** Constructs an OSCArgument with type int32 and a given value. */
    OSCArgument (int32 value) noexcept;

    /** Constructs an OSCArgument with type float32 and a given value. */
    OSCArgument (float value) noexcept;

    /** Constructs an OSCArgument with type string and a given value */
    OSCArgument (const String& value) noexcept;

    /** Constructs an OSCArgument with type blob and copies dataSize bytes
        from the memory pointed to by data into the blob.

        The data owned by the blob will be released when the OSCArgument object
        gets destructed.
    */
    OSCArgument (const MemoryBlock& blob);

    /** Returns the type of the OSCArgument as an OSCType.
        OSCType is a char type, and its value will be the OSC type tag of the type.
    */
    OSCType getType() const noexcept        { return type; }

    /** Returns whether the type of the OSCArgument is int32. */
    bool isInt32() const noexcept           { return type == OSCTypes::int32; }

    /** Returns whether the type of the OSCArgument is int32. */
    bool isFloat32() const noexcept         { return type == OSCTypes::float32; }

    /** Returns whether the type of the OSCArgument is int32. */
    bool isString() const noexcept          { return type == OSCTypes::string; }

    /** Returns whether the type of the OSCArgument is int32. */
    bool isBlob() const noexcept            { return type == OSCTypes::blob; }

    /** Returns the value of the OSCArgument as an int32.
        If the type of the OSCArgument is not int32, the behaviour is undefined.
     */
    int32 getInt32() const noexcept;

    /** Returns the value of the OSCArgument as a float32.
        If the type of the OSCArgument is not float32, the behaviour is undefined.
     */
    float getFloat32() const noexcept;

    /** Returns the value of the OSCArgument as a string.
        If the type of the OSCArgument is not string, the behaviour is undefined.
     */
    String getString() const noexcept;

    /** Returns the binary data contained in the blob and owned by the OSCArgument,
        as a reference to a Juce MemoryBlock object.

        If the type of the OSCArgument is not blob, the behaviour is undefined.
     */
    const MemoryBlock& getBlob() const noexcept;


private:
    //==============================================================================
    OSCType type;

    union
    {
        int32 intValue;
        float floatValue;
    };

    String stringValue;
    MemoryBlock blob;
};


#endif // JUCE_OSCARGUMENT_H_INCLUDED
