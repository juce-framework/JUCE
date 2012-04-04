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
         blocked (false), stopReadOperation (false)
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

    int read (char* destBuffer, int maxBytesToRead)
    {
        int bytesRead = -1;
        blocked = true;

        if (pipeIn == -1)
        {
            pipeIn = ::open ((createdPipe ? pipeInName
                                          : pipeOutName).toUTF8(), O_RDWR);

            if (pipeIn == -1)
            {
                blocked = false;
                return -1;
            }
        }

        bytesRead = 0;

        while (bytesRead < maxBytesToRead)
        {
            const int bytesThisTime = maxBytesToRead - bytesRead;
            const int numRead = (int) ::read (pipeIn, destBuffer, bytesThisTime);

            if (numRead <= 0 || stopReadOperation)
            {
                bytesRead = -1;
                break;
            }

            bytesRead += numRead;
            destBuffer += numRead;
        }

        blocked = false;
        return bytesRead;
    }

    int write (const char* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
    {
        int bytesWritten = -1;

        if (pipeOut == -1)
        {
            pipeOut = ::open ((createdPipe ? pipeOutName
                                           : pipeInName).toUTF8(), O_WRONLY);

            if (pipeOut == -1)
                return -1;
        }

        bytesWritten = 0;
        const uint32 timeOutTime = Time::getMillisecondCounter() + timeOutMilliseconds;

        while (bytesWritten < numBytesToWrite
                && (timeOutMilliseconds < 0 || Time::getMillisecondCounter() < timeOutTime))
        {
            const int bytesThisTime = numBytesToWrite - bytesWritten;
            const int numWritten = (int) ::write (pipeOut, sourceBuffer, bytesThisTime);

            if (numWritten <= 0)
            {
                bytesWritten = -1;
                break;
            }

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
    bool volatile blocked, stopReadOperation;

private:
    static void signalHandler (int) {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl);
};

NamedPipe::NamedPipe()
{
}

NamedPipe::~NamedPipe()
{
    close();
}

void NamedPipe::cancelPendingReads()
{
    while (pimpl != nullptr && pimpl->blocked)
    {
        pimpl->stopReadOperation = true;

        char buffer[1] = { 0 };
        int bytesWritten = (int) ::write (pimpl->pipeIn, buffer, 1);
        (void) bytesWritten;

        int timeout = 2000;
        while (pimpl->blocked && --timeout >= 0)
            Thread::sleep (2);

        pimpl->stopReadOperation = false;
    }
}

void NamedPipe::close()
{
    cancelPendingReads();
    ScopedPointer<Pimpl> deleter (pimpl); // (clears the pimpl member variable before deleting it)
}

bool NamedPipe::openInternal (const String& pipeName, const bool createPipe)
{
    close();

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

int NamedPipe::read (void* destBuffer, int maxBytesToRead, int /*timeOutMilliseconds*/)
{
    return pimpl != nullptr ? pimpl->read (static_cast <char*> (destBuffer), maxBytesToRead) : -1;
}

int NamedPipe::write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
{
    return pimpl != nullptr ? pimpl->write (static_cast <const char*> (sourceBuffer), numBytesToWrite, timeOutMilliseconds) : -1;
}

bool NamedPipe::isOpen() const
{
    return pimpl != nullptr;
}
