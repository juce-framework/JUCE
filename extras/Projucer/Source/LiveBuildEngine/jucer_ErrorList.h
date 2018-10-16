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

#pragma once


//==============================================================================
struct ErrorList    : public ChangeBroadcaster
{
    ErrorList() : warningsEnabled (true) {}

    void takeCopy (Array<DiagnosticMessage>& dest) const
    {
        checkThread();

        if (warningsEnabled)
        {
            dest = messages;
        }
        else
        {
            for (auto& d : messages)
                if (d.isError())
                    dest.add (d);
        }
    }

    void resetToError (const String& message)
    {
        DiagnosticMessage m;
        m.message = message;
        m.type = DiagnosticMessage::error;

        DiagnosticList list;
        list.add (m);
        setList (list.toValueTree());
    }

    void setList (const ValueTree& newList)
    {
        checkThread();
        messages.clear();

        for (int i = 0; i < newList.getNumChildren(); ++i)
            messages.add (DiagnosticMessage::fromValueTree (newList.getChild(i)));

        sendChangeMessage();
    }

    bool isEmpty() const noexcept   { return messages.size() == 0; }

    int getNumErrors() const
    {
        checkThread();

        int num = 0;
        for (const auto& m : messages)
            if (m.isError())
                ++num;

        return num;
    }

    int getNumWarnings() const
    {
        checkThread();

        int num = 0;

        for (const auto& m : messages)
            if (m.isWarning())
                ++num;

        return num;
    }

    void setWarningsEnabled (bool enabled)
    {
        if (warningsEnabled != enabled)
        {
            warningsEnabled = enabled;

            if (messages.size() > 0)
                sendChangeMessage();
        }
    }

private:
    Array<DiagnosticMessage> messages;
    bool warningsEnabled;

    static void checkThread()
    {
        JUCE_ASSERT_MESSAGE_THREAD
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ErrorList)
};
