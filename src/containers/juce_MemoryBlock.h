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

#ifndef __JUCE_MEMORYBLOCK_JUCEHEADER__
#define __JUCE_MEMORYBLOCK_JUCEHEADER__

#include "../text/juce_String.h"
#include "juce_HeapBlock.h"


//==============================================================================
/**
    A class to hold a resizable block of raw data.

*/
class JUCE_API  MemoryBlock
{
public:
    //==============================================================================
    /** Create an uninitialised block with 0 size. */
    MemoryBlock() throw();

    /** Creates a memory block with a given initial size.

        @param initialSize          the size of block to create
        @param initialiseToZero     whether to clear the memory or just leave it uninitialised
    */
    MemoryBlock (const size_t initialSize,
                 const bool initialiseToZero = false) throw();

    /** Creates a copy of another memory block. */
    MemoryBlock (const MemoryBlock& other) throw();

    /** Creates a memory block using a copy of a block of data.

        @param dataToInitialiseFrom     some data to copy into this block
        @param sizeInBytes              how much space to use
    */
    MemoryBlock (const void* const dataToInitialiseFrom,
                 const size_t sizeInBytes) throw();

    /** Destructor. */
    ~MemoryBlock() throw();

    /** Copies another memory block onto this one.

        This block will be resized and copied to exactly match the other one.
    */
    const MemoryBlock& operator= (const MemoryBlock& other) throw();

    //==============================================================================
    /** Compares two memory blocks.

        @returns true only if the two blocks are the same size and have identical contents.
    */
    bool operator== (const MemoryBlock& other) const throw();

    /** Compares two memory blocks.

        @returns true if the two blocks are different sizes or have different contents.
    */
    bool operator!= (const MemoryBlock& other) const throw();

    //==============================================================================
    /** Returns a pointer to the data, casting it to any type of primitive data required.

        Note that the pointer returned will probably become invalid when the
        block is resized.
    */
    template <class DataType>
    operator DataType*() const throw()                              { return (DataType*) data; }

    /** Returns a void pointer to the data.

        Note that the pointer returned will probably become invalid when the
        block is resized.
    */
    void* getData() const throw()                                   { return data; }

    /** Returns a byte from the memory block.

        This returns a reference, so you can also use it to set a byte.
    */
    template <typename Type>
    char& operator[] (const Type offset) const throw()              { return data [offset]; }


    //==============================================================================
    /** Returns the block's current allocated size, in bytes. */
    size_t getSize() const throw()                                  { return size; }

    /** Resizes the memory block.

        This will try to keep as much of the block's current content as it can,
        and can optionally be made to clear any new space that gets allocated at
        the end of the block.

        @param newSize                      the new desired size for the block
        @param initialiseNewSpaceToZero     if the block gets enlarged, this determines
                                            whether to clear the new section or just leave it
                                            uninitialised
        @see ensureSize
    */
    void setSize (const size_t newSize,
                  const bool initialiseNewSpaceToZero = false) throw();

    /** Increases the block's size only if it's smaller than a given size.

        @param minimumSize                  if the block is already bigger than this size, no action
                                            will be taken; otherwise it will be increased to this size
        @param initialiseNewSpaceToZero     if the block gets enlarged, this determines
                                            whether to clear the new section or just leave it
                                            uninitialised
        @see setSize
    */
    void ensureSize (const size_t minimumSize,
                     const bool initialiseNewSpaceToZero = false) throw();

    //==============================================================================
    /** Fills the entire memory block with a repeated byte value.

        This is handy for clearing a block of memory to zero.
    */
    void fillWith (const uint8 valueToUse) throw();

    /** Adds another block of data to the end of this one.

        This block's size will be increased accordingly.
    */
    void append (const void* const data,
                 const size_t numBytes) throw();

    /** Exchanges the contents of this and another memory block.
        No actual copying is required for this, so it's very fast.
    */
    void swapWith (MemoryBlock& other) throw();

    //==============================================================================
    /** Copies data into this MemoryBlock from a memory address.

        @param srcData              the memory location of the data to copy into this block
        @param destinationOffset    the offset in this block at which the data being copied should begin
        @param numBytes             how much to copy in (if this goes beyond the size of the memory block,
                                    it will be clipped so not to do anything nasty)
    */
    void copyFrom (const void* srcData,
                   int destinationOffset,
                   size_t numBytes) throw();

    /** Copies data from this MemoryBlock to a memory address.

        @param destData         the memory location to write to
        @param sourceOffset     the offset within this block from which the copied data will be read
        @param numBytes         how much to copy (if this extends beyond the limits of the memory block,
                                zeros will be used for that portion of the data)
    */
    void copyTo (void* destData,
                 int sourceOffset,
                 size_t numBytes) const throw();

    /** Chops out a section  of the block.

        This will remove a section of the memory block and close the gap around it,
        shifting any subsequent data downwards and reducing the size of the block.

        If the range specified goes beyond the size of the block, it will be clipped.
    */
    void removeSection (size_t startByte, size_t numBytesToRemove) throw();

    //==============================================================================
    /** Attempts to parse the contents of the block as a zero-terminated string of 8-bit
        characters in the system's default encoding. */
    const String toString() const throw();

    //==============================================================================
    /** Parses a string of hexadecimal numbers and writes this data into the memory block.

        The block will be resized to the number of valid bytes read from the string.
        Non-hex characters in the string will be ignored.

        @see String::toHexString()
    */
    void loadFromHexString (const String& sourceHexString) throw();

    //==============================================================================
    /** Sets a number of bits in the memory block, treating it as a long binary sequence. */
    void setBitRange (size_t bitRangeStart,
                      size_t numBits,
                      int binaryNumberToApply) throw();

    /** Reads a number of bits from the memory block, treating it as one long binary sequence */
    int getBitRange (size_t bitRangeStart,
                     size_t numBitsToRead) const throw();

    //==============================================================================
    /** Returns a string of characters that represent the binary contents of this block.

        Uses a 64-bit encoding system to allow binary data to be turned into a string
        of simple non-extended characters, e.g. for storage in XML.

        @see fromBase64Encoding
    */
    const String toBase64Encoding() const throw();

    /** Takes a string of encoded characters and turns it into binary data.

        The string passed in must have been created by to64BitEncoding(), and this
        block will be resized to recreate the original data block.

        @see toBase64Encoding
    */
    bool fromBase64Encoding  (const String& encodedString) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    HeapBlock <char> data;
    size_t size;
};


#endif   // __JUCE_MEMORYBLOCK_JUCEHEADER__
