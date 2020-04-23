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
