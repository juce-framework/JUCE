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

//==============================================================================
static NSMutableArray* createAllowedTypesArray (const StringArray& filters)
{
    if (filters.size() == 0)
        return nil;

    NSMutableArray* filterArray = [[[NSMutableArray alloc] init] autorelease];

    for (int i = 0; i < filters.size(); ++i)
    {
        // From OS X 10.6 you can only specify allowed extensions, so any filters containing wildcards
        // must be of the form "*.extension"
        jassert (filters[i] == "*"
                 || (filters[i].startsWith ("*.") && filters[i].lastIndexOfChar ('*') == 0));
        const String f (filters[i].replace ("*.", ""));

        if (f == "*")
            return nil;

        [filterArray addObject: juceStringToNS (f)];
    }

    return filterArray;
}

//==============================================================================
class FileChooser::Native final : public Component,
                                  public FileChooser::Pimpl
{
public:
    Native (FileChooser& fileChooser, int flags, FilePreviewComponent* previewComponent)
        : owner (fileChooser), preview (previewComponent),
          selectsDirectories ((flags & FileBrowserComponent::canSelectDirectories)   != 0),
          selectsFiles       ((flags & FileBrowserComponent::canSelectFiles)         != 0),
          isSave             ((flags & FileBrowserComponent::saveMode)               != 0),
          selectMultiple     ((flags & FileBrowserComponent::canSelectMultipleItems) != 0)
    {
        setBounds (0, 0, 0, 0);
        setOpaque (true);

        panel = [&]
        {
            if (SystemStats::isAppSandboxEnabled())
                return isSave ? [[NSSavePanel alloc] init]
                              : [[NSOpenPanel alloc] init];

            static SafeSavePanel safeSavePanel;
            static SafeOpenPanel safeOpenPanel;

            return isSave ? [safeSavePanel.createInstance() init]
                          : [safeOpenPanel.createInstance() init];
        }();

        static DelegateClass delegateClass;

        delegate = [delegateClass.createInstance() init];
        object_setInstanceVariable (delegate, "cppObject", this);

        [panel setDelegate: delegate];

        filters.addTokens (owner.filters.replaceCharacters (",:", ";;"), ";", String());
        filters.trim();
        filters.removeEmptyStrings();

        auto* nsTitle = juceStringToNS (owner.title);
        [panel setTitle: nsTitle];
        [panel setReleasedWhenClosed: YES];

        JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
        [panel setAllowedFileTypes: createAllowedTypesArray (filters)];
        JUCE_END_IGNORE_DEPRECATION_WARNINGS

        if (! isSave)
        {
            auto* openPanel = static_cast<NSOpenPanel*> (panel);

            [openPanel setCanChooseDirectories: selectsDirectories];
            [openPanel setCanChooseFiles: selectsFiles];
            [openPanel setAllowsMultipleSelection: selectMultiple];
            [openPanel setResolvesAliases: YES];
            [openPanel setMessage: nsTitle]; // equivalent to the title bar since 10.11

            if (owner.treatFilePackagesAsDirs)
                [openPanel setTreatsFilePackagesAsDirectories: YES];
        }

        if (preview != nullptr)
        {
            nsViewPreview = [[NSView alloc] initWithFrame: makeCGRect (preview->getLocalBounds())];
            [panel setAccessoryView: nsViewPreview];

            preview->addToDesktop (0, (void*) nsViewPreview);
            preview->setVisible (true);

            if (! isSave)
            {
                auto* openPanel = static_cast<NSOpenPanel*> (panel);
                [openPanel setAccessoryViewDisclosed: YES];
            }
        }

        if (isSave || selectsDirectories)
            [panel setCanCreateDirectories: YES];

        [panel setLevel: NSModalPanelWindowLevel];

        if (owner.startingFile.isDirectory())
        {
            startingDirectory = owner.startingFile.getFullPathName();
        }
        else
        {
            startingDirectory = owner.startingFile.getParentDirectory().getFullPathName();
            filename = owner.startingFile.getFileName();
        }

        [panel setDirectoryURL: createNSURLFromFile (startingDirectory)];
        [panel setNameFieldStringValue: juceStringToNS (filename)];
    }

    ~Native() override
    {
        exitModalState (0);

        if (preview != nullptr)
            preview->removeFromDesktop();

        removeFromDesktop();

        if (panel != nil)
        {
            [panel setDelegate: nil];

            if (nsViewPreview != nil)
            {
                [panel setAccessoryView: nil];
                [nsViewPreview release];
            }

            [panel close];
        }

        if (delegate != nil)
            [delegate release];
    }

    void launch() override
    {
        if (panel != nil)
        {
            setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());
            addToDesktop (0);

            enterModalState (true);

            MessageManager::callAsync ([ref = SafePointer<Native> (this)]
            {
                if (ref == nullptr)
                    return;

                [ref->panel beginWithCompletionHandler: ^(NSInteger result)
                                                        {
                                                            if (auto* ptr = ref.getComponent())
                                                                ptr->finished (result);
                                                        }];

                if (ref->preview != nullptr)
                    ref->preview->toFront (true);
            });
        }
    }

    void runModally() override
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        ensurePanelSafe();

        std::unique_ptr<TemporaryMainMenuWithStandardCommands> tempMenu;

        if (JUCEApplicationBase::isStandaloneApp())
            tempMenu = std::make_unique<TemporaryMainMenuWithStandardCommands> (preview);

        jassert (panel != nil);
        auto result = [panel runModal];
        finished (result);
       #else
        jassertfalse;
       #endif
    }

    bool canModalEventBeSentToComponent (const Component* targetComponent) override
    {
        return TemporaryMainMenuWithStandardCommands::checkModalEvent (preview, targetComponent);
    }

