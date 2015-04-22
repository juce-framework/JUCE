/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_IMAGECACHE_H_INCLUDED
#define JUCE_IMAGECACHE_H_INCLUDED


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
class JUCE_API  ImageCache
{
public:
    //==============================================================================
    /** Loads an image from a file, (or just returns the image if it's already cached).

        If the cache already contains an image that was loaded from this file,
        that image will be returned. Otherwise, this method will try to load the
        file, add it to the cache, and return it.

        Remember that the image returned is shared, so drawing into it might
        affect other things that are using it! If you want to draw on it, first
        call Image::duplicateIfShared()

        @param file     the file to try to load
        @returns        the image, or null if it there was an error loading it
        @see getFromMemory, getFromCache, ImageFileFormat::loadFrom
    */
    static Image getFromFile (const File& file);

    /** Loads an image from an in-memory image file, (or just returns the image if it's already cached).

        If the cache already contains an image that was loaded from this block of memory,
        that image will be returned. Otherwise, this method will try to load the
        file, add it to the cache, and return it.

        Remember that the image returned is shared, so drawing into it might
        affect other things that are using it! If you want to draw on it, first
        call Image::duplicateIfShared()

        @param imageData    the block of memory containing the image data
        @param dataSize     the data size in bytes
        @returns            the image, or an invalid image if it there was an error loading it
        @see getFromMemory, getFromCache, ImageFileFormat::loadFrom
    */
    static Image getFromMemory (const void* imageData, int dataSize);

    //==============================================================================
    /** Checks the cache for an image with a particular hashcode.

        If there's an image in the cache with this hashcode, it will be returned,
        otherwise it will return an invalid image.

        @param hashCode the hash code that was associated with the image by addImageToCache()
        @see addImageToCache
    */
    static Image getFromHashCode (int64 hashCode);

    /** Adds an image to the cache with a user-defined hash-code.

        The image passed-in will be referenced (not copied) by the cache, so it's probably
        a good idea not to draw into it after adding it, otherwise this will affect all
        instances of it that may be in use.

        @param image    the image to add
        @param hashCode the hash-code to associate with it
        @see getFromHashCode
    */
    static void addImageToCache (const Image& image, int64 hashCode);

    /** Changes the amount of time before an unused image will be removed from the cache.
        By default this is about 5 seconds.
    */
    static void setCacheTimeout (int millisecs);

    /** Releases any images in the cache that aren't being referenced by active
        Image objects.
    */
    static void releaseUnusedImages();

private:
    //==============================================================================
    class Pimpl;
    friend class Pimpl;

    ImageCache();
    ~ImageCache();

    JUCE_DECLARE_NON_COPYABLE (ImageCache)
};

#endif   // JUCE_IMAGECACHE_H_INCLUDED
