/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_TimeSliceThread.h"
#include "juce_ScopedLock.h"


//==============================================================================
TimeSliceThread::TimeSliceThread (const String& threadName)
    : Thread (threadName),
      index (0),
      clientBeingCalled (0),
      clientsChanged (false)
{
}

TimeSliceThread::~TimeSliceThread()
{
    stopThread (2000);
}

//==============================================================================
void TimeSliceThread::addTimeSliceClient (TimeSliceClient* const client)
{
    const ScopedLock sl (listLock);
    clients.addIfNotAlreadyThere (client);
    clientsChanged = true;
    notify();
}

void TimeSliceThread::removeTimeSliceClient (TimeSliceClient* const client)
{
    const ScopedLock sl1 (listLock);
    clientsChanged = true;

    // if there's a chance we're in the middle of calling this client, we need to
    // also lock the outer lock..
    if (clientBeingCalled == client)
    {
        const ScopedUnlock ul (listLock); // unlock first to get the order right..

        const ScopedLock sl1 (callbackLock);
        const ScopedLock sl2 (listLock);

        clients.removeValue (client);
    }
    else
    {
        clients.removeValue (client);
    }
}

int TimeSliceThread::getNumClients() const throw()
{
    return clients.size();
}

TimeSliceClient* TimeSliceThread::getClient (const int index) const throw()
{
    const ScopedLock sl (listLock);
    return clients [index];
}

//==============================================================================
void TimeSliceThread::run()
{
    int numCallsSinceBusy = 0;

    while (! threadShouldExit())
    {
        int timeToWait = 500;

        {
            const ScopedLock sl (callbackLock);

            {
                const ScopedLock sl (listLock);

                if (clients.size() > 0)
                {
                    index = (index + 1) % clients.size();

                    clientBeingCalled = clients [index];
                }
                else
                {
                    index = 0;
                    clientBeingCalled = 0;
                }

                if (clientsChanged)
                {
                    clientsChanged = false;
                    numCallsSinceBusy = 0;
                }
            }

            if (clientBeingCalled != 0)
            {
                if (clientBeingCalled->useTimeSlice())
                    numCallsSinceBusy = 0;
                else
                    ++numCallsSinceBusy;

                if (numCallsSinceBusy >= clients.size())
                    timeToWait = 500;
                else if (index == 0)
                    timeToWait = 1;   // throw in an occasional pause, to stop everything locking up
                else
                    timeToWait = 0;
            }
        }

        if (timeToWait > 0)
            wait (timeToWait);
    }
}


END_JUCE_NAMESPACE
