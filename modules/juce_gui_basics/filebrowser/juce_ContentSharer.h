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

#pragma once

namespace juce
{

/** A singleton class responsible for sharing content between apps and devices.

    You can share text, images, files or an arbitrary data block.
*/
class JUCE_API ContentSharer
{
public:
    JUCE_DECLARE_SINGLETON (ContentSharer, false)

    /** Shares the given files. Each URL should be either a full file path
        or it should point to a resource within the application bundle. For
        resources on iOS it should be something like "content/image.png" if you
        want to specify a file from application bundle located in "content"
        directory. On Android you should specify only a filename, without an
        extension.

        Upon completion you will receive a callback with a sharing result. Note:
        Sadly on Android the returned success flag may be wrong as there is no
        standard way the sharing targets report if the sharing operation
        succeeded. Also, the optional error message is always empty on Android.
    */
    void shareFiles (const Array<URL>& files,
                     std::function<void (bool /*success*/, const String& /*error*/)> callback);

    /** Shares the given text.

        Upon completion you will receive a callback with a sharing result. Note:
        Sadly on Android the returned success flag may be wrong as there is no
        standard way the sharing targets report if the sharing operation
        succeeded. Also, the optional error message is always empty on Android.
    */
    void shareText (const String& text,
                    std::function<void (bool /*success*/, const String& /*error*/)> callback);

    /** A convenience function to share an image. This is useful when you have images
        loaded in memory. The images will be written to temporary files first, so if
        you have the images in question stored on disk already call shareFiles() instead.
        By default, images will be saved to PNG files, but you can supply a custom
        ImageFileFormat to override this. The custom file format will be owned and
        deleted by the sharer. e.g.

        @code
        Graphics g (myImage);
        g.setColour (Colours::green);
        g.fillEllipse (20, 20, 300, 200);
        Array<Image> images;
        images.add (myImage);
        ContentSharer::getInstance()->shareImages (images, myCallback);
        @endcode

        Upon completion you will receive a callback with a sharing result. Note:
        Sadly on Android the returned success flag may be wrong as there is no
        standard way the sharing targets report if the sharing operation
        succeeded. Also, the optional error message is always empty on Android.
    */
    void shareImages (const Array<Image>& images,
                      std::function<void (bool /*success*/, const String& /*error*/)> callback,
                      ImageFileFormat* imageFileFormatToUse = nullptr);

    /** A convenience function to share arbitrary data. The data will be written
        to a temporary file and then that file will be shared. If you have
        your data stored on disk already, call shareFiles() instead.

        Upon completion you will receive a callback with a sharing result. Note:
        Sadly on Android the returned success flag may be wrong as there is no
        standard way the sharing targets report if the sharing operation
        succeeded. Also, the optional error message is always empty on Android.
    */
    void shareData (const MemoryBlock& mb,
                    std::function<void (bool /*success*/, const String& /*error*/)> callback);

private:
    ContentSharer();
    ~ContentSharer();

    Array<File> temporaryFiles;

    std::function<void (bool, String)> callback;

  #if JUCE_IOS || JUCE_ANDROID
    struct Pimpl
    {
        virtual ~Pimpl() {}
        virtual void shareFiles (const Array<URL>& files) = 0;
        virtual void shareText (const String& text) = 0;
    };

    ScopedPointer<Pimpl> pimpl;
    Pimpl* createPimpl();

    void startNewShare (std::function<void (bool, const String&)>);

    class ContentSharerNativeImpl;
    friend class ContentSharerNativeImpl;

    class PrepareImagesThread;
    friend class PrepareImagesThread;
    ScopedPointer<PrepareImagesThread> prepareImagesThread;

    class PrepareDataThread;
    friend class PrepareDataThread;
    ScopedPointer<PrepareDataThread> prepareDataThread;

    void filesToSharePrepared();
  #endif

    void deleteTemporaryFiles();
    void sharingFinished (bool, const String&);

  #if JUCE_ANDROID
    friend void* juce_contentSharerOpenFile (void*, void*, void*);
    friend void* juce_contentSharerQuery (void*, void*, void*, void*, void*, void*);
    friend void* juce_contentSharerGetStreamTypes (void*, void*);
    friend void  juce_contentSharingCompleted (int);
  #endif
};

} // namespace juce
