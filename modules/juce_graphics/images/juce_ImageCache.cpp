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

struct ImageCache::Pimpl     : private Timer,
                               private DeletedAtShutdown
{
    Pimpl() = default;

    ~Pimpl() override
    {
        stopTimer();
        clearSingletonInstance();
    }

    JUCE_DECLARE_SINGLETON (ImageCache::Pimpl, false)

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
            auto& item = images.getReference (i);

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
            if (images.getReference (i).image.getReferenceCount() <= 1)
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

JUCE_IMPLEMENT_SINGLETON (ImageCache::Pimpl)


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
