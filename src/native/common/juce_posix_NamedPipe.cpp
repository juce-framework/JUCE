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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

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
    while (internal != nullptr && static_cast <NamedPipeInternal*> (internal)->blocked)
    {
        NamedPipeInternal* const intern = static_cast <NamedPipeInternal*> (internal);

        intern->stopReadOperation = true;

        char buffer [1] = { 0 };
        int bytesWritten = (int) ::write (intern->pipeIn, buffer, 1);
        (void) bytesWritten;

        int timeout = 2000;
        while (intern->blocked && --timeout >= 0)
            Thread::sleep (2);

        intern->stopReadOperation = false;
    }
}

void NamedPipe::close()
{
    NamedPipeInternal* const intern = static_cast <NamedPipeInternal*> (internal);

    if (intern != nullptr)
    {
        internal = nullptr;

        if (intern->pipeIn != -1)
            ::close (intern->pipeIn);

        if (intern->pipeOut != -1)
            ::close (intern->pipeOut);

        if (intern->createdPipe)
        {
            unlink (intern->pipeInName.toUTF8());
            unlink (intern->pipeOutName.toUTF8());
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

    const String pipePath ("/tmp/" + File::createLegalFileName (pipeName));

    intern->pipeInName  = pipePath + "_in";
    intern->pipeOutName = pipePath + "_out";
    intern->pipeIn = -1;
    intern->pipeOut = -1;

    if (createPipe)
    {
        if ((mkfifo (intern->pipeInName.toUTF8(), 0666) && errno != EEXIST)
            || (mkfifo (intern->pipeOutName.toUTF8(), 0666) && errno != EEXIST))
        {
            delete intern;
            internal = nullptr;

            return false;
        }
    }

    return true;
}

int NamedPipe::read (void* destBuffer, int maxBytesToRead, int /*timeOutMilliseconds*/)
{
    int bytesRead = -1;
    NamedPipeInternal* const intern = static_cast <NamedPipeInternal*> (internal);

    if (intern != nullptr)
    {
        intern->blocked = true;

        if (intern->pipeIn == -1)
        {
            if (intern->createdPipe)
                intern->pipeIn = ::open (intern->pipeInName.toUTF8(), O_RDWR);
            else
                intern->pipeIn = ::open (intern->pipeOutName.toUTF8(), O_RDWR);

            if (intern->pipeIn == -1)
            {
                intern->blocked = false;
                return -1;
            }
        }

        bytesRead = 0;

        char* p = static_cast<char*> (destBuffer);

        while (bytesRead < maxBytesToRead)
        {
            const int bytesThisTime = maxBytesToRead - bytesRead;
            const int numRead = (int) ::read (intern->pipeIn, p, bytesThisTime);

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
    NamedPipeInternal* const intern = static_cast <NamedPipeInternal*> (internal);

    if (intern != nullptr)
    {
        if (intern->pipeOut == -1)
        {
            if (intern->createdPipe)
                intern->pipeOut = ::open (intern->pipeOutName.toUTF8(), O_WRONLY);
            else
                intern->pipeOut = ::open (intern->pipeInName.toUTF8(), O_WRONLY);

            if (intern->pipeOut == -1)
            {
                return -1;
            }
        }

        const char* p = static_cast<const char*> (sourceBuffer);
        bytesWritten = 0;

        const uint32 timeOutTime = Time::getMillisecondCounter() + timeOutMilliseconds;

        while (bytesWritten < numBytesToWrite
               && (timeOutMilliseconds < 0 || Time::getMillisecondCounter() < timeOutTime))
        {
            const int bytesThisTime = numBytesToWrite - bytesWritten;
            const int numWritten = (int) ::write (intern->pipeOut, p, bytesThisTime);

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

#endif
