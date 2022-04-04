/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An instance of this class is used to manage multiple AudioThumbnail objects.

    The cache runs a single background thread that is shared by all the thumbnails
    that need it, and it maintains a set of low-res previews in memory, to avoid
    having to re-scan audio files too often.

    @see AudioThumbnail

    @tags{Audio}
*/
class JUCE_API  AudioThumbnailCache
{
public:
    //==============================================================================
    /** Creates a cache object.

        The maxNumThumbsToStore parameter lets you specify how many previews should
        be kept in memory at once.
    */
    explicit AudioThumbnailCache (int maxNumThumbsToStore);

    /** Destructor. */
    virtual ~AudioThumbnailCache();

    //==============================================================================
    /** Clears out any stored thumbnails. */
    void clear();

    /** Reloads the specified thumb if this cache contains the appropriate stored
        data.

        This is called automatically by the AudioThumbnail class, so you shouldn't
        normally need to call it directly.
    */
    bool loadThumb (AudioThumbnailBase& thumb, int64 hashCode);

    /** Stores the cacheable data from the specified thumb in this cache.

        This is called automatically by the AudioThumbnail class, so you shouldn't
        normally need to call it directly.
    */
    void storeThumb (const AudioThumbnailBase& thumb, int64 hashCode);

    /** Tells the cache to forget about the thumb with the given hashcode. */
    void removeThumb (int64 hashCode);

    //==============================================================================
    /** Attempts to re-load a saved cache of thumbnails from a stream.
        The cache data must have been written by the writeToStream() method.
        This will replace all currently-loaded thumbnails with the new data.
    */
    bool readFromStream (InputStream& source);

    /** Writes all currently-loaded cache data to a stream.
        The resulting data can be re-loaded with readFromStream().
    */
    void writeToStream (OutputStream& stream);

    /** Returns the thread that client thumbnails can use. */
    TimeSliceThread& getTimeSliceThread() noexcept      { return thread; }

protected:
    /** This can be overridden to provide a custom callback for saving thumbnails
        once they have finished being loaded.
    */
    virtual void saveNewlyFinishedThumbnail (const AudioThumbnailBase&, int64 hashCode);

    /** This can be overridden to provide a custom callback for loading thumbnails
        from pre-saved files to save the cache the trouble of having to create them.
    */
    virtual bool loadNewThumb (AudioThumbnailBase&, int64 hashCode);

private:
    //==============================================================================
    TimeSliceThread thread;

    class ThumbnailCacheEntry;
    OwnedArray<ThumbnailCacheEntry> thumbs;
    CriticalSection lock;
    int maxNumThumbsToStore;

    ThumbnailCacheEntry* findThumbFor (int64 hash) const;
    int findOldestThumb() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioThumbnailCache)
};

} // namespace juce
