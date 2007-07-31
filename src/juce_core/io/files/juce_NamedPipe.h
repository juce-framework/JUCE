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

#ifndef __JUCE_NAMEDPIPE_JUCEHEADER__
#define __JUCE_NAMEDPIPE_JUCEHEADER__

#include "../juce_OutputStream.h"


//==============================================================================
/**
    A cross-process pipe that can have data written to and read from it.

    Two or more processes can use these for inter-process communication.

    @see InterprocessConnection
*/
class JUCE_API  NamedPipe
{
public:
    //==============================================================================
    /** Creates a NamedPipe. */
    NamedPipe();

    /** Destructor. */
    ~NamedPipe();


    //==============================================================================
    /** Tries to open a pipe that already exists.

        Returns true if it succeeds.
    */
    bool openExisting (const String& pipeName);

    /** Tries to create a new pipe.

        Returns true if it succeeds.
    */
    bool createNewPipe (const String& pipeName);

    /** Closes the pipe, if it's open. */
    void close();

    /** True if the pipe is currently open. */
    bool isOpen() const throw();

    /** Returns the last name that was used to try to open this pipe. */
    const String getName() const throw();

    //==============================================================================
    /** Reads data from the pipe.

        This will block until another thread has written enough data into the pipe to fill
        the number of bytes specified, or until another thread calls the cancelPendingReads()
        method.

        If the operation fails, it returns -1, otherwise, it will return the number of
        bytes read.

        If timeOutMilliseconds is less than zero, it will wait indefinitely, otherwise
        this is a maximum timeout for reading from the pipe.
    */
    int read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds = 5000);

    /** Writes some data to the pipe.

        If the operation fails, it returns -1, otherwise, it will return the number of
        bytes written.
    */
    int write (const void* sourceBuffer, int numBytesToWrite,
               int timeOutMilliseconds = 2000);

    /** If any threads are currently blocked on a read operation, this tells them to abort.
    */
    void cancelPendingReads();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    void* internal;
    String currentPipeName;

    NamedPipe (const NamedPipe&);
    const NamedPipe& operator= (const NamedPipe&);

    bool openInternal (const String& pipeName, const bool createPipe);
};


#endif   // __JUCE_NAMEDPIPE_JUCEHEADER__
