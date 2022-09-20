/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An OSC Message.

    An OSCMessage consists of an OSCAddressPattern and zero or more OSCArguments.

    OSC messages are the elementary objects that are used to exchange any data
    via OSC. An OSCSender can send OSCMessage objects to an OSCReceiver.

    @tags{OSC}
*/
class JUCE_API  OSCMessage
{
public:

    //==============================================================================
    /** Constructs an OSCMessage object with the given address pattern and no
        arguments.

        @param ap    the address pattern of the message. This must be a valid OSC
                     address (starting with a forward slash) and may contain
                     OSC wildcard expressions. You can pass in a string literal
                     or a juce String (they will be converted to an OSCAddressPattern
                     automatically).
    */
    OSCMessage (const OSCAddressPattern& ap) noexcept;


    /** Constructs an OSCMessage object with the given address pattern and list
        of arguments.

        @param ap    the address pattern of the message. This must be a valid OSC
                     address (starting with a forward slash) and may contain
                     OSC wildcard expressions. You can pass in a string literal
                     or a juce String (they will be converted to an OSCAddressPattern
                     automatically).

        @param arg1  the first argument of the message.
        @param args  an optional list of further arguments to add to the message.
    */
    template <typename Arg1, typename... Args>
    OSCMessage (const OSCAddressPattern& ap, Arg1&& arg1, Args&&... args);

    /** Sets the address pattern of the OSCMessage.

        @param ap    the address pattern of the message. This must be a valid OSC
                     address (starting with a forward slash) and may contain
                     OSC wildcard expressions. You can pass in a string literal
                     or a juce String (they will be converted to an OSCAddressPattern
                     automatically).
    */
    void setAddressPattern (const OSCAddressPattern& ap) noexcept;

    /** Returns the address pattern of the OSCMessage. */
    OSCAddressPattern getAddressPattern() const noexcept;

    /** Returns the number of OSCArgument objects that belong to this OSCMessage. */
    int size() const noexcept;

    /** Returns true if the OSCMessage contains no OSCArgument objects; false otherwise. */
    bool isEmpty() const noexcept;

    /** Returns a reference to the OSCArgument at index i in the OSCMessage object.
        This method does not check the range and results in undefined behaviour
        in case i < 0 or i >= size().
    */
    OSCArgument& operator[] (const int i) noexcept;
    const OSCArgument& operator[] (const int i) const noexcept;

    /** Returns a pointer to the first OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    OSCArgument* begin() noexcept;

    /** Returns a pointer to the first OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    const OSCArgument* begin() const noexcept;

    /** Returns a pointer to the last OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    OSCArgument* end() noexcept;

    /** Returns a pointer to the last OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    const OSCArgument* end() const noexcept;

    /** Removes all arguments from the OSCMessage. */
    void clear();

    //==============================================================================
    /** Creates a new OSCArgument of type int32 with the given value,
        and adds it to the OSCMessage object.
    */
    void addInt32 (int32 value);

    /** Creates a new OSCArgument of type float32 with the given value,
        and adds it to the OSCMessage object.
    */
    void addFloat32 (float value);

    /** Creates a new OSCArgument of type string with the given value,
        and adds it to the OSCMessage object.
    */
    void addString (const String& value);

    /** Creates a new OSCArgument of type blob with binary data content copied from
        the given MemoryBlock.

        Note: If the argument passed is an lvalue, this may copy the binary data.
    */
    void addBlob (MemoryBlock blob);

    /** Creates a new OSCArgument of type colour with the given value,
        and adds it to the OSCMessage object.
    */
    void addColour (OSCColour colour);

    /** Adds the OSCArgument argument to the OSCMessage object.

        Note: This method will result in a copy of the OSCArgument object if it is passed
        as an lvalue. If the OSCArgument is of type blob, this will also copy the underlying
        binary data. In general, you should use addInt32, addFloat32, etc. instead.
    */
    void addArgument (OSCArgument argument);

private:

    //==============================================================================
    template <typename Arg1, typename... Args>
    void addArguments (Arg1&& arg1, Args&&... args)
    {
        addArgument (arg1);
        addArguments (std::forward<Args> (args)...);
    }

    void addArguments() {}

    //==============================================================================
    OSCAddressPattern addressPattern;
    Array<OSCArgument> arguments;
};


//==============================================================================
template <typename Arg1, typename... Args>
OSCMessage::OSCMessage (const OSCAddressPattern& ap, Arg1&& arg1, Args&&... args)
    : addressPattern (ap)
{
    addArguments (std::forward<Arg1> (arg1), std::forward<Args> (args)...);
}

} // namespace juce
