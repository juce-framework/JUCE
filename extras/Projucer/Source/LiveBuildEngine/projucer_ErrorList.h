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
            for (DiagnosticMessage& d : messages)
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
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ErrorList)
};
