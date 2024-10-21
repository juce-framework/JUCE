/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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
    : thread (SystemStats::getJUCEVersion() + ": thumb cache"),
      maxNumThumbsToStore (maxNumThumbs)
{
    jassert (maxNumThumbsToStore > 0);
    thread.startThread (Thread::Priority::low);
}

AudioThumbnailCache::~AudioThumbnailCache()
{
}

AudioThumbnailCache::ThumbnailCacheEntry* AudioThumbnailCache::findThumbFor (const int64 hash) const
{
    for (int i = thumbs.size(); --i >= 0;)
        if (thumbs.getUnchecked (i)->hash == hash)
            return thumbs.getUnchecked (i);

    return nullptr;
}

int AudioThumbnailCache::findOldestThumb() const
{
    int oldest = 0;
    uint32 oldestTime = Time::getMillisecondCounter() + 1;

    for (int i = thumbs.size(); --i >= 0;)
    {
        const ThumbnailCacheEntry* const te = thumbs.getUnchecked (i);

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
        if (thumbs.getUnchecked (i)->hash == hashCode)
            thumbs.remove (i);
}

static int getThumbnailCacheFileMagicHeader() noexcept
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
        thumbs.getUnchecked (i)->write (out);
}

void AudioThumbnailCache::saveNewlyFinishedThumbnail (const AudioThumbnailBase&, int64)
{
}

bool AudioThumbnailCache::loadNewThumb (AudioThumbnailBase&, int64)
{
    return false;
}

} // namespace juce
