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

@interface FileChooserControllerClass : UIDocumentPickerViewController
- (void) setParent: (FileChooser::Native*) ptr;
@end

@interface FileChooserDelegateClass : NSObject<UIDocumentPickerDelegate>
- (id) initWithOwner: (FileChooser::Native*) owner;
@end

namespace juce
{

#if ! (defined (__IPHONE_16_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_16_0)
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
 #define JUCE_DEPRECATION_IGNORED 1
#endif

class FileChooser::Native  : public FileChooser::Pimpl,
                             public Component,
                             public AsyncUpdater,
                             public std::enable_shared_from_this<Native>
{
public:
    static std::shared_ptr<Native> make (FileChooser& fileChooser, int flags)
    {
        std::shared_ptr<Native> result { new Native (fileChooser, flags) };
        /*  Must be called after forming a shared_ptr to an instance of this class.
            Note that we can't call this directly inside the class constructor, because
            the owning shared_ptr might not yet exist.
        */
        [result->controller.get() setParent: result.get()];
        return result;
    }

    ~Native() override
    {
        exitModalState (0);
    }

    void launch() override
    {
        jassert (shared_from_this() != nullptr);

        /*  Normally, when deleteWhenDismissed is true, the modal component manger will keep a copy of a raw pointer
            to our component and delete it when the modal state has ended. However, this is incompatible with
            our class being tracked by shared_ptr as it will force delete our class regardless of the current
            reference count. On the other hand, it's important that the modal manager keeps a reference as it can
            sometimes be the only reference to our class.

            To do this, we set deleteWhenDismissed to false so that the modal component manager does not delete
            our class. Instead, we pass in a lambda which captures a shared_ptr to ourselves to increase the
            reference count while the component is modal.
        */
        enterModalState (true,
                         ModalCallbackFunction::create ([_self = shared_from_this()] (int) {}),
                         false);
    }

    void runModally() override
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        launch();
        runModalLoop();
       #else
        jassertfalse;
       #endif
    }

    void parentHierarchyChanged() override
    {
        auto* newPeer = dynamic_cast<UIViewComponentPeer*> (getPeer());

        if (peer != newPeer)
        {
            peer = newPeer;

            if (peer != nullptr)
            {
                if (auto* parentController = peer->controller)
                    [parentController showViewController: controller.get() sender: parentController];

                peer->toFront (false);
            }
        }
    }

    void handleAsyncUpdate() override
    {
        pickerWasCancelled();
    }

    //==============================================================================
    void didPickDocumentsAtURLs (NSArray<NSURL*>* urls)
    {
        cancelPendingUpdate();

        const auto isWriting =  controller.get().documentPickerMode == UIDocumentPickerModeExportToService
                             || controller.get().documentPickerMode == UIDocumentPickerModeMoveToService;
        const auto accessOptions = isWriting ? 0 : NSFileCoordinatorReadingWithoutChanges;

        auto* fileCoordinator = [[[NSFileCoordinator alloc] initWithFilePresenter: nil] autorelease];
        auto* intents = [[[NSMutableArray alloc] init] autorelease];

        for (NSURL* url in urls)
        {
            auto* fileAccessIntent = isWriting
                                   ? [NSFileAccessIntent writingIntentWithURL: url options: accessOptions]
                                   : [NSFileAccessIntent readingIntentWithURL: url options: accessOptions];
            [intents addObject: fileAccessIntent];
        }

        [fileCoordinator coordinateAccessWithIntents: intents queue: [NSOperationQueue mainQueue] byAccessor: ^(NSError* err)
        {
            if (err != nil)
            {
                [[maybe_unused]] auto desc = [err localizedDescription];
                jassertfalse;
                return;
            }

            Array<URL> result;

            for (NSURL* url in urls)
            {
                [url startAccessingSecurityScopedResource];

                NSError* error = nil;

                auto* bookmark = [url bookmarkDataWithOptions: 0
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
                    [[maybe_unused]] auto desc = [error localizedDescription];
                    jassertfalse;
                }

                result.add (std::move (juceUrl));
            }

            passResultsToInitiator (std::move (result));
        }];
    }

    void didPickDocumentAtURL (NSURL* url)
    {
        didPickDocumentsAtURLs (@[url]);
    }

    void pickerWasCancelled()
    {
        passResultsToInitiator ({});
    }

private:
    Native (FileChooser& fileChooser, int flags)
        : owner (fileChooser)
    {
        delegate.reset ([[FileChooserDelegateClass alloc] initWithOwner: this]);

        String firstFileExtension;
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

            controller.reset ([[FileChooserControllerClass alloc] initWithURL: url inMode: pickerMode]);
            [url release];
        }
        else
        {
            controller.reset ([[FileChooserControllerClass alloc] initWithDocumentTypes: utTypeArray inMode: UIDocumentPickerModeOpen]);

            if (@available (iOS 11.0, *))
                [controller.get() setAllowsMultipleSelection: (flags & FileBrowserComponent::canSelectMultipleItems) != 0];
        }


