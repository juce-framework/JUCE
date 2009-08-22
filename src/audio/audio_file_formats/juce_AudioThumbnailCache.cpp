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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioThumbnailCache.h"
#include "../../io/streams/juce_MemoryInputStream.h"
#include "../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
struct ThumbnailCacheEntry
{
    int64 hash;
    uint32 lastUsed;
    MemoryBlock data;

    juce_UseDebuggingNewOperator
};

//==============================================================================
AudioThumbnailCache::AudioThumbnailCache (const int maxNumThumbsToStore_)
    : TimeSliceThread (T("thumb cache")),
      maxNumThumbsToStore (maxNumThumbsToStore_)
{
    startThread (2);
}

AudioThumbnailCache::~AudioThumbnailCache()
{
}

bool AudioThumbnailCache::loadThumb (AudioThumbnail& thumb, const int64 hashCode)
{
    for (int i = thumbs.size(); --i >= 0;)
    {
        if (thumbs[i]->hash == hashCode)
        {
            MemoryInputStream in ((const char*) thumbs[i]->data.getData(),
                                  thumbs[i]->data.getSize(),
                                  false);

            thumb.loadFrom (in);

            thumbs[i]->lastUsed = Time::getMillisecondCounter();
            return true;
        }
    }

    return false;
}

void AudioThumbnailCache::storeThumb (const AudioThumbnail& thumb,
                                      const int64 hashCode)
{
    MemoryOutputStream out;
    thumb.saveTo (out);

    ThumbnailCacheEntry* te = 0;

    for (int i = thumbs.size(); --i >= 0;)
    {
        if (thumbs[i]->hash == hashCode)
        {
            te = thumbs[i];
            break;
        }
    }

    if (te == 0)
    {
        te = new ThumbnailCacheEntry();
        te->hash = hashCode;

        if (thumbs.size() < maxNumThumbsToStore)
        {
            thumbs.add (te);
        }
        else
        {
            int oldest = 0;
            unsigned int oldestTime = Time::getMillisecondCounter() + 1;

            int i;
            for (i = thumbs.size(); --i >= 0;)
                if (thumbs[i]->lastUsed < oldestTime)
                    oldest = i;

            thumbs.set (i, te);
        }
    }

    te->lastUsed = Time::getMillisecondCounter();
    te->data.setSize (0);
    te->data.append (out.getData(), out.getDataSize());
}

void AudioThumbnailCache::clear()
{
    thumbs.clear();
}

//==============================================================================
void AudioThumbnailCache::addThumbnail (AudioThumbnail* const thumb)
{
    addTimeSliceClient (thumb);
}

void AudioThumbnailCache::removeThumbnail (AudioThumbnail* const thumb)
{
    removeTimeSliceClient (thumb);
}


END_JUCE_NAMESPACE
