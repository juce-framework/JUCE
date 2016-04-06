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
        listeners.call (&ActivityList::Listener::classListChanged, newList);
    }

private:
    StringArray activities;
    ListenerList<Listener> listeners;

    static void checkThread()
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActivityList)
};
