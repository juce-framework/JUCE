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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ImageCache.h"
#include "juce_ImageFileFormat.h"
#include "../../../../juce_core/threads/juce_ScopedLock.h"


//==============================================================================
struct CachedImageInfo
{
    Image* image;
    int64 hashCode;
    int refCount;
    unsigned int releaseTime;

    juce_UseDebuggingNewOperator
};

//==============================================================================
static ImageCache* instance = 0;
static int cacheTimeout = 5000;


ImageCache::ImageCache() throw()
    : images (4)
{
}

ImageCache::~ImageCache()
{
    const ScopedLock sl (lock);

    for (int i = images.size(); --i >= 0;)
    {
        CachedImageInfo* const ci = (CachedImageInfo*)(images.getUnchecked(i));
        delete ci->image;
        delete ci;
    }

    images.clear();

    jassert (instance == this);
    instance = 0;
}

Image* ImageCache::getFromHashCode (const int64 hashCode)
{
    if (instance != 0)
    {
        const ScopedLock sl (instance->lock);

        for (int i = instance->images.size(); --i >= 0;)
        {
            CachedImageInfo* const ci = (CachedImageInfo*) instance->images.getUnchecked(i);

            if (ci->hashCode == hashCode)
            {
                atomicIncrement (ci->refCount);
                return ci->image;
            }
        }
    }

    return 0;
}

void ImageCache::addImageToCache (Image* const image,
                                  const int64 hashCode)
{
    if (image != 0)
    {
        if (instance == 0)
            instance = new ImageCache();

        CachedImageInfo* const newC = new CachedImageInfo();
        newC->hashCode = hashCode;
        newC->image = image;
        newC->refCount = 1;
        newC->releaseTime = 0;

        const ScopedLock sl (instance->lock);
        instance->images.add (newC);
    }
}

void ImageCache::release (Image* const imageToRelease)
{
    if (imageToRelease != 0 && instance != 0)
    {
        const ScopedLock sl (instance->lock);

        for (int i = instance->images.size(); --i >= 0;)
        {
            CachedImageInfo* const ci = (CachedImageInfo*) instance->images.getUnchecked(i);

            if (ci->image == imageToRelease)
            {
                if (--(ci->refCount) == 0)
                    ci->releaseTime = Time::getApproximateMillisecondCounter();

                if (! instance->isTimerRunning())
                    instance->startTimer (999);

                break;
            }
        }
    }
}

bool ImageCache::isImageInCache (Image* const imageToLookFor)
{
    if (instance != 0)
    {
        const ScopedLock sl (instance->lock);

        for (int i = instance->images.size(); --i >= 0;)
            if (((const CachedImageInfo*) instance->images.getUnchecked(i))->image == imageToLookFor)
                return true;
    }

    return false;
}

void ImageCache::incReferenceCount (Image* const image)
{
    if (instance != 0)
    {
        const ScopedLock sl (instance->lock);

        for (int i = instance->images.size(); --i >= 0;)
        {
            CachedImageInfo* const ci = (CachedImageInfo*) instance->images.getUnchecked(i);

            if (ci->image == image)
            {
                ci->refCount++;
                return;
            }
        }
    }

    jassertfalse  // (trying to inc the ref count of an image that's not in the cache)
}

void ImageCache::timerCallback()
{
    int numberStillNeedingReleasing = 0;
    const unsigned int now = Time::getApproximateMillisecondCounter();

    const ScopedLock sl (lock);

    for (int i = images.size(); --i >= 0;)
    {
        CachedImageInfo* const ci = (CachedImageInfo*) images.getUnchecked(i);

        if (ci->refCount <= 0)
        {
            if (now > ci->releaseTime + cacheTimeout
                 || now < ci->releaseTime - 1000)
            {
                images.remove (i);
                delete ci->image;
                delete ci;
            }
            else
            {
                ++numberStillNeedingReleasing;
            }
        }
    }

    if (numberStillNeedingReleasing == 0)
        stopTimer();
}

Image* ImageCache::getFromFile (const File& file)
{
    const int64 hashCode = file.getFullPathName().hashCode64();

    Image* image = getFromHashCode (hashCode);

    if (image == 0)
    {
        image = ImageFileFormat::loadFrom (file);
        addImageToCache (image, hashCode);
    }

    return image;
}

Image* ImageCache::getFromMemory (const void* imageData,
                                  const int dataSize)
{
    const int64 hashCode = (int64) (pointer_sized_int) imageData;

    Image* image = getFromHashCode (hashCode);

    if (image == 0)
    {
        image = ImageFileFormat::loadFrom (imageData, dataSize);
        addImageToCache (image, hashCode);
    }

    return image;
}

void ImageCache::setCacheTimeout (const int millisecs)
{
    cacheTimeout = millisecs;
}


END_JUCE_NAMESPACE
