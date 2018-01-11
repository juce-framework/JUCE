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
static NSMutableArray* createAllowedTypesArray (const StringArray& filters)
{
    if (filters.size() == 0)
        return nil;

    NSMutableArray* filterArray = [[[NSMutableArray alloc] init] autorelease];

    for (int i = 0; i < filters.size(); ++i)
    {
       #if defined (MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
        // From OS X 10.6 you can only specify allowed extensions, so any filters containing wildcards
        // must be of the form "*.extension"
        jassert (filters[i] == "*"
                 || (filters[i].startsWith ("*.") && filters[i].lastIndexOfChar ('*') == 0));
       #endif

        const String f (filters[i].replace ("*.", ""));

        if (f == "*")
            return nil;

        [filterArray addObject: juceStringToNS (f)];
    }

    return filterArray;
}

//==============================================================================
template <> struct ContainerDeletePolicy<NSSavePanel>    { static void destroy (NSObject* o) { [o release]; } };

class FileChooser::Native     : public Component,
                                public FileChooser::Pimpl
{
public:
    Native (FileChooser& fileChooser, int flags, FilePreviewComponent* previewComponent)
        : owner (fileChooser), preview (previewComponent),
          selectsDirectories ((flags & FileBrowserComponent::canSelectDirectories)   != 0),
          selectsFiles       ((flags & FileBrowserComponent::canSelectFiles)         != 0),
          isSave             ((flags & FileBrowserComponent::saveMode)               != 0),
          selectMultiple     ((flags & FileBrowserComponent::canSelectMultipleItems) != 0),
          panel (isSave ? [[NSSavePanel alloc] init] : [[NSOpenPanel alloc] init])
    {
        setBounds (0, 0, 0, 0);
        setOpaque (true);

        static DelegateClass cls;

        delegate = [cls.createInstance() init];
        object_setInstanceVariable (delegate, "cppObject", this);

        [panel setDelegate: delegate];

        filters.addTokens (owner.filters.replaceCharacters (",:", ";;"), ";", String());
        filters.trim();
        filters.removeEmptyStrings();

        [panel setTitle: juceStringToNS (owner.title)];
        [panel setAllowedFileTypes: createAllowedTypesArray (filters)];

        if (! isSave)
        {
            NSOpenPanel* openPanel = (NSOpenPanel*) panel;

            [openPanel setCanChooseDirectories: selectsDirectories];
            [openPanel setCanChooseFiles: selectsFiles];
            [openPanel setAllowsMultipleSelection: selectMultiple];
            [openPanel setResolvesAliases: YES];

            if (owner.treatFilePackagesAsDirs)
                [openPanel setTreatsFilePackagesAsDirectories: YES];
        }

        if (preview != nullptr)
        {
            nsViewPreview = [[NSView alloc] initWithFrame: makeNSRect (preview->getLocalBounds())];
            preview->addToDesktop (0, (void*) nsViewPreview);
            preview->setVisible (true);

            [panel setAccessoryView: nsViewPreview];
        }

        if (isSave || selectsDirectories)
            [panel setCanCreateDirectories: YES];

        [panel setLevel:NSModalPanelWindowLevel];

        if (owner.startingFile.isDirectory())
        {
            startingDirectory = owner.startingFile.getFullPathName();
        }
        else
        {
            startingDirectory = owner.startingFile.getParentDirectory().getFullPathName();
            filename = owner.startingFile.getFileName();
        }

       #if defined (MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
        [panel setDirectoryURL: createNSURLFromFile (startingDirectory)];
        [panel setNameFieldStringValue: juceStringToNS (filename)];
       #endif
    }

    ~Native()
    {
        exitModalState (0);
        removeFromDesktop();

        if (panel != nil)
        {
            [panel setDelegate: nil];

            if (nsViewPreview != nil)
            {
                [panel setAccessoryView: nil];

                [nsViewPreview release];

                nsViewPreview = nil;
                preview = nullptr;
            }

            [panel close];
            [panel release];
        }

        if (delegate != nil)
        {
            [delegate release];
            delegate = nil;
        }
    }

    void launch() override
    {
        if (panel != nil)
        {
            setAlwaysOnTop (juce_areThereAnyAlwaysOnTopWindows());
            addToDesktop (0);

            enterModalState (true);
            [panel beginWithCompletionHandler:CreateObjCBlock (this, &Native::finished)];
        }
    }

    void runModally() override
    {
        ScopedPointer<TemporaryMainMenuWithStandardCommands> tempMenu;

        if (JUCEApplicationBase::isStandaloneApp())
            tempMenu.reset (new TemporaryMainMenuWithStandardCommands());

        jassert (panel != nil);
       #if defined (MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
        auto result = [panel runModal];
       #else
        auto result = [panel runModalForDirectory: juceStringToNS (startingDirectory)
                                              file: juceStringToNS (filename)];
       #endif

        finished (result);
    }

private:
    //==============================================================================
   #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
    typedef NSObject<NSOpenSavePanelDelegate> DelegateType;
   #else
    typedef NSObject DelegateType;
   #endif

    void finished (NSInteger result)
    {
        Array<URL> chooserResults;

        exitModalState (0);

        if (panel != nil && result == NSFileHandlingPanelOKButton)
        {
            auto addURLResult = [&chooserResults] (NSURL* urlToAdd)
            {
                auto scheme = nsStringToJuce ([urlToAdd scheme]);
                auto path = nsStringToJuce ([urlToAdd path]);
                chooserResults.add (URL (scheme + "://" + path));
            };

            if (isSave)
            {
                addURLResult ([panel URL]);
            }
            else
            {
                auto* openPanel = (NSOpenPanel*) panel;
                auto* urls = [openPanel URLs];

                for (unsigned int i = 0; i < [urls count]; ++i)
                    addURLResult ([urls objectAtIndex: i]);
            }
        }

        owner.finished (chooserResults);
    }

    bool shouldShowFilename (const String& filenameToTest)
    {
        const File f (filenameToTest);
        auto nsFilename = juceStringToNS (filenameToTest);

        for (int i = filters.size(); --i >= 0;)
            if (f.getFileName().matchesWildcard (filters[i], true))
                return true;

       #if (! defined (MAC_OS_X_VERSION_10_7)) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7
        NSError* error;
        NSString* name = [[NSWorkspace sharedWorkspace] typeOfFile: nsFilename error: &error];

        if ([name isEqualToString: nsStringLiteral ("com.apple.alias-file")])
        {
            FSRef ref;
            FSPathMakeRef ((const UInt8*) [nsFilename fileSystemRepresentation], &ref, nullptr);

            Boolean targetIsFolder = false, wasAliased = false;
            FSResolveAliasFileWithMountFlags (&ref, true, &targetIsFolder, &wasAliased, 0);

            return wasAliased && targetIsFolder;
        }
       #endif

        return f.isDirectory()
                 && ! [[NSWorkspace sharedWorkspace] isFilePackageAtPath: nsFilename];
    }

    void panelSelectionDidChange (id sender)
    {
        // NB: would need to extend FilePreviewComponent to handle the full list rather than just the first one
        if (preview != nullptr)
            preview->selectedFileChanged (File (getSelectedPaths (sender)[0]));
    }

    static StringArray getSelectedPaths (id sender)
    {
        StringArray paths;

        if ([sender isKindOfClass: [NSOpenPanel class]])
        {
            NSArray* urls = [(NSOpenPanel*) sender URLs];

            for (NSUInteger i = 0; i < [urls count]; ++i)
                paths.add (nsStringToJuce ([[urls objectAtIndex: i] path]));
        }
        else if ([sender isKindOfClass: [NSSavePanel class]])
        {
            paths.add (nsStringToJuce ([[(NSSavePanel*) sender URL] path]));
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

    //==============================================================================
    struct DelegateClass : ObjCClass<DelegateType>
    {
        DelegateClass()  : ObjCClass <DelegateType> ("JUCEFileChooser_")
        {
            addIvar<Native*> ("cppObject");

            addMethod (@selector (panel:shouldShowFilename:), shouldShowFilename,      "c@:@@");
            addMethod (@selector (panelSelectionDidChange:),  panelSelectionDidChange, "c@");

           #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
            addProtocol (@protocol (NSOpenSavePanelDelegate));
           #endif

            registerClass();
        }

    private:
        static BOOL shouldShowFilename (id self, SEL, id /*sender*/, NSString* filename)
        {
            auto* _this = getIvar<Native*> (self, "cppObject");

            return _this->shouldShowFilename (nsStringToJuce (filename)) ? YES : NO;
        }

        static void panelSelectionDidChange (id self, SEL, id sender)
        {
            auto* _this = getIvar<Native*> (self, "cppObject");

            _this->panelSelectionDidChange (sender);
        }
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Native)
};

FileChooser::Pimpl* FileChooser::showPlatformDialog (FileChooser& owner, int flags,
                                        FilePreviewComponent* preview)
{
    return new FileChooser::Native (owner, flags, preview);
}

bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return true;
   #endif
}

}
