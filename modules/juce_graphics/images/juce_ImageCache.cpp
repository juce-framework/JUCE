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

class ImageCache::Pimpl     : private Timer,
                              private DeletedAtShutdown
{
public:
    Pimpl()  : cacheTimeout (5000)
    {
    }

    ~Pimpl()
    {
        clearSingletonInstance();
    }

    Image getFromHashCode (const int64 hashCode)
    {
        const ScopedLock sl (lock);

        for (int i = images.size(); --i >= 0;)
        {
            const Item* const item = images.getUnchecked(i);

            if (item->hashCode == hashCode)
                return item->image;
        }

        return Image::null;
    }

    void addImageToCache (const Image& image, const int64 hashCode)
    {
        if (image.isValid())
        {
            if (! isTimerRunning())
                startTimer (2000);

            Item* const item = new Item();
            item->hashCode = hashCode;
            item->image = image;
            item->lastUseTime = Time::getApproximateMillisecondCounter();

            const ScopedLock sl (lock);
            images.add (item);
        }
    }

    void timerCallback() override
    {
        const uint32 now = Time::getApproximateMillisecondCounter();

        const ScopedLock sl (lock);

        for (int i = images.size(); --i >= 0;)
        {
            Item* const item = images.getUnchecked(i);

            if (item->image.getReferenceCount() <= 1)
            {
                if (now > item->lastUseTime + cacheTimeout || now < item->lastUseTime - 1000)
                    images.remove (i);
            }
            else
            {
                item->lastUseTime = now; // multiply-referenced, so this image is still in use.
            }
        }

        if (images.size() == 0)
            stopTimer();
    }

    void releaseUnusedImages()
    {
        const ScopedLock sl (lock);

        for (int i = images.size(); --i >= 0;)
            if (images.getUnchecked(i)->image.getReferenceCount() <= 1)
                images.remove (i);
    }

    struct Item
    {
        Image image;
        int64 hashCode;
        uint32 lastUseTime;
    };

    unsigned int cacheTimeout;

    juce_DeclareSingleton_SingleThreaded_Minimal (ImageCache::Pimpl);

private:
    OwnedArray<Item> images;
    CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

juce_ImplementSingleton_SingleThreaded (ImageCache::Pimpl);


//==============================================================================
Image ImageCache::getFromHashCode (const int64 hashCode)
{
    if (Pimpl::getInstanceWithoutCreating() != nullptr)
        return Pimpl::getInstanceWithoutCreating()->getFromHashCode (hashCode);

    return Image::null;
}

void ImageCache::addImageToCache (const Image& image, const int64 hashCode)
{
    Pimpl::getInstance()->addImageToCache (image, hashCode);
}

Image ImageCache::getFromFile (const File& file)
{
    const int64 hashCode = file.hashCode64();
    Image image (getFromHashCode (hashCode));

    if (image.isNull())
    {
        image = ImageFileFormat::loadFrom (file);
        addImageToCache (image, hashCode);
    }

    return image;
}

Image ImageCache::getFromMemory (const void* imageData, const int dataSize)
{
    const int64 hashCode = (int64) (pointer_sized_int) imageData;
    Image image (getFromHashCode (hashCode));

    if (image.isNull())
    {
        image = ImageFileFormat::loadFrom (imageData, (size_t) dataSize);
        addImageToCache (image, hashCode);
    }

    return image;
}

void ImageCache::setCacheTimeout (const int millisecs)
{
    jassert (millisecs >= 0);
    Pimpl::getInstance()->cacheTimeout = (unsigned int) millisecs;
}

void ImageCache::releaseUnusedImages()
{
    Pimpl::getInstance()->releaseUnusedImages();
}
