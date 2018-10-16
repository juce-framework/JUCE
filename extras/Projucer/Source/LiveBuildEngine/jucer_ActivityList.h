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
struct ActivityList   : public ChangeBroadcaster
{
    ActivityList() {}

    void setList (const StringArray& newList)
    {
        checkThread();

        if (activities != newList)
        {
            const bool wasEmpty = isEmpty();
            activities = newList;
            sendChangeMessage();

            if (wasEmpty != isEmpty())
                ProjucerApplication::getCommandManager().commandStatusChanged();
        }
    }

    void clear()
    {
        setList (StringArray());
    }

    StringArray getActivities() const
    {
        checkThread();

        StringArray s;

        for (auto a : activities)
            s.add (a.upToFirstOccurrenceOf ("|||", false, false));

        return s;
    }

    bool isEmpty() const noexcept
    {
        return activities.size() == 0;
    }

    int getNumActivities() const
    {
        checkThread();
        return activities.size();
    }

    struct Listener
    {
        virtual ~Listener() {}
        virtual void classListChanged (const ClassDatabase::ClassList&) = 0;
    };

    void addListener (Listener* l)
    {
        checkThread();
        listeners.add (l);
    }

    void removeListener (Listener* l)
    {
        checkThread();
        listeners.remove (l);
    }

    void sendClassListChangedMessage (const ClassDatabase::ClassList& newList)
    {
        checkThread();
        listeners.call ([&] (Listener& l) { l.classListChanged (newList); });
    }

private:
    StringArray activities;
    ListenerList<Listener> listeners;

    static void checkThread()
    {
        JUCE_ASSERT_MESSAGE_THREAD
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActivityList)
};
