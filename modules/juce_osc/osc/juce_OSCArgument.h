/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An OSC argument.

    An OSC argument is a value of one of the following types: int32, float32, string,
    or blob (raw binary data).

    OSCMessage objects are essentially arrays of OSCArgument objects.

    @tags{OSC}
*/
class JUCE_API  OSCArgument
{
public:
    /** Constructs an OSCArgument with type I (for Impulse). */
    OSCArgument();
    
    /** Constructs an OSCArgument with type boolean T or F depending on the value. */
    OSCArgument(bool trueOrFalse);

    /** Constructs an OSCArgument with type int32 and a given value. */
    OSCArgument (int32 value);

    /** Constructs an OSCArgument with type float32 and a given value. */
    OSCArgument (float value);

    /** Constructs an OSCArgument with type string and a given value */
    OSCArgument (const String& value);

    /** Constructs an OSCArgument with type blob and copies dataSize bytes
        from the memory pointed to by data into the blob.

        The data owned by the blob will be released when the OSCArgument object
        gets destructed.
    */
    OSCArgument (MemoryBlock blob);

    /** Constructs an OSCArgument with type colour and a given colour value */
    OSCArgument (OSCColour colour);

    /** Returns the type of the OSCArgument as an OSCType.
        OSCType is a char type, and its value will be the OSC type tag of the type.
    */
    OSCType getType() const noexcept        { return type; }

    /** Returns whether the type of the OSCArgument is T or F. */
    bool isImpulse() const noexcept { return type == OSCTypes::I; }
   
    /** Returns whether the type of the OSCArgument is T or F. */
    bool isTorF() const noexcept { return type == OSCTypes::T || type == OSCTypes::F; }
    
    /** Returns whether the type of the OSCArgument is int32. */
    bool isInt32() const noexcept           { return type == OSCTypes::int32; }

    /** Returns whether the type of the OSCArgument is float. */
    bool isFloat32() const noexcept         { return type == OSCTypes::float32; }

    /** Returns whether the type of the OSCArgument is string. */
    bool isString() const noexcept          { return type == OSCTypes::string; }

    /** Returns whether the type of the OSCArgument is blob. */
    bool isBlob() const noexcept            { return type == OSCTypes::blob; }

    /** Returns whether the type of the OSCArgument is colour. */
    bool isColour() const noexcept          { return type == OSCTypes::colour; }

    /** Returns the value of the OSCArgument as an int32.
        If the type of the OSCArgument is not int32, the behaviour is undefined.
    */
    bool getBool() const noexcept;

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
        as a reference to a JUCE MemoryBlock object.

        If the type of the OSCArgument is not blob, the behaviour is undefined.
    */
    const MemoryBlock& getBlob() const noexcept;

    /** Returns the value of the OSCArgument as an OSCColour.
        If the type of the OSCArgument is not a colour, the behaviour is undefined.
    */
    OSCColour getColour() const noexcept;

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

} // namespace juce
