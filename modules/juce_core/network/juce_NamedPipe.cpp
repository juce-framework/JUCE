/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

NamedPipe::NamedPipe()
{
}

NamedPipe::~NamedPipe()
{
    close();
}

bool NamedPipe::openExisting (const String& pipeName)
{
    close();

    ScopedWriteLock sl (lock);
    currentPipeName = pipeName;
    return openInternal (pipeName, false, false);
}

bool NamedPipe::isOpen() const
{
    return pimpl != nullptr;
}

bool NamedPipe::createNewPipe (const String& pipeName, bool mustNotExist)
{
    close();

    ScopedWriteLock sl (lock);
    currentPipeName = pipeName;
    return openInternal (pipeName, true, mustNotExist);
}

String NamedPipe::getName() const
{
    return currentPipeName;
}

// other methods for this class are implemented in the platform-specific files
