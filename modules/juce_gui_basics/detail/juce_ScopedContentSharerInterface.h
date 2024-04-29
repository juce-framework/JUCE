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

namespace juce::detail
{

/*
    Instances of this type can show and dismiss a content sharer.

    This is an interface rather than a concrete type so that platforms can pick an implementation at
    runtime if necessary.
*/
struct ScopedContentSharerInterface
{
    virtual ~ScopedContentSharerInterface() = default;

    /*  Shows the content sharer.

        When the content sharer exits normally, it should send the result to the passed-in function.
        The passed-in function is safe to call from any thread at any time.
    */
    virtual void runAsync (ContentSharer::Callback callback)
    {
        jassertfalse;
        NullCheckedInvocation::invoke (callback, false, "Content sharing not available on this platform!");
    }

    /*  Forcefully closes the content sharer.

        This will be called when the content sharer handle has fallen out of scope.
        If the content sharer has already been closed by the user, this shouldn't do anything.
    */
    virtual void close() {}

    /*  Implemented differently for each platform. */
    static std::unique_ptr<ScopedContentSharerInterface> shareFiles (const Array<URL>&, Component*);
    static std::unique_ptr<ScopedContentSharerInterface> shareText (const String&, Component*);

    /*  Implemented below. */
    static std::unique_ptr<ScopedContentSharerInterface> shareImages (const Array<Image>&, std::unique_ptr<ImageFileFormat>, Component*);
    static std::unique_ptr<ScopedContentSharerInterface> shareData (MemoryBlock, Component*);
};

class TemporaryFilesDecorator : public ScopedContentSharerInterface,
                                public AsyncUpdater
{
public:
    explicit TemporaryFilesDecorator (Component* parentIn)
        : parent (parentIn) {}

    void runAsync (ContentSharer::Callback cb) override
    {
        callback = std::move (cb);

        task = std::async (std::launch::async, [this]
        {
            std::tie (temporaryFiles, error) = prepareTemporaryFiles();
            triggerAsyncUpdate();
        });
    }

    void close() override
    {
        if (inner != nullptr)
            inner->close();
    }

private:
    virtual std::tuple<Array<URL>, String> prepareTemporaryFiles() const = 0;

    void handleAsyncUpdate() override
    {
        if (error.isNotEmpty())
        {
            NullCheckedInvocation::invoke (callback, false, error);
            return;
        }

        inner = shareFiles (temporaryFiles, parent);

        if (inner == nullptr)
        {
            NullCheckedInvocation::invoke (callback, false, TRANS ("Failed to create file sharer"));
            return;
        }

        inner->runAsync (callback);
    }

    Array<URL> temporaryFiles;
    String error;
    std::unique_ptr<ScopedContentSharerInterface> inner;
    ContentSharer::Callback callback;
    std::future<void> task;
    Component* parent = nullptr;
};

std::unique_ptr<ScopedContentSharerInterface> ScopedContentSharerInterface::shareImages (const Array<Image>& images,
                                                                                         std::unique_ptr<ImageFileFormat> format,
                                                                                         Component* parent)
{
    class Decorator : public TemporaryFilesDecorator
    {
    public:
        Decorator (Array<Image> imagesIn, std::unique_ptr<ImageFileFormat> formatIn, Component* parentIn)
            : TemporaryFilesDecorator (parentIn), images (std::move (imagesIn)), format (std::move (formatIn)) {}

    private:
        std::tuple<Array<URL>, String> prepareTemporaryFiles() const override
        {
            const auto extension = format->getFormatName().toLowerCase();

            Array<URL> result;

            for (const auto& image : images)
            {
                File tempFile = File::createTempFile (extension);

                if (! tempFile.create().wasOk())
                    return { Array<URL>{}, TRANS ("Failed to create temporary file") };

                std::unique_ptr<FileOutputStream> outputStream (tempFile.createOutputStream());

                if (outputStream == nullptr)
                    return { Array<URL>{}, TRANS ("Failed to open temporary file for writing") };

                if (format->writeImageToStream (image, *outputStream))
                    result.add (URL (tempFile));
            }

            jassert (std::all_of (result.begin(),
                                  result.end(),
                                  [] (const auto& url)
                                  {
                                      return url.isLocalFile() && url.getLocalFile().existsAsFile();
                                  }));

            return { std::move (result), String{} };
        }

        Array<Image> images;
        std::unique_ptr<ImageFileFormat> format;
    };

    return std::make_unique<Decorator> (images,
                                        format == nullptr ? std::make_unique<PNGImageFormat>() : std::move (format),
                                        parent);
}

std::unique_ptr<ScopedContentSharerInterface> ScopedContentSharerInterface::shareData (MemoryBlock mb, Component* parent)
{
    class Decorator : public TemporaryFilesDecorator
    {
    public:
        Decorator (MemoryBlock mbIn, Component* parentIn)
            : TemporaryFilesDecorator (parentIn), mb (std::move (mbIn)) {}

    private:
        std::tuple<Array<URL>, String> prepareTemporaryFiles() const override
        {
            File tempFile = File::createTempFile ("data");

            if (! tempFile.create().wasOk())
                return { Array<URL>{}, TRANS ("Failed to create temporary file") };

            std::unique_ptr<FileOutputStream> outputStream (tempFile.createOutputStream());

            if (outputStream == nullptr)
                return { Array<URL>{}, TRANS ("Failed to open temporary file for writing") };

            size_t pos = 0;
            size_t totalSize = mb.getSize();

            while (pos < totalSize)
            {
                size_t numToWrite = std::min ((size_t) 8192, totalSize - pos);

                if (! outputStream->write (mb.begin() + pos, numToWrite))
                    return { Array<URL>{}, TRANS ("Failed to write to temporary file") };

                pos += numToWrite;
            }

            return { Array<URL> { URL (tempFile) }, String{} };
        }

        MemoryBlock mb;
    };

    return std::make_unique<Decorator> (std::move (mb), parent);
}

} // namespace juce::detail
