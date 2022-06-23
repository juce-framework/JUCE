/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_CONTENT_SHARING
//==============================================================================
class ContentSharer::PrepareImagesThread    : private Thread
{
public:
    PrepareImagesThread (ContentSharer& cs, const Array<Image>& imagesToUse,
                         ImageFileFormat* imageFileFormatToUse)
        : Thread ("ContentSharer::PrepareImagesThread"),
          owner (cs),
          images (imagesToUse),
          imageFileFormat (imageFileFormatToUse == nullptr ? new PNGImageFormat()
                                                           : imageFileFormatToUse),
          extension (imageFileFormat->getFormatName().toLowerCase())
    {
        startThread();
    }

    ~PrepareImagesThread() override
    {
        signalThreadShouldExit();
        waitForThreadToExit (10000);
    }

private:
    void run() override
    {
        for (const auto& image : images)
        {
            if (threadShouldExit())
                return;

            File tempFile = File::createTempFile (extension);

            if (! tempFile.create().wasOk())
                break;

            std::unique_ptr<FileOutputStream> outputStream (tempFile.createOutputStream());

            if (outputStream == nullptr)
                break;

            if (imageFileFormat->writeImageToStream (image, *outputStream))
                owner.temporaryFiles.add (tempFile);
        }

        finish();
    }

    void finish()
    {
        MessageManager::callAsync ([this]() { owner.filesToSharePrepared(); });
    }

    ContentSharer& owner;
    const Array<Image> images;
    std::unique_ptr<ImageFileFormat> imageFileFormat;
    String extension;
};

//==============================================================================
class ContentSharer::PrepareDataThread    : private Thread
{
public:
    PrepareDataThread (ContentSharer& cs, const MemoryBlock& mb)
        : Thread ("ContentSharer::PrepareDataThread"),
          owner (cs),
          data (mb)
    {
        startThread();
    }

    ~PrepareDataThread() override
    {
        signalThreadShouldExit();
        waitForThreadToExit (10000);
    }

private:
    void run() override
    {
        File tempFile = File::createTempFile ("data");

        if (tempFile.create().wasOk())
        {
            if (auto outputStream = std::unique_ptr<FileOutputStream> (tempFile.createOutputStream()))
            {
                size_t pos = 0;
                size_t totalSize = data.getSize();

                while (pos < totalSize)
                {
                    if (threadShouldExit())
                        return;

                    size_t numToWrite = std::min ((size_t) 8192, totalSize - pos);

                    outputStream->write (data.begin() + pos, numToWrite);

                    pos += numToWrite;
                }

                owner.temporaryFiles.add (tempFile);
            }
        }

        finish();
    }

    void finish()
    {
        MessageManager::callAsync ([this]() { owner.filesToSharePrepared(); });
    }

    ContentSharer& owner;
    const MemoryBlock data;
};
#endif

//==============================================================================
JUCE_IMPLEMENT_SINGLETON (ContentSharer)

ContentSharer::ContentSharer() {}
ContentSharer::~ContentSharer() { clearSingletonInstance(); }

void ContentSharer::shareFiles (const Array<URL>& files,
                                std::function<void (bool, const String&)> callbackToUse)
{
  #if JUCE_CONTENT_SHARING
    startNewShare (callbackToUse);
    pimpl->shareFiles (files);
  #else
    ignoreUnused (files);

    // Content sharing is not available on this platform!
    jassertfalse;

    if (callbackToUse)
        callbackToUse (false, "Content sharing is not available on this platform!");
  #endif
}

#if JUCE_CONTENT_SHARING
void ContentSharer::startNewShare (std::function<void (bool, const String&)> callbackToUse)
{
    // You should not start another sharing operation before the previous one is finished.
    // Forcibly stopping a previous sharing operation is rarely a good idea!
    jassert (pimpl == nullptr);
    pimpl.reset();

    prepareDataThread = nullptr;
    prepareImagesThread = nullptr;

    deleteTemporaryFiles();

    // You need to pass a valid callback.
    jassert (callbackToUse);
    callback = std::move (callbackToUse);

    pimpl.reset (createPimpl());
}
#endif

void ContentSharer::shareText (const String& text,
                               std::function<void (bool, const String&)> callbackToUse)
{
  #if JUCE_CONTENT_SHARING
    startNewShare (callbackToUse);
    pimpl->shareText (text);
  #else
    ignoreUnused (text);

    // Content sharing is not available on this platform!
    jassertfalse;

    if (callbackToUse)
        callbackToUse (false, "Content sharing is not available on this platform!");
  #endif
}

void ContentSharer::shareImages (const Array<Image>& images,
                                 std::function<void (bool, const String&)> callbackToUse,
                                 ImageFileFormat* imageFileFormatToUse)
{
  #if JUCE_CONTENT_SHARING
    startNewShare (callbackToUse);
    prepareImagesThread.reset (new PrepareImagesThread (*this, images, imageFileFormatToUse));
  #else
    ignoreUnused (images, imageFileFormatToUse);

    // Content sharing is not available on this platform!
    jassertfalse;

    if (callbackToUse)
        callbackToUse (false, "Content sharing is not available on this platform!");
  #endif
}

#if JUCE_CONTENT_SHARING
void ContentSharer::filesToSharePrepared()
{
    Array<URL> urls;

    for (const auto& tempFile : temporaryFiles)
        urls.add (URL (tempFile));

    prepareImagesThread = nullptr;
    prepareDataThread = nullptr;

    pimpl->shareFiles (urls);
}
#endif

void ContentSharer::shareData (const MemoryBlock& mb,
                               std::function<void (bool, const String&)> callbackToUse)
{
  #if JUCE_CONTENT_SHARING
    startNewShare (callbackToUse);
    prepareDataThread.reset (new PrepareDataThread (*this, mb));
  #else
    ignoreUnused (mb);

    if (callbackToUse)
        callbackToUse (false, "Content sharing not available on this platform!");
  #endif
}

void ContentSharer::sharingFinished (bool succeeded, const String& errorDescription)
{
    deleteTemporaryFiles();

    std::function<void (bool, String)> cb;
    std::swap (cb, callback);

    String error (errorDescription);

  #if JUCE_CONTENT_SHARING
    pimpl.reset();
  #endif

    if (cb)
        cb (succeeded, error);
}

void ContentSharer::deleteTemporaryFiles()
{
    for (auto& f : temporaryFiles)
        f.deleteFile();

    temporaryFiles.clear();
}

} // namespace juce
