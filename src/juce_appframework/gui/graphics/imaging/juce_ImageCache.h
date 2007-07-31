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

#ifndef __JUCE_IMAGECACHE_JUCEHEADER__
#define __JUCE_IMAGECACHE_JUCEHEADER__

#include "juce_Image.h"
#include "../../../../juce_core/io/files/juce_File.h"
#include "../../../events/juce_Timer.h"
#include "../../../application/juce_DeletedAtShutdown.h"


//==============================================================================
/**
    A global cache of images that have been loaded from files or memory.

    If you're loading an image and may need to use the image in more than one
    place, this is used to allow the same image to be shared rather than loading
    multiple copies into memory.

    Another advantage is that after images are released, they will be kept in
    memory for a few seconds before it is actually deleted, so if you're repeatedly
    loading/deleting the same image, it'll reduce the chances of having to reload it
    each time.

    @see Image, ImageFileFormat
*/
class JUCE_API  ImageCache  : private DeletedAtShutdown,
                              private Timer
{
public:
    //==============================================================================
    /** Loads an image from a file, (or just returns the image if it's already cached).

        If the cache already contains an image that was loaded from this file,
        that image will be returned. Otherwise, this method will try to load the
        file, add it to the cache, and return it.

        It's very important not to delete the image that is returned - instead use
        the ImageCache::release() method.

        Also, remember that the image returned is shared, so drawing into it might
        affect other things that are using it!

        @param file     the file to try to load
        @returns        the image, or null if it there was an error loading it
        @see release, getFromMemory, getFromCache, ImageFileFormat::loadFrom
    */
    static Image* getFromFile (const File& file);

    /** Loads an image from an in-memory image file, (or just returns the image if it's already cached).

        If the cache already contains an image that was loaded from this block of memory,
        that image will be returned. Otherwise, this method will try to load the
        file, add it to the cache, and return it.

        It's very important not to delete the image that is returned - instead use
        the ImageCache::release() method.

        Also, remember that the image returned is shared, so drawing into it might
        affect other things that are using it!

        @param imageData    the block of memory containing the image data
        @param dataSize     the data size in bytes
        @returns            the image, or null if it there was an error loading it
        @see release, getFromMemory, getFromCache, ImageFileFormat::loadFrom
    */
    static Image* getFromMemory (const void* imageData,
                                 const int dataSize);

    /** Releases an image that was previously created by the ImageCache.

        If an image has been returned by the getFromFile() or getFromMemory() methods,
        it mustn't be deleted directly, but should be released with this method
        instead.

        @see getFromFile, getFromMemory
    */
    static void release (Image* const imageToRelease);

    /** Checks whether an image is in the cache or not.

        @returns true if the image is currently in the cache
    */
    static bool isImageInCache (Image* const imageToLookFor);

    /** Increments the reference-count for a cached image.

        If the image isn't in the cache, this method won't do anything.
    */
    static void incReferenceCount (Image* const image);

    //==============================================================================
    /** Checks the cache for an image with a particular hashcode.

        If there's an image in the cache with this hashcode, it will be returned,
        otherwise it will return zero.

        If an image is returned, it must be released with the release() method
        when no longer needed, to maintain the correct reference counts.

        @param hashCode the hash code that would have been associated with the
                        image by addImageToCache()
        @see addImageToCache
    */
    static Image* getFromHashCode (const int64 hashCode);

    /** Adds an image to the cache with a user-defined hash-code.

        After calling this, responsibilty for deleting the image will be taken
        by the ImageCache.

        The image will be initially be given a reference count of 1, so call
        the release() method to delete it.

        @param image    the image to add
        @param hashCode the hash-code to associate with it
        @see getFromHashCode
    */
    static void addImageToCache (Image* const image,
                                 const int64 hashCode);

    /** Changes the amount of time before an unused image will be removed from the cache.

        By default this is about 5 seconds.
    */
    static void setCacheTimeout (const int millisecs);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    CriticalSection lock;
    VoidArray images;

    ImageCache() throw();
    ImageCache (const ImageCache&);
    const ImageCache& operator= (const ImageCache&);
    ~ImageCache();

    void timerCallback();
};

#endif   // __JUCE_IMAGECACHE_JUCEHEADER__
