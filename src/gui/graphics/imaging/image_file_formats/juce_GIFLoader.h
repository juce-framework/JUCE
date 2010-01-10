/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_GIFLOADER_JUCEHEADER__
#define __JUCE_GIFLOADER_JUCEHEADER__

#ifndef DOXYGEN

#include "../../../../io/streams/juce_InputStream.h"
#include "../juce_Image.h"


//==============================================================================
/**
    Used internally by ImageFileFormat - don't use this class directly in your
    application.

    @see ImageFileFormat
*/
class GIFLoader
{
public:
    GIFLoader (InputStream& in);
    ~GIFLoader();

    Image* getImage() const             { return image; }

private:
    Image* image;
    InputStream& input;
    uint8 buffer [300];
    uint8 palette [256][4];
    bool dataBlockIsZero, fresh, finished;
    int currentBit, lastBit, lastByteIndex;
    int codeSize, setCodeSize;
    int maxCode, maxCodeSize;
    int firstcode, oldcode;
    int clearCode, end_code;
    enum { maxGifCode = 1 << 12 };
    int table [2] [maxGifCode];
    int stack [2 * maxGifCode];
    int *sp;

    bool getSizeFromHeader (int& width, int& height);
    bool readPalette (const int numCols);
    int readDataBlock (unsigned char* dest);
    int processExtension (int type, int& transparent);
    int readLZWByte (bool initialise, int input_code_size);
    int getCode (int code_size, bool initialise);
    bool readImage (int width, int height, int interlace, int transparent);
    static inline int makeWord (const uint8 a, const uint8 b)    { return (b << 8) | a; }

    GIFLoader (const GIFLoader&);
    const GIFLoader& operator= (const GIFLoader&);
};

#endif   // DOXYGEN

#endif   // __JUCE_GIFLOADER_JUCEHEADER__
