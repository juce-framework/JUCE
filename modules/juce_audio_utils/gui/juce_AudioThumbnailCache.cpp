/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

class AudioThumbnailCache::ThumbnailCacheEntry
{
public:
    ThumbnailCacheEntry (const int64 hashCode)
        : hash (hashCode),
          lastUsed (Time::getMillisecondCounter())
    {
    }

    ThumbnailCacheEntry (InputStream& in)
        : hash (in.readInt64()),
          lastUsed (0)
    {
        const int64 len = in.readInt64();
        in.readIntoMemoryBlock (data, (ssize_t) len);
    }

    void write (OutputStream& out)
    {
        out.writeInt64 (hash);
        out.writeInt64 ((int64) data.getSize());
        out << data;
    }

    int64 hash;
    uint32 lastUsed;
    MemoryBlock data;

private:
    JUCE_LEAK_DETECTOR (ThumbnailCacheEntry)
};

//==============================================================================
AudioThumbnailCache::AudioThumbnailCache (const int maxNumThumbs)
    : thread ("thumb cache"),
      maxNumThumbsToStore (maxNumThumbs)
{
    jassert (maxNumThumbsToStore > 0);
    thread.startThread (2);
}

AudioThumbnailCache::~AudioThumbnailCache()
{
}

AudioThumbnailCache::ThumbnailCacheEntry* AudioThumbnailCache::findThumbFor (const int64 hash) const
{
    for (int i = thumbs.size(); --i >= 0;)
        if (thumbs.getUnchecked(i)->hash == hash)
            return thumbs.getUnchecked(i);

    return nullptr;
}

int AudioThumbnailCache::findOldestThumb() const
{
    int oldest = 0;
    uint32 oldestTime = Time::getMillisecondCounter() + 1;

    for (int i = thumbs.size(); --i >= 0;)
    {
        const ThumbnailCacheEntry* const te = thumbs.getUnchecked(i);

        if (te->lastUsed < oldestTime)
        {
            oldest = i;
            oldestTime = te->lastUsed;
        }
    }

    return oldest;
}

bool AudioThumbnailCache::loadThumb (AudioThumbnailBase& thumb, const int64 hashCode)
{
    const ScopedLock sl (lock);

    if (ThumbnailCacheEntry* te = findThumbFor (hashCode))
    {
        te->lastUsed = Time::getMillisecondCounter();

        MemoryInputStream in (te->data, false);
        thumb.loadFrom (in);
        return true;
    }

    return loadNewThumb (thumb, hashCode);
}

void AudioThumbnailCache::storeThumb (const AudioThumbnailBase& thumb,
                                      const int64 hashCode)
{
    const ScopedLock sl (lock);
    ThumbnailCacheEntry* te = findThumbFor (hashCode);

    if (te == nullptr)
    {
        te = new ThumbnailCacheEntry (hashCode);

        if (thumbs.size() < maxNumThumbsToStore)
            thumbs.add (te);
        else
            thumbs.set (findOldestThumb(), te);
    }

    {
        MemoryOutputStream out (te->data, false);
        thumb.saveTo (out);
    }

    saveNewlyFinishedThumbnail (thumb, hashCode);
}

void AudioThumbnailCache::clear()
{
    const ScopedLock sl (lock);
    thumbs.clear();
}

void AudioThumbnailCache::removeThumb (const int64 hashCode)
{
    const ScopedLock sl (lock);

    for (int i = thumbs.size(); --i >= 0;)
        if (thumbs.getUnchecked(i)->hash == hashCode)
            thumbs.remove (i);
}

static inline int getThumbnailCacheFileMagicHeader() noexcept
{
    return (int) ByteOrder::littleEndianInt ("ThmC");
}

bool AudioThumbnailCache::readFromStream (InputStream& source)
{
    if (source.readInt() != getThumbnailCacheFileMagicHeader())
        return false;

    const ScopedLock sl (lock);
    clear();
    int numThumbnails = jmin (maxNumThumbsToStore, source.readInt());

    while (--numThumbnails >= 0 && ! source.isExhausted())
        thumbs.add (new ThumbnailCacheEntry (source));

    return true;
}

void AudioThumbnailCache::writeToStream (OutputStream& out)
{
    const ScopedLock sl (lock);

    out.writeInt (getThumbnailCacheFileMagicHeader());
    out.writeInt (thumbs.size());

    for (int i = 0; i < thumbs.size(); ++i)
        thumbs.getUnchecked(i)->write (out);
}

void AudioThumbnailCache::saveNewlyFinishedThumbnail (const AudioThumbnailBase&, int64)
{
}

bool AudioThumbnailCache::loadNewThumb (AudioThumbnailBase&, int64)
{
    return false;
}
