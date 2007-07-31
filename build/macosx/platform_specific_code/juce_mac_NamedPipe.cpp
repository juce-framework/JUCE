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

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>

// As well as being for the mac, this file is included by the linux build.

#if JUCE_MAC
 #include <Carbon/Carbon.h>
#else
 #include <sys/wait.h>
 #include <errno.h>
 #include <unistd.h>
#endif

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/io/files/juce_File.h"
#include "../../../src/juce_core/io/files/juce_NamedPipe.h"


//==============================================================================
struct NamedPipeInternal
{
    String pipeInName, pipeOutName;
    int pipeIn, pipeOut;

    bool volatile createdPipe, blocked, stopReadOperation;

    static void signalHandler (int) {}
};

void NamedPipe::cancelPendingReads()
{
    while (internal != 0 && ((NamedPipeInternal*) internal)->blocked)
    {
        NamedPipeInternal* const intern = (NamedPipeInternal*) internal;

        intern->stopReadOperation = true;

        char buffer [1] = { 0 };
        ::write (intern->pipeIn, buffer, 1);

        int timeout = 2000;
        while (intern->blocked && --timeout >= 0)
            sleep (2);

        intern->stopReadOperation = false;
    }
}

void NamedPipe::close()
{
    NamedPipeInternal* const intern = (NamedPipeInternal*) internal;

    if (intern != 0)
    {
        internal = 0;

        if (intern->pipeIn != -1)
            ::close (intern->pipeIn);

        if (intern->pipeOut != -1)
            ::close (intern->pipeOut);

        if (intern->createdPipe)
        {
            unlink (intern->pipeInName);
            unlink (intern->pipeOutName);
        }

        delete intern;
    }
}

bool NamedPipe::openInternal (const String& pipeName, const bool createPipe)
{
    close();

    NamedPipeInternal* const intern = new NamedPipeInternal();
    internal = intern;
    intern->createdPipe = createPipe;
    intern->blocked = false;
    intern->stopReadOperation = false;

    signal (SIGPIPE, NamedPipeInternal::signalHandler);
    siginterrupt (SIGPIPE, 1);

    const String pipePath (T("/tmp/") + File::createLegalFileName  (pipeName));

    intern->pipeInName  = pipePath + T("_in");
    intern->pipeOutName = pipePath + T("_out");
    intern->pipeIn = -1;
    intern->pipeOut = -1;

    if (createPipe)
    {
        if ((mkfifo (intern->pipeInName, 0666) && errno != EEXIST)
            || (mkfifo (intern->pipeOutName, 0666) && errno != EEXIST))
        {
            delete intern;
            internal = 0;

            return false;
        }
    }

    return true;
}

int NamedPipe::read (void* destBuffer, int maxBytesToRead, int /*timeOutMilliseconds*/)
{
    int bytesRead = -1;
    NamedPipeInternal* const intern = (NamedPipeInternal*) internal;

    if (intern != 0)
    {
        intern->blocked = true;

        if (intern->pipeIn == -1)
        {
            if (intern->createdPipe)
                intern->pipeIn = ::open (intern->pipeInName, O_RDWR);
            else
                intern->pipeIn = ::open (intern->pipeOutName, O_RDWR);

            if (intern->pipeIn == -1)
            {
                intern->blocked = false;
                return -1;
            }
        }

        bytesRead = 0;

        char* p = (char*) destBuffer;

        while (bytesRead < maxBytesToRead)
        {
            const int bytesThisTime = maxBytesToRead - bytesRead;
            const int numRead = ::read (intern->pipeIn, p, bytesThisTime);

            if (numRead <= 0 || intern->stopReadOperation)
            {
                bytesRead = -1;
                break;
            }

            bytesRead += numRead;
            p += bytesRead;
        }

        intern->blocked = false;
    }

    return bytesRead;
}

int NamedPipe::write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
{
    int bytesWritten = -1;
    NamedPipeInternal* const intern = (NamedPipeInternal*) internal;

    if (intern != 0)
    {
        if (intern->pipeOut == -1)
        {
            if (intern->createdPipe)
                intern->pipeOut = ::open (intern->pipeOutName, O_WRONLY);
            else
                intern->pipeOut = ::open (intern->pipeInName, O_WRONLY);

            if (intern->pipeOut == -1)
            {
                return -1;
            }
        }

        const char* p = (const char*) sourceBuffer;
        bytesWritten = 0;

        const uint32 timeOutTime = Time::getMillisecondCounter() + timeOutMilliseconds;

        while (bytesWritten < numBytesToWrite
               && (timeOutMilliseconds < 0 || Time::getMillisecondCounter() < timeOutTime))
        {
            const int bytesThisTime = numBytesToWrite - bytesWritten;
            const int numWritten = ::write (intern->pipeOut, p, bytesThisTime);

            if (numWritten <= 0)
            {
                bytesWritten = -1;
                break;
            }

            bytesWritten += numWritten;
            p += bytesWritten;
        }
    }

    return bytesWritten;
}


END_JUCE_NAMESPACE
