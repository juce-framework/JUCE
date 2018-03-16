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

//==============================================================================
template <> struct ContainerDeletePolicy<UIDocumentPickerViewController>     { static void destroy (NSObject* o) { [o release]; } };
template <> struct ContainerDeletePolicy<NSObject<UIDocumentPickerDelegate>> { static void destroy (NSObject* o) { [o release]; } };

class FileChooser::Native    : private Component,
                               public FileChooser::Pimpl
{
public:
    Native (FileChooser& fileChooser, int flags)
        : owner (fileChooser)
    {
        String firstFileExtension;

        static FileChooserDelegateClass cls;
        delegate = [cls.createInstance() init];
        FileChooserDelegateClass::setOwner (delegate, this);

        auto utTypeArray = createNSArrayFromStringArray (getUTTypesForWildcards (owner.filters, firstFileExtension));

        if ((flags & FileBrowserComponent::saveMode) != 0)
        {
            auto currentFileOrDirectory = owner.startingFile;

            UIDocumentPickerMode pickerMode = currentFileOrDirectory.existsAsFile()
                                                ? UIDocumentPickerModeExportToService
                                                : UIDocumentPickerModeMoveToService;

            if (! currentFileOrDirectory.existsAsFile())
            {
                auto filename = getFilename (currentFileOrDirectory, firstFileExtension);
                auto tmpDirectory = File::createTempFile ("JUCE-filepath");

                if (tmpDirectory.createDirectory().wasOk())
                {
                    currentFileOrDirectory = tmpDirectory.getChildFile (filename);
                    currentFileOrDirectory.replaceWithText ("");
                }
                else
                {
                    // Temporary directory creation failed! You need to specify a
                    // path you have write access to. Saving will not work for
                    // current path.
                    jassertfalse;
                }
            }

            auto url = [[NSURL alloc] initFileURLWithPath: juceStringToNS (currentFileOrDirectory.getFullPathName())];
            controller = [[UIDocumentPickerViewController alloc] initWithURL: url
                                                                      inMode: pickerMode];
            [url release];
        }
        else
        {
            controller = [[UIDocumentPickerViewController alloc] initWithDocumentTypes: utTypeArray
                                                                                inMode: UIDocumentPickerModeOpen];
        }

        [controller setDelegate: delegate];
        [controller setModalTransitionStyle: UIModalTransitionStyleCrossDissolve];

        setOpaque (false);

        auto chooserBounds = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
        setBounds (chooserBounds);

        setAlwaysOnTop (true);
        addToDesktop (0);
    }

    ~Native()
    {
        exitModalState (0);
    }

    void launch() override
    {
        enterModalState (true, nullptr, true);
    }

    void runModally() override
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        runModalLoop();
       #endif
    }