private:
    //==============================================================================
    typedef NSObject<NSOpenSavePanelDelegate> DelegateType;

    static URL urlFromNSURL (NSURL* url)
    {
        const auto scheme = nsStringToJuce ([url scheme]);

        auto pathComponents = StringArray::fromTokens (nsStringToJuce ([url path]), "/", {});

        for (auto& component : pathComponents)
            component = URL::addEscapeChars (component, false);

        return { scheme + "://" + pathComponents.joinIntoString ("/") };
    }

    void finished (NSInteger result)
    {
        Array<URL> chooserResults;

        exitModalState (0);

        if (panel != nil && result == NSModalResponseOK)
        {
            auto addURLResult = [&chooserResults] (NSURL* urlToAdd)
            {
                chooserResults.add (urlFromNSURL (urlToAdd));
            };

            if (isSave)
            {
                addURLResult ([panel URL]);
            }
            else
            {
                auto* openPanel = static_cast<NSOpenPanel*> (panel);
                auto urls = [openPanel URLs];

                for (unsigned int i = 0; i < [urls count]; ++i)
                    addURLResult ([urls objectAtIndex: i]);
            }
        }

        owner.finished (chooserResults);
    }

    BOOL shouldShowURL (const URL& urlToTest)
    {
        for (int i = filters.size(); --i >= 0;)
            if (urlToTest.getFileName().matchesWildcard (filters[i], true))
                return YES;

        const auto f = urlToTest.getLocalFile();
        return f.isDirectory()
                 && ! [[NSWorkspace sharedWorkspace] isFilePackageAtPath: juceStringToNS (f.getFullPathName())];
    }

    void panelSelectionDidChange ([[maybe_unused]] id sender)
    {
        jassert (sender == panel);

        // NB: would need to extend FilePreviewComponent to handle the full list rather than just the first one
        if (preview != nullptr)
            preview->selectedFileChanged (File (getSelectedPaths()[0]));
    }

    StringArray getSelectedPaths() const
    {
        if (panel == nullptr)
            return {};

        StringArray paths;

        if (isSave)
        {
            paths.add (nsStringToJuce ([[panel URL] path]));
        }
        else
        {
            auto* urls = [static_cast<NSOpenPanel*> (panel) URLs];

            for (NSUInteger i = 0; i < [urls count]; ++i)
                paths.add (nsStringToJuce ([[urls objectAtIndex: i] path]));
        }

        return paths;
    }

    //==============================================================================
    FileChooser& owner;
    FilePreviewComponent* preview;
    NSView* nsViewPreview = nullptr;
    bool selectsDirectories, selectsFiles, isSave, selectMultiple;

    NSSavePanel* panel;
    DelegateType* delegate;

    StringArray filters;
    String startingDirectory, filename;

    void ensurePanelSafe()
    {
        // If you hit this, something (probably the plugin host) has modified the panel,
        // allowing the application to terminate while the panel's modal loop is running.
        // This is a very bad idea! Quitting from within the panel's modal loop may cause
        // your plugin/app destructor to run directly from within `runModally`, which will
        // dispose all app resources while they're still in use.
        // A safer alternative is to invoke the FileChooser with `launchAsync`, rather than
        // using the modal launchers.
        jassert ([panel preventsApplicationTerminationWhenModal]);
    }

    static BOOL preventsApplicationTerminationWhenModal (id, SEL) { return YES; }

    template <typename Base>
    struct SafeModalPanel : public ObjCClass<Base>
    {
        explicit SafeModalPanel (const char* name) : ObjCClass<Base> (name)
        {
            this->addMethod (@selector (preventsApplicationTerminationWhenModal),
                             preventsApplicationTerminationWhenModal);

            this->registerClass();
        }
    };

    struct SafeSavePanel : SafeModalPanel<NSSavePanel>
    {
        SafeSavePanel() : SafeModalPanel ("SafeSavePanel_") {}
    };

    struct SafeOpenPanel : SafeModalPanel<NSOpenPanel>
    {
        SafeOpenPanel() : SafeModalPanel ("SafeOpenPanel_") {}
    };

    //==============================================================================
    struct DelegateClass final : public ObjCClass<DelegateType>
    {
        DelegateClass() : ObjCClass<DelegateType> ("JUCEFileChooser_")
        {
            addIvar<Native*> ("cppObject");

            addMethod (@selector (panel:shouldEnableURL:),   shouldEnableURL);
            addMethod (@selector (panelSelectionDidChange:), panelSelectionDidChange);

            addProtocol (@protocol (NSOpenSavePanelDelegate));

            registerClass();
        }

    private:
        static BOOL shouldEnableURL (id self, SEL, id /*sender*/, NSURL* url)
        {
            return getIvar<Native*> (self, "cppObject")->shouldShowURL (urlFromNSURL (url));
        }

        static void panelSelectionDidChange (id self, SEL, id sender)
        {
            getIvar<Native*> (self, "cppObject")->panelSelectionDidChange (sender);
        }
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Native)
};

std::shared_ptr<FileChooser::Pimpl> FileChooser::showPlatformDialog (FileChooser& owner, int flags,
                                                                     FilePreviewComponent* preview)
{
    return std::make_shared<FileChooser::Native> (owner, flags, preview);
}

bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return true;
   #endif
}

} // namespace juce
