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

#ifndef __JUCE_GIFLOADER_JUCEHEADER__
#define __JUCE_GIFLOADER_JUCEHEADER__

#ifndef DOXYGEN

#include "../../../../../juce_core/io/juce_InputStream.h"
#include "../juce_Image.h"


//==============================================================================
static const int maxGifCode = 1 << 12;

/**
    Used internally by ImageFileFormat - don't use this class directly in your
    application.

    @see ImageFileFormat
*/
class GIFLoader
{
public:
    GIFLoader (InputStream& in);
    ~GIFLoader() throw();

    Image* getImage() const throw()         { return image; }

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
    int table [2] [maxGifCode];
    int stack [2 * maxGifCode];
    int *sp;

    bool getSizeFromHeader (int& width, int& height);
    bool readPalette (const int numCols);
    int readDataBlock (unsigned char* dest);
    int processExtension (int type, int& transparent);
    int readLZWByte (bool initialise, int input_code_size);
    int getCode (int code_size, bool initialise);
    bool readImage (int width, int height,
                    int interlace, int transparent);

    GIFLoader (const GIFLoader&);
    const GIFLoader& operator= (const GIFLoader&);
};

#endif   // DOXYGEN

#endif   // __JUCE_GIFLOADER_JUCEHEADER__