private:
    //==============================================================================
    void parentHierarchyChanged() override
    {
        auto* newPeer = dynamic_cast<UIViewComponentPeer*> (getPeer());

        if (peer != newPeer)
        {
            peer = newPeer;

            if (auto* parentController = peer->controller)
                [parentController showViewController: controller sender: parentController];

            if (peer->view.window != nil)
                peer->view.window.autoresizesSubviews = YES;
        }
    }

    //==============================================================================
    static StringArray getUTTypesForWildcards (const String& filterWildcards, String& firstExtension)
    {
        auto filters = StringArray::fromTokens (filterWildcards, ";", "");
        StringArray result;

        firstExtension = {};

        if (! filters.contains ("*") && filters.size() > 0)
        {
            for (auto filter : filters)
            {
                if (filter.isEmpty())
                    continue;

                // iOS only supports file extension wild cards
                jassert (filter.upToLastOccurrenceOf (".", true, false) == "*.");

                auto fileExtension = filter.fromLastOccurrenceOf (".", false, false);
                auto fileExtensionCF = fileExtension.toCFString();

                if (firstExtension.isEmpty())
                    firstExtension = fileExtension;

                auto tag = UTTypeCreatePreferredIdentifierForTag (kUTTagClassFilenameExtension, fileExtensionCF, nullptr);

                if (tag != nullptr)
                {
                    result.add (String::fromCFString (tag));
                    CFRelease (tag);
                }

                CFRelease (fileExtensionCF);
            }
        }
        else
            result.add ("public.data");

        return result;
    }

    static String getFilename (const File& path, const String& fallbackExtension)
    {
        auto filename  = path.getFileNameWithoutExtension();
        auto extension = path.getFileExtension().substring (1);

        if (filename.isEmpty())
            filename = "Untitled";

        if (extension.isEmpty())
            extension = fallbackExtension;

        if (extension.isNotEmpty())
            filename += "." + extension;

        return filename;
    }

    //==============================================================================
    void didPickDocumentAtURL (NSURL* url)
    {
        bool isWriting = controller.get().documentPickerMode == UIDocumentPickerModeExportToService
                       | controller.get().documentPickerMode == UIDocumentPickerModeMoveToService;

        NSUInteger accessOptions = isWriting ? 0 : NSFileCoordinatorReadingWithoutChanges;

        auto* fileAccessIntent = isWriting
                               ? [NSFileAccessIntent writingIntentWithURL: url options: accessOptions]
                               : [NSFileAccessIntent readingIntentWithURL: url options: accessOptions];

        NSArray<NSFileAccessIntent*>* intents = @[fileAccessIntent];

        auto* fileCoordinator = [[NSFileCoordinator alloc] initWithFilePresenter: nil];

        [fileCoordinator coordinateAccessWithIntents: intents queue: [NSOperationQueue mainQueue] byAccessor: ^(NSError* err)
        {
            Array<URL> chooserResults;

            if (err == nil)
            {
                [url startAccessingSecurityScopedResource];

                NSError* error = nil;

                NSData* bookmark = [url bookmarkDataWithOptions: 0
                                 includingResourceValuesForKeys: nil
                                                  relativeToURL: nil
                                                          error: &error];

                [bookmark retain];

                [url stopAccessingSecurityScopedResource];

                URL juceUrl (nsStringToJuce ([url absoluteString]));

                if (error == nil)
                {
                    setURLBookmark (juceUrl, (void*) bookmark);
                }
                else
                {
                    auto* desc = [error localizedDescription];
                    ignoreUnused (desc);
                    jassertfalse;
                }

                chooserResults.add (juceUrl);
            }
            else
            {
                auto* desc = [err localizedDescription];
                ignoreUnused (desc);
                jassertfalse;
            }

            owner.finished (chooserResults);
        }];
    }

    void pickerWasCancelled()
    {
        Array<URL> chooserResults;

        owner.finished (chooserResults);
        exitModalState (0);
    }

    //==============================================================================
    struct FileChooserDelegateClass  : public ObjCClass<NSObject<UIDocumentPickerDelegate>>
    {
        FileChooserDelegateClass()  : ObjCClass<NSObject<UIDocumentPickerDelegate>> ("FileChooserDelegate_")
        {
            addIvar<Native*> ("owner");

            addMethod (@selector (documentPicker:didPickDocumentAtURL:), didPickDocumentAtURL,       "v@:@@");
            addMethod (@selector (documentPickerWasCancelled:),          documentPickerWasCancelled, "v@:@");

            addProtocol (@protocol (UIDocumentPickerDelegate));

            registerClass();
        }

        static void setOwner (id self, Native* owner)   { object_setInstanceVariable (self, "owner", owner); }
        static Native* getOwner (id self)               { return getIvar<Native*> (self, "owner"); }

        //==============================================================================
        static void didPickDocumentAtURL (id self, SEL, UIDocumentPickerViewController*, NSURL* url)
        {
            auto picker = getOwner (self);

            if (picker != nullptr)
                picker->didPickDocumentAtURL (url);
        }

        static void documentPickerWasCancelled (id self, SEL, UIDocumentPickerViewController*)
        {
            auto picker = getOwner (self);

            if (picker != nullptr)
                picker->pickerWasCancelled();
        }
    };

    //==============================================================================
    FileChooser& owner;
    ScopedPointer<NSObject<UIDocumentPickerDelegate>> delegate;
    ScopedPointer<UIDocumentPickerViewController> controller;
    UIViewComponentPeer* peer = nullptr;

    static FileChooserDelegateClass fileChooserDelegateClass;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Native)
};

//==============================================================================
bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return [[NSFileManager defaultManager] ubiquityIdentityToken] != nil;
   #endif
}

FileChooser::Pimpl* FileChooser::showPlatformDialog (FileChooser& owner, int flags,
                                                     FilePreviewComponent*)
{
    return new FileChooser::Native (owner, flags);
}

} // namespace juce
