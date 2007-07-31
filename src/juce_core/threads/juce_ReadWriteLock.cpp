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


#include "juce_ReadWriteLock.h"
#include "juce_ScopedLock.h"
#include "juce_Thread.h"


//==============================================================================
ReadWriteLock::ReadWriteLock() throw()
    : numWaitingWriters (0),
      numWriters (0),
      writerThreadId (0)
{
}

ReadWriteLock::~ReadWriteLock() throw()
{
    jassert (readerThreads.size() == 0);
    jassert (numWriters == 0);
}

//==============================================================================
void ReadWriteLock::enterRead() const throw()
{
    const int threadId = Thread::getCurrentThreadId();
    const ScopedLock sl (accessLock);

    for (;;)
    {
        jassert (readerThreads.size() % 2 == 0);

        int i;
        for (i = 0; i < readerThreads.size(); i += 2)
            if (readerThreads.getUnchecked(i) == threadId)
                break;

        if (i < readerThreads.size()
              || numWriters + numWaitingWriters == 0
              || (threadId == writerThreadId && numWriters > 0))
        {
            if (i < readerThreads.size())
            {
                readerThreads.set (i + 1, readerThreads.getUnchecked (i + 1) + 1);
            }
            else
            {
                readerThreads.add (threadId);
                readerThreads.add (1);
            }

            return;
        }

        const ScopedUnlock ul (accessLock);
        waitEvent.wait (100);
    }
}

void ReadWriteLock::exitRead() const throw()
{
    const int threadId = Thread::getCurrentThreadId();
    const ScopedLock sl (accessLock);

    for (int i = 0; i < readerThreads.size(); i += 2)
    {
        if (readerThreads.getUnchecked(i) == threadId)
        {
            const int newCount = readerThreads.getUnchecked (i + 1) - 1;

            if (newCount == 0)
            {
                readerThreads.removeRange (i, 2);
                waitEvent.signal();
            }
            else
            {
                readerThreads.set (i + 1, newCount);
            }

            return;
        }
    }

    jassertfalse // unlocking a lock that wasn't locked..
}

//==============================================================================
void ReadWriteLock::enterWrite() const throw()
{
    const int threadId = Thread::getCurrentThreadId();
    const ScopedLock sl (accessLock);

    for (;;)
    {
        if (readerThreads.size() + numWriters == 0
             || threadId == writerThreadId
             || (readerThreads.size() == 2
                  && readerThreads.getUnchecked(0) == threadId))
        {
            writerThreadId = threadId;
            ++numWriters;
            break;
        }

        ++numWaitingWriters;
        accessLock.exit();
        waitEvent.wait (100);
        accessLock.enter();
        --numWaitingWriters;
    }
}

void ReadWriteLock::exitWrite() const throw()
{
    const ScopedLock sl (accessLock);

    // check this thread actually had the lock..
    jassert (numWriters > 0 && writerThreadId == Thread::getCurrentThreadId());

    if (--numWriters == 0)
    {
        writerThreadId = 0;
        waitEvent.signal();
    }
}

END_JUCE_NAMESPACE
