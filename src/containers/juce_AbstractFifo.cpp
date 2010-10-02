/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AbstractFifo.h"


//==============================================================================
AbstractFifo::AbstractFifo (const int capacity) throw()
    : bufferSize (capacity)
{
    jassert (bufferSize > 0);
}

AbstractFifo::~AbstractFifo() {}

int AbstractFifo::getTotalSize() const throw()            { return bufferSize; }
int AbstractFifo::getFreeSpace() const throw()            { return bufferSize - getNumReady(); }
int AbstractFifo::getNumReady() const throw()             { return validEnd.get() - validStart.get(); }

void AbstractFifo::reset() throw()
{
    validEnd = 0;
    validStart = 0;
}

void AbstractFifo::setTotalSize (int newSize) throw()
{
    jassert (newSize > 0);
    reset();
    bufferSize = newSize;
}

//==============================================================================
void AbstractFifo::prepareToWrite (int numToWrite, int& startIndex1, int& blockSize1, int& startIndex2, int& blockSize2) const throw()
{
    const int vs = validStart.get();
    const int ve = validEnd.get();

    const int freeSpace = bufferSize - (ve - vs);
    numToWrite = jmin (numToWrite, freeSpace);

    if (numToWrite <= 0)
    {
        startIndex1 = 0;
        startIndex2 = 0;
        blockSize1 = 0;
        blockSize2 = 0;
    }
    else
    {
        startIndex1 = (int) (ve % bufferSize);
        startIndex2 = 0;
        blockSize1 = jmin (bufferSize - startIndex1, numToWrite);
        numToWrite -= blockSize1;
        blockSize2 = numToWrite <= 0 ? 0 : jmin (numToWrite, (int) (vs % bufferSize));
    }
}

void AbstractFifo::finishedWrite (int numWritten) throw()
{
    jassert (numWritten >= 0 && numWritten < bufferSize);
    validEnd += numWritten;
}

void AbstractFifo::prepareToRead (int numWanted, int& startIndex1, int& blockSize1, int& startIndex2, int& blockSize2) const throw()
{
    const int vs = validStart.get();
    const int ve = validEnd.get();

    const int numReady = ve - vs;
    numWanted = jmin (numWanted, numReady);

    if (numWanted <= 0)
    {
        startIndex1 = 0;
        startIndex2 = 0;
        blockSize1 = 0;
        blockSize2 = 0;
    }
    else
    {
        startIndex1 = (int) (vs % bufferSize);
        startIndex2 = 0;
        blockSize1 = jmin (bufferSize - startIndex1, numWanted);
        numWanted -= blockSize1;
        blockSize2 = numWanted <= 0 ? 0 : jmin (numWanted, (int) (ve % bufferSize));
    }
}

void AbstractFifo::finishedRead (int numRead) throw()
{
    jassert (numRead >= 0 && numRead < bufferSize);
    validStart += numRead;
}


END_JUCE_NAMESPACE