        [controller.get() setDelegate: delegate.get()];
        [controller.get() setModalTransitionStyle: UIModalTransitionStyleCrossDissolve];

        setOpaque (false);

        if (fileChooser.parent != nullptr)
        {
            [controller.get() setModalPresentationStyle: UIModalPresentationFullScreen];

            auto chooserBounds = fileChooser.parent->getBounds();
            setBounds (chooserBounds);

            setAlwaysOnTop (true);
            fileChooser.parent->addAndMakeVisible (this);
        }
        else
        {
            if (SystemStats::isRunningInAppExtensionSandbox())
            {
                // Opening a native top-level window in an AUv3 is not allowed (sandboxing). You need to specify a
                // parent component (for example your editor) to parent the native file chooser window. To do this
                // specify a parent component in the FileChooser's constructor!
                jassertfalse;
                return;
            }

            auto chooserBounds = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
            setBounds (chooserBounds);

            setAlwaysOnTop (true);
            setVisible (true);
            addToDesktop (0);
        }
    }

    void passResultsToInitiator (Array<URL> urls)
    {
        cancelPendingUpdate();
        exitModalState (0);

        // If the caller attempts to show a platform-native dialog box inside the results callback (e.g. in the DialogsDemo)
        // then the original peer must already have focus. Otherwise, there's a danger that either the invisible FileChooser
        // components will display the popup, locking the application, or maybe no component will have focus, and the
        // dialog won't show at all.
        for (auto i = 0; i < ComponentPeer::getNumPeers(); ++i)
            if (auto* p = ComponentPeer::getPeer (i))
                if (p != getPeer())
                    if (auto* view = (UIView*) p->getNativeHandle())
                        if ([view becomeFirstResponder] && [view isFirstResponder])
                            break;

        // Calling owner.finished will delete this Pimpl instance, so don't call any more member functions here!
        owner.finished (std::move (urls));
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
                CFUniquePtr<CFStringRef> fileExtensionCF (fileExtension.toCFString());

                if (firstExtension.isEmpty())
                    firstExtension = fileExtension;

                if (auto tag = CFUniquePtr<CFStringRef> (UTTypeCreatePreferredIdentifierForTag (kUTTagClassFilenameExtension, fileExtensionCF.get(), nullptr)))
                    result.add (String::fromCFString (tag.get()));
            }
        }
        else
        {
            result.add ("public.data");
        }

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
    FileChooser& owner;
    NSUniquePtr<NSObject<UIDocumentPickerDelegate>> delegate;
    NSUniquePtr<FileChooserControllerClass> controller;
    UIViewComponentPeer* peer = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Native)
};

//==============================================================================
bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return true;
   #endif
}

std::shared_ptr<FileChooser::Pimpl> FileChooser::showPlatformDialog (FileChooser& owner, int flags,
                                                                     FilePreviewComponent*)
{
    return Native::make (owner, flags);
}

#if JUCE_DEPRECATION_IGNORED
 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif

} // namespace juce

@implementation FileChooserControllerClass
{
    std::weak_ptr<FileChooser::Native> ptr;
}

- (void) setParent: (FileChooser::Native*) parent
{
    jassert (parent != nullptr);
    jassert (parent->shared_from_this() != nullptr);
    ptr = parent->weak_from_this();
}

- (void) viewDidDisappear: (BOOL) animated
{
    [super viewDidDisappear: animated];

    if (auto nativeParent = ptr.lock())
        nativeParent->triggerAsyncUpdate();
}

@end

@implementation FileChooserDelegateClass
{
    FileChooser::Native* owner;
}

- (id) initWithOwner: (FileChooser::Native*) o
{
    self = [super init];
    owner = o;
    return self;
}

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-implementations")
- (void) documentPicker: (UIDocumentPickerViewController*) controller didPickDocumentAtURL: (NSURL*) url
{
    if (owner != nullptr)
        owner->didPickDocumentAtURL (url);
}
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

- (void) documentPicker: (UIDocumentPickerViewController*) controller didPickDocumentsAtURLs: (NSArray<NSURL*>*) urls
{
    if (owner != nullptr)
        owner->didPickDocumentsAtURLs (urls);
}

- (void) documentPickerWasCancelled: (UIDocumentPickerViewController*) controller
{
    if (owner != nullptr)
        owner->pickerWasCancelled();
}

@end
