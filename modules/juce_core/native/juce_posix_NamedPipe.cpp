/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

class NamedPipe::Pimpl
{
public:
    Pimpl (const String& pipePath, bool createPipe)
       : pipeInName  (pipePath + "_in"),
         pipeOutName (pipePath + "_out"),
         pipeIn (-1), pipeOut (-1),
         createdPipe (createPipe),
         stopReadOperation (false)
    {
        signal (SIGPIPE, signalHandler);
        siginterrupt (SIGPIPE, 1);
    }

    ~Pimpl()
    {
        if (pipeIn  != -1)  ::close (pipeIn);
        if (pipeOut != -1)  ::close (pipeOut);

        if (createdPipe)
        {
            unlink (pipeInName.toUTF8());
            unlink (pipeOutName.toUTF8());
        }
    }

    int read (char* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
    {
        const uint32 timeoutEnd = getTimeoutEnd (timeOutMilliseconds);

        if (pipeIn == -1)
        {
            pipeIn = openPipe (createdPipe ? pipeInName : pipeOutName, O_RDWR | O_NONBLOCK, timeoutEnd);

            if (pipeIn == -1)
                return -1;
        }

        int bytesRead = 0;

        while (bytesRead < maxBytesToRead)
        {
            const int bytesThisTime = maxBytesToRead - bytesRead;
            const int numRead = (int) ::read (pipeIn, destBuffer, (size_t) bytesThisTime);

            if (numRead <= 0)
            {
                if (errno != EWOULDBLOCK || stopReadOperation || hasExpired (timeoutEnd))
                    return -1;

                const int maxWaitingTime = 30;
                waitForInput (pipeIn, timeoutEnd == 0 ? maxWaitingTime
                                                      : jmin (maxWaitingTime,
                                                              (int) (timeoutEnd - Time::getMillisecondCounter())));
                continue;
            }

            bytesRead += numRead;
            destBuffer += numRead;
        }

        return bytesRead;
    }

    int write (const char* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
    {
        const uint32 timeoutEnd = getTimeoutEnd (timeOutMilliseconds);

        if (pipeOut == -1)
        {
            pipeOut = openPipe (createdPipe ? pipeOutName : pipeInName, O_WRONLY, timeoutEnd);

            if (pipeOut == -1)
                return -1;
        }

        int bytesWritten = 0;

        while (bytesWritten < numBytesToWrite && ! hasExpired (timeoutEnd))
        {
            const int bytesThisTime = numBytesToWrite - bytesWritten;
            const int numWritten = (int) ::write (pipeOut, sourceBuffer, (size_t) bytesThisTime);

            if (numWritten <= 0)
                return -1;

            bytesWritten += numWritten;
            sourceBuffer += numWritten;
        }

        return bytesWritten;
    }

    bool createFifos() const
    {
        return (mkfifo (pipeInName .toUTF8(), 0666) == 0 || errno == EEXIST)
            && (mkfifo (pipeOutName.toUTF8(), 0666) == 0 || errno == EEXIST);
    }

    const String pipeInName, pipeOutName;
    int pipeIn, pipeOut;

    const bool createdPipe;
    bool stopReadOperation;

private:
    static void signalHandler (int) {}

    static uint32 getTimeoutEnd (const int timeOutMilliseconds)
    {
        return timeOutMilliseconds >= 0 ? Time::getMillisecondCounter() + (uint32) timeOutMilliseconds : 0;
    }

    static bool hasExpired (const uint32 timeoutEnd)
    {
        return timeoutEnd != 0 && Time::getMillisecondCounter() >= timeoutEnd;
    }

    static int openPipe (const String& name, int flags, const uint32 timeoutEnd)
    {
        for (;;)
        {
            const int p = ::open (name.toUTF8(), flags);

            if (p != -1 || hasExpired (timeoutEnd))
                return p;

            Thread::sleep (2);
        }
    }

    static void waitForInput (const int handle, const int timeoutMsecs) noexcept
    {
        struct timeval timeout;
        timeout.tv_sec = timeoutMsecs / 1000;
        timeout.tv_usec = (timeoutMsecs % 1000) * 1000;

        fd_set rset;
        FD_ZERO (&rset);
        FD_SET (handle, &rset);

        select (handle + 1, &rset, nullptr, 0, &timeout);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

void NamedPipe::close()
{
    if (pimpl != nullptr)
    {
        pimpl->stopReadOperation = true;

        char buffer[1] = { 0 };
        ssize_t done = ::write (pimpl->pipeIn, buffer, 1);
        (void) done;

        ScopedWriteLock sl (lock);
        pimpl = nullptr;
    }
}

bool NamedPipe::openInternal (const String& pipeName, const bool createPipe)
{
   #if JUCE_IOS
    pimpl = new Pimpl (File::getSpecialLocation (File::tempDirectory)
                         .getChildFile (File::createLegalFileName (pipeName)).getFullPathName(), createPipe);
   #else
    pimpl = new Pimpl ("/tmp/" + File::createLegalFileName (pipeName), createPipe);
   #endif

    if (createPipe && ! pimpl->createFifos())
    {
        pimpl = nullptr;
        return false;
    }

    return true;
}

int NamedPipe::read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
{
    ScopedReadLock sl (lock);
    return pimpl != nullptr ? pimpl->read (static_cast <char*> (destBuffer), maxBytesToRead, timeOutMilliseconds) : -1;
}

int NamedPipe::write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
{
    ScopedReadLock sl (lock);
    return pimpl != nullptr ? pimpl->write (static_cast <const char*> (sourceBuffer), numBytesToWrite, timeOutMilliseconds) : -1;
}
