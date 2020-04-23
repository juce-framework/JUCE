/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
