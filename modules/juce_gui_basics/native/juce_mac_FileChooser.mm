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

#if JUCE_MAC

struct FileChooserDelegateClass  : public ObjCClass <NSObject>
{
    FileChooserDelegateClass()  : ObjCClass <NSObject> ("JUCEFileChooser_")
    {
        addIvar<StringArray*> ("filters");
        addIvar<FilePreviewComponent*> ("filePreviewComponent");

        addMethod (@selector (dealloc),                   dealloc,                 "v@:");
        addMethod (@selector (panel:shouldShowFilename:), shouldShowFilename,      "c@:@@");
        addMethod (@selector (panelSelectionDidChange:),  panelSelectionDidChange, "c@");

       #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
        addProtocol (@protocol (NSOpenSavePanelDelegate));
       #endif

        registerClass();
    }

    static void setFilters (id self, StringArray* filters)                      { object_setInstanceVariable (self, "filters", filters); }
    static void setFilePreviewComponent (id self, FilePreviewComponent* comp)   { object_setInstanceVariable (self, "filePreviewComponent", comp); }
    static StringArray* getFilters (id self)                                    { return getIvar<StringArray*> (self, "filters"); }
    static FilePreviewComponent* getFilePreviewComponent (id self)              { return getIvar<FilePreviewComponent*> (self, "filePreviewComponent"); }

private:
    static void dealloc (id self, SEL)
    {
        delete getFilters (self);
        sendSuperclassMessage (self, @selector (dealloc));
    }

