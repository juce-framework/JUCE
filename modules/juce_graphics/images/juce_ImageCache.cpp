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

struct ImageCache::Pimpl     : private Timer,
                               private DeletedAtShutdown
{
    Pimpl() {}
    ~Pimpl() { clearSingletonInstance(); }

    juce_DeclareSingleton_SingleThreaded_Minimal (ImageCache::Pimpl)

    Image getFromHashCode (const int64 hashCode) noexcept
    {
        const ScopedLock sl (lock);

        for (auto& item : images)
        {
            if (item.hashCode == hashCode)
            {
                item.lastUseTime = Time::getApproximateMillisecondCounter();
                return item.image;
            }
        }

        return {};
     }

    void addImageToCache (const Image& image, const int64 hashCode)
    {
        if (image.isValid())
        {
            if (! isTimerRunning())
                startTimer (2000);

            const ScopedLock sl (lock);
            images.add ({ image, hashCode, Time::getApproximateMillisecondCounter() });
        }
    }

    void timerCallback() override
    {
        auto now = Time::getApproximateMillisecondCounter();

        const ScopedLock sl (lock);

        for (int i = images.size(); --i >= 0;)
        {
            auto& item = images.getReference(i);

            if (item.image.getReferenceCount() <= 1)
            {
                if (now > item.lastUseTime + cacheTimeout || now < item.lastUseTime - 1000)
                    images.remove (i);
            }
            else
            {
                item.lastUseTime = now; // multiply-referenced, so this image is still in use.
            }
        }

        if (images.isEmpty())
            stopTimer();
    }

    void releaseUnusedImages()
    {
        const ScopedLock sl (lock);

        for (int i = images.size(); --i >= 0;)
            if (images.getReference(i).image.getReferenceCount() <= 1)
                images.remove (i);
    }

    struct Item
    {
        Image image;
        int64 hashCode;
        uint32 lastUseTime;
    };

    Array<Item> images;
    CriticalSection lock;
    unsigned int cacheTimeout = 5000;

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

juce_ImplementSingleton_SingleThreaded (ImageCache::Pimpl)


//==============================================================================
Image ImageCache::getFromHashCode (const int64 hashCode)
{
    if (Pimpl::getInstanceWithoutCreating() != nullptr)
        return Pimpl::getInstanceWithoutCreating()->getFromHashCode (hashCode);

    return {};
}

void ImageCache::addImageToCache (const Image& image, const int64 hashCode)
{
    Pimpl::getInstance()->addImageToCache (image, hashCode);
}

Image ImageCache::getFromFile (const File& file)
{
    auto hashCode = file.hashCode64();
    auto image = getFromHashCode (hashCode);

    if (image.isNull())
    {
        image = ImageFileFormat::loadFrom (file);
        addImageToCache (image, hashCode);
    }

    return image;
}

Image ImageCache::getFromMemory (const void* imageData, const int dataSize)
{
    auto hashCode = (int64) (pointer_sized_int) imageData;
    auto image = getFromHashCode (hashCode);

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
