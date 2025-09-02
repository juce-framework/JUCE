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

@interface FileChooserControllerClass : UIDocumentPickerViewController
- (void) setParent: (std::shared_ptr<FileChooser::Native>) ptr;
@end

@interface FileChooserDelegateClass : NSObject<UIDocumentPickerDelegate, UIAdaptivePresentationControllerDelegate>
- (void) setParent: (std::shared_ptr<FileChooser::Native>) owner;
@end

namespace juce
{

//==============================================================================
class FileChooser::Native final : public FileChooser::Pimpl,
                                  public detail::NativeModalWrapperComponent,
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
        [result->controller.get() setParent: result];
        [result->delegate.get() setParent: result];
        return result;
    }

    void launch() override
    {
        jassert (shared_from_this() != nullptr);

        /*  Normally, when deleteWhenDismissed is true, the modal component manager will keep a copy of a raw pointer
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

    //==============================================================================
    void didPickDocumentsAtURLs (NSArray<NSURL*>* urls)
    {
        const auto isWriting = (savedFlags & FileBrowserComponent::saveMode) != 0;
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

        auto strong = shared_from_this();

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

            strong->passResultsToInitiator (std::move (result));
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
    UIViewController* getViewController() const override { return controller.get(); }

    struct CreateSaveControllerTrait
    {
        API_AVAILABLE (ios (14))
        static FileChooserControllerClass* newFn (NSURL* url, const File& currentFileOrDirectory)
        {
            return [[FileChooserControllerClass alloc] initForExportingURLs: @[url] asCopy: currentFileOrDirectory.existsAsFile()];
        }

        static FileChooserControllerClass* oldFn (NSURL* url, const File& currentFileOrDirectory)
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
            const auto pickerMode = currentFileOrDirectory.existsAsFile()
                                  ? UIDocumentPickerModeExportToService
                                  : UIDocumentPickerModeMoveToService;
            return [[FileChooserControllerClass alloc] initWithURL: url inMode: pickerMode];
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
    };

    struct CreateOpenControllerTrait
    {
        API_AVAILABLE (ios (14))
        static FileChooserControllerClass* newFn (int flags, const StringArray& validExtensions)
        {
            NSUniquePtr<NSMutableArray> types ([[NSMutableArray alloc] init]);

            if ((flags & FileBrowserComponent::canSelectDirectories) != 0)
            {
                if (auto* ptr = [UTType typeWithIdentifier: @"public.folder"])
                    [types.get() addObject: ptr];
            }
            else
            {
                if (validExtensions.isEmpty())
                    if (auto* ptr = [UTType typeWithIdentifier: @"public.data"])
                        [types.get() addObject: ptr];

                for (const auto& extension : validExtensions)
                    if (auto* ptr = [UTType typeWithFilenameExtension: juceStringToNS (extension)])
                        [types.get() addObject: ptr];
            }

            return [[FileChooserControllerClass alloc] initForOpeningContentTypes: types.get()];
        }

        static FileChooserControllerClass* oldFn (int flags, const StringArray& validExtensions)
        {
            const NSUniquePtr<NSArray> utTypeArray { std::invoke ([&]
            {
                if ((flags & FileBrowserComponent::canSelectDirectories) != 0)
                    return @[@"public.folder"];

                if (validExtensions.isEmpty())
                    return @[@"public.data"];

                StringArray result;

                for (const auto& extension : validExtensions)
                {
                    if (extension.isEmpty())
                        continue;

                    CFUniquePtr<CFStringRef> fileExtensionCF (extension.toCFString());

                    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
                    if (CFUniquePtr<CFStringRef> tag { UTTypeCreatePreferredIdentifierForTag (kUTTagClassFilenameExtension, fileExtensionCF.get(), nullptr) })
                        result.add (String::fromCFString (tag.get()));
                    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
                }

                return createNSArrayFromStringArray (result);
            }) };

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
            return [[FileChooserControllerClass alloc] initWithDocumentTypes: utTypeArray.get() inMode: UIDocumentPickerModeOpen];
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
    };

    Native (FileChooser& fileChooser, int flags)
        : owner (fileChooser),
          savedFlags (flags)
    {
        delegate.reset ([[FileChooserDelegateClass alloc] init]);

        const auto validExtensions = getValidExtensionsForWildcards (owner.filters);

        if ((flags & FileBrowserComponent::saveMode) != 0)
        {
            auto currentFileOrDirectory = owner.startingFile;

            if (! currentFileOrDirectory.existsAsFile())
            {
                const auto extension = validExtensions.isEmpty() ? String()
                                                                 : validExtensions.getReference (0);
                const auto filename = getFilename (currentFileOrDirectory, extension);
                const auto tmpDirectory = File::createTempFile ("JUCE-filepath");

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

            const NSUniquePtr<NSURL> url { [[NSURL alloc] initFileURLWithPath: juceStringToNS (currentFileOrDirectory.getFullPathName())] };
            controller.reset (ifelse_14_0<CreateSaveControllerTrait> (url.get(), currentFileOrDirectory));
        }
        else
        {
            controller.reset (ifelse_14_0<CreateOpenControllerTrait> (flags, validExtensions));
            [controller.get() setAllowsMultipleSelection: (flags & FileBrowserComponent::canSelectMultipleItems) != 0];
        }

        [controller.get() setDelegate: delegate.get()];

        if (auto* pc = [controller.get() presentationController])
            [pc setDelegate: delegate.get()];

        displayNativeWindowModally (fileChooser.parent);
    }

    void passResultsToInitiator (Array<URL> urls)
    {
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
    static StringArray getValidExtensionsForWildcards (const String& filterWildcards)
    {
        const auto filters = StringArray::fromTokens (filterWildcards, ";", "");

        if (filters.contains ("*") || filters.isEmpty())
            return {};

        StringArray result;

        for (const auto& filter : filters)
        {
            if (filter.isEmpty())
                continue;

            // iOS only supports file extension wild cards
            jassert (filter.upToLastOccurrenceOf (".", true, false) == "*.");

            result.add (filter.fromLastOccurrenceOf (".", false, false));
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
    NSUniquePtr<FileChooserDelegateClass> delegate;
    NSUniquePtr<FileChooserControllerClass> controller;
    int savedFlags = 0;

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

} // namespace juce

@implementation FileChooserControllerClass
{
    std::weak_ptr<FileChooser::Native> ptr;
}

- (void) setParent: (std::shared_ptr<FileChooser::Native>) parent
{
    ptr = parent;
}

@end

@implementation FileChooserDelegateClass
{
    std::weak_ptr<FileChooser::Native> weak;
}

- (void) setParent: (std::shared_ptr<FileChooser::Native>) o
{
    weak = o;
}

- (void) documentPicker: (UIDocumentPickerViewController*) controller didPickDocumentsAtURLs: (NSArray<NSURL*>*) urls
{
    if (auto strong = weak.lock())
        strong->didPickDocumentsAtURLs (urls);
}

- (void) documentPickerWasCancelled: (UIDocumentPickerViewController*) controller
{
    if (auto strong = weak.lock())
        strong->pickerWasCancelled();
}

- (void) presentationControllerDidDismiss: (UIPresentationController *) presentationController
{
    if (auto strong = weak.lock())
        strong->pickerWasCancelled();
}

@end