    static BOOL shouldShowFilename (id self, SEL, id /*sender*/, NSString* filename)
    {
        StringArray* const filters = getFilters (self);

        const File f (nsStringToJuce (filename));

        for (int i = filters->size(); --i >= 0;)
            if (f.getFileName().matchesWildcard ((*filters)[i], true))
                return true;

       #if (! defined (MAC_OS_X_VERSION_10_7)) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_7
        NSError* error;
        NSString* name = [[NSWorkspace sharedWorkspace] typeOfFile: filename error: &error];

        if ([name isEqualToString: nsStringLiteral ("com.apple.alias-file")])
        {
            FSRef ref;
            FSPathMakeRef ((const UInt8*) [filename fileSystemRepresentation], &ref, nullptr);

            Boolean targetIsFolder = false, wasAliased = false;
            FSResolveAliasFileWithMountFlags (&ref, true, &targetIsFolder, &wasAliased, 0);

            return wasAliased && targetIsFolder;
        }
       #endif

        return f.isDirectory()
                 && ! [[NSWorkspace sharedWorkspace] isFilePackageAtPath: filename];
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

    static void panelSelectionDidChange (id self, SEL, id sender)
    {
        // NB: would need to extend FilePreviewComponent to handle the full list rather than just the first one
        if (FilePreviewComponent* const previewComp = getFilePreviewComponent (self))
            previewComp->selectedFileChanged (File (getSelectedPaths (sender)[0]));
    }
};

static NSMutableArray* createAllowedTypesArray (const StringArray& filters)
{
    if (filters.size() == 0)
        return nil;

    NSMutableArray* filterArray = [[[NSMutableArray alloc] init] autorelease];

    for (int i = 0; i < filters.size(); ++i)
    {
        const String f (filters[i].replace ("*.", ""));

        if (f == "*")
            return nil;

        [filterArray addObject: juceStringToNS (f)];
    }

    return filterArray;
}

//==============================================================================
void FileChooser::showPlatformDialog (Array<File>& results,
                                      const String& title,
                                      const File& currentFileOrDirectory,
                                      const String& filter,
                                      bool selectsDirectory,
                                      bool selectsFiles,
                                      bool isSaveDialogue,
                                      bool /*warnAboutOverwritingExistingFiles*/,
                                      bool selectMultipleFiles,
                                      bool treatFilePackagesAsDirs,
                                      FilePreviewComponent* extraInfoComponent)
{
    JUCE_AUTORELEASEPOOL
    {
        ScopedPointer<TemporaryMainMenuWithStandardCommands> tempMenu;
        if (JUCEApplicationBase::isStandaloneApp())
            tempMenu = new TemporaryMainMenuWithStandardCommands();

        StringArray* filters = new StringArray();
        filters->addTokens (filter.replaceCharacters (",:", ";;"), ";", String());
        filters->trim();
        filters->removeEmptyStrings();

       #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
        typedef NSObject<NSOpenSavePanelDelegate> DelegateType;
       #else
        typedef NSObject DelegateType;
       #endif

        static FileChooserDelegateClass cls;
        DelegateType* delegate = (DelegateType*) [[cls.createInstance() init] autorelease];
        FileChooserDelegateClass::setFilters (delegate, filters);

        NSSavePanel* panel = isSaveDialogue ? [NSSavePanel savePanel]
                                            : [NSOpenPanel openPanel];

        [panel setTitle: juceStringToNS (title)];
        [panel setAllowedFileTypes: createAllowedTypesArray (*filters)];

        if (! isSaveDialogue)
        {
            NSOpenPanel* openPanel = (NSOpenPanel*) panel;
            [openPanel setCanChooseDirectories: selectsDirectory];
            [openPanel setCanChooseFiles: selectsFiles];
            [openPanel setAllowsMultipleSelection: selectMultipleFiles];
            [openPanel setResolvesAliases: YES];

            if (treatFilePackagesAsDirs)
                [openPanel setTreatsFilePackagesAsDirectories: YES];
        }

        if (extraInfoComponent != nullptr)
        {
            NSView* view = [[[NSView alloc] initWithFrame: makeNSRect (extraInfoComponent->getLocalBounds())] autorelease];
            extraInfoComponent->addToDesktop (0, (void*) view);
            extraInfoComponent->setVisible (true);
            FileChooserDelegateClass::setFilePreviewComponent (delegate, extraInfoComponent);

            [panel setAccessoryView: view];
        }

        [panel setDelegate: delegate];

        if (isSaveDialogue || selectsDirectory)
            [panel setCanCreateDirectories: YES];

        String directory, filename;

        if (currentFileOrDirectory.isDirectory())
        {
            directory = currentFileOrDirectory.getFullPathName();
        }
        else
        {
            directory = currentFileOrDirectory.getParentDirectory().getFullPathName();
            filename = currentFileOrDirectory.getFileName();
        }

       #if defined (MAC_OS_X_VERSION_10_6) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6)
        [panel setDirectoryURL: createNSURLFromFile (directory)];
        [panel setNameFieldStringValue: juceStringToNS (filename)];

        if ([panel runModal] == 1 /*NSModalResponseOK*/)
       #else
        if ([panel runModalForDirectory: juceStringToNS (directory)
                                   file: juceStringToNS (filename)] == 1 /*NSModalResponseOK*/)
       #endif
        {
            if (isSaveDialogue)
            {
                results.add (File (nsStringToJuce ([[panel URL] path])));
            }
            else
            {
                NSOpenPanel* openPanel = (NSOpenPanel*) panel;
                NSArray* urls = [openPanel URLs];

                for (unsigned int i = 0; i < [urls count]; ++i)
                    results.add (File (nsStringToJuce ([[urls objectAtIndex: i] path])));
            }
        }

        [panel setDelegate: nil];
    }
}

bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return true;
   #endif
}

#else

//==============================================================================
bool FileChooser::isPlatformDialogAvailable()
{
    return false;
}

void FileChooser::showPlatformDialog (Array<File>&,
                                      const String& /*title*/,
                                      const File& /*currentFileOrDirectory*/,
                                      const String& /*filter*/,
                                      bool /*selectsDirectory*/,
                                      bool /*selectsFiles*/,
                                      bool /*isSaveDialogue*/,
                                      bool /*warnAboutOverwritingExistingFiles*/,
                                      bool /*selectMultipleFiles*/,
                                      bool /*treatFilePackagesAsDirs*/,
                                      FilePreviewComponent*)
{
    jassertfalse; //there's no such thing in iOS
}

#endif

} // namespace juce
