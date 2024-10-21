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
- (void) setParent: (FileChooser::Native*) ptr;
@end

@interface FileChooserDelegateClass : NSObject<UIDocumentPickerDelegate, UIAdaptivePresentationControllerDelegate>
- (id) initWithOwner: (FileChooser::Native*) owner;
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
        [result->controller.get() setParent: result.get()];
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
    UIViewController* getViewController() const override { return controller.get(); }

    Native (FileChooser& fileChooser, int flags)
        : owner (fileChooser)
    {
        delegate.reset ([[FileChooserDelegateClass alloc] initWithOwner: this]);

        const auto validExtensions = getValidExtensionsForWildcards (owner.filters);
        const auto utTypeArray = (flags & FileBrowserComponent::canSelectDirectories) != 0
                               ? @[@"public.folder"]
                               : createNSArrayFromStringArray (getUTTypesForExtensions (validExtensions));

        if ((flags & FileBrowserComponent::saveMode) != 0)
        {
            auto currentFileOrDirectory = owner.startingFile;

            UIDocumentPickerMode pickerMode = currentFileOrDirectory.existsAsFile()
                                                ? UIDocumentPickerModeExportToService
                                                : UIDocumentPickerModeMoveToService;

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

            auto url = [[NSURL alloc] initFileURLWithPath: juceStringToNS (currentFileOrDirectory.getFullPathName())];

            controller.reset ([[FileChooserControllerClass alloc] initWithURL: url inMode: pickerMode]);
            [url release];
        }
        else
        {
            controller.reset ([[FileChooserControllerClass alloc] initWithDocumentTypes: utTypeArray inMode: UIDocumentPickerModeOpen]);

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

    static StringArray getUTTypesForExtensions (const StringArray& extensions)
    {
        if (extensions.isEmpty())
            return { "public.data" };

        StringArray result;

        for (const auto& extension : extensions)
        {
            if (extension.isEmpty())
                continue;

            CFUniquePtr<CFStringRef> fileExtensionCF (extension.toCFString());

            if (const auto tag = CFUniquePtr<CFStringRef> (UTTypeCreatePreferredIdentifierForTag (kUTTagClassFilenameExtension, fileExtensionCF.get(), nullptr)))
                result.add (String::fromCFString (tag.get()));
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
    NSUniquePtr<NSObject<UIDocumentPickerDelegate, UIAdaptivePresentationControllerDelegate>> delegate;
    NSUniquePtr<FileChooserControllerClass> controller;

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

- (void) setParent: (FileChooser::Native*) parent
{
    jassert (parent != nullptr);
    jassert (parent->shared_from_this() != nullptr);
    ptr = parent->weak_from_this();
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

- (void) presentationControllerDidDismiss: (UIPresentationController *) presentationController
{
    if (owner != nullptr)
        owner->pickerWasCancelled();
}

@end
