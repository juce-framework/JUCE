/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

SubregionStream::SubregionStream (InputStream* const sourceStream,
                                  const int64 start, const int64 length,
                                  const bool deleteSourceWhenDestroyed)
  : source (sourceStream, deleteSourceWhenDestroyed),
    startPositionInSourceStream (start),
    lengthOfSourceStream (length)
{
    SubregionStream::setPosition (0);
}

SubregionStream::~SubregionStream()
{
}

int64 SubregionStream::getTotalLength()
{
    const int64 srcLen = source->getTotalLength() - startPositionInSourceStream;

    return lengthOfSourceStream >= 0 ? jmin (lengthOfSourceStream, srcLen)
                                     : srcLen;
}

int64 SubregionStream::getPosition()
{
    return source->getPosition() - startPositionInSourceStream;
}

bool SubregionStream::setPosition (int64 newPosition)
{
    return source->setPosition (jmax ((int64) 0, newPosition + startPositionInSourceStream));
}

int SubregionStream::read (void* destBuffer, int maxBytesToRead)
{
    jassert (destBuffer != nullptr && maxBytesToRead >= 0);

    if (lengthOfSourceStream < 0)
        return source->read (destBuffer, maxBytesToRead);

    maxBytesToRead = (int) jmin ((int64) maxBytesToRead, lengthOfSourceStream - getPosition());

    if (maxBytesToRead <= 0)
        return 0;

    return source->read (destBuffer, maxBytesToRead);
}

bool SubregionStream::isExhausted()
{
    if (lengthOfSourceStream >= 0 && getPosition() >= lengthOfSourceStream)
        return true;

    return source->isExhausted();
}
