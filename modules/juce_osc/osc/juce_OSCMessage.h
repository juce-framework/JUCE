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

#ifndef JUCE_OSCMESSAGE_H_INCLUDED
#define JUCE_OSCMESSAGE_H_INCLUDED


//==============================================================================
/**
    An OSC Message.

    An OSCMessage consists of an OSCAddressPattern and zero or more OSCArguments.

    OSC messages are the elementary objects that are used to exchange any data
    via OSC. An OSCSender can send OSCMessage objects to an OSCReceiver.
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


   #if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
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
   #endif

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
    OSCArgument& operator[] (const int i) const noexcept;

    /** Returns a pointer to the first OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    OSCArgument* begin() const noexcept;

    /** Returns a pointer to the last OSCArgument in the OSCMessage object.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    OSCArgument* end() const noexcept;

    /** Removes all arguments from the OSCMessage. */
    void clear();


    //==============================================================================
    /** Creates a new OSCArgument of type int32 with a given value
        and adds it to the OSCMessage object.
    */
    void addInt32 (int32 value);

    /** Creates a new OSCArgument of type float32 with a given value
        and adds it to the OSCMessage object.
    */
    void addFloat32 (float value);

    /** Creates a new OSCArgument of type string with a given value
        and adds it to the OSCMessage object.
    */
    void addString (const String& value);

    /** Creates a new OSCArgument of type blob with binary data content copied from
        the given MemoryBlock.

        Note: if the argument passed is an lvalue, this may copy the binary data.
    */
    void addBlob (const MemoryBlock& blob);

    /** Adds the OSCArgument argument to the OSCMessage object.

        Note: This method will result in a copy of the OSCArgument object if it is passed
        as an lvalue. If the OSCArgument is of type blob, this will also copy the underlying
        binary data. In general, you should use addInt32, addFloat32, etc. instead.
    */
    void addArgument (OSCArgument argument);

private:

    //==============================================================================
   #if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    template <typename Arg1, typename... Args>
    void addArguments (Arg1&& arg1, Args&&... args)
    {
        addArgument (arg1);
        addArguments (std::forward<Args> (args)...);
    }

    void addArguments() {}
   #endif

    //==============================================================================
    OSCAddressPattern addressPattern;
    Array<OSCArgument> arguments;
};


//==============================================================================
#if JUCE_COMPILER_SUPPORTS_VARIADIC_TEMPLATES && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
 template <typename Arg1, typename... Args>
 OSCMessage::OSCMessage (const OSCAddressPattern& ap, Arg1&& arg1, Args&&... args)
     : addressPattern (ap)
 {
     addArguments (std::forward<Arg1> (arg1), std::forward<Args> (args)...);
 }
#endif

#endif // JUCE_OSCMESSAGE_H_INCLUDED
