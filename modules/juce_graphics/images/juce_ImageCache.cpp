/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

} // namespace juce
