/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_MESSAGE_JUCEHEADER__
#define __JUCE_MESSAGE_JUCEHEADER__

class MessageListener;
class MessageManager;

//==============================================================================
/** The base class for objects that can be delivered to a MessageListener.

    The simplest Message object contains a few integer and pointer parameters
    that the user can set, and this is enough for a lot of purposes. For passing more
    complex data, subclasses of Message can also be used.

    @see MessageListener, MessageManager, ActionListener, ChangeListener
*/
class JUCE_API  Message
{
public:
    //==============================================================================
    /** Creates an uninitialised message.

        The class's variables will also be left uninitialised.
    */
    Message() throw();

    /** Creates a message object, filling in the member variables.

        The corresponding public member variables will be set from the parameters
        passed in.
    */
    Message (const int intParameter1,
             const int intParameter2,
             const int intParameter3,
             void* const pointerParameter) throw();

    /** Destructor. */
    virtual ~Message() throw();

    //==============================================================================
    // These values can be used for carrying simple data that the application needs to
    // pass around. For more complex messages, just create a subclass.

    int intParameter1;          /**< user-defined integer value. */
    int intParameter2;          /**< user-defined integer value. */
    int intParameter3;          /**< user-defined integer value. */
    void* pointerParameter;     /**< user-defined pointer value. */


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class MessageListener;
    friend class MessageManager;
    MessageListener* messageRecipient;

    Message (const Message&);
    const Message& operator= (const Message&);
};


#endif   // __JUCE_MESSAGE_JUCEHEADER__
