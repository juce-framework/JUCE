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
/**
    Manages a list of plugin types.

    This can be easily edited, saved and loaded, and used to create instances of
    the plugin types in it.

    @see PluginListComponent
*/
class JUCE_API  KnownPluginList   : public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an empty list. */
    KnownPluginList();

    /** Destructor. */
    ~KnownPluginList();

    //==============================================================================
    /** Clears the list. */
    void clear();

    /** Returns the number of types currently in the list.
        @see getType
    */
    int getNumTypes() const noexcept                                { return types.size(); }

    /** Returns one of the types.
        @see getNumTypes
    */
    PluginDescription* getType (int index) const noexcept           { return types [index]; }

    /** Type iteration. */
    PluginDescription** begin() const noexcept                      { return types.begin(); }
    /** Type iteration. */
    PluginDescription** end() const noexcept                        { return types.end(); }

    /** Looks for a type in the list which comes from this file. */
    PluginDescription* getTypeForFile (const String& fileOrIdentifier) const;

    /** Looks for a type in the list which matches a plugin type ID.

        The identifierString parameter must have been created by
        PluginDescription::createIdentifierString().
    */
    PluginDescription* getTypeForIdentifierString (const String& identifierString) const;

    /** Adds a type manually from its description. */
    bool addType (const PluginDescription& type);

    /** Removes a type. */
    void removeType (int index);

    /** Looks for all types that can be loaded from a given file, and adds them
        to the list.

        If dontRescanIfAlreadyInList is true, then the file will only be loaded and
        re-tested if it's not already in the list, or if the file's modification
        time has changed since the list was created. If dontRescanIfAlreadyInList is
        false, the file will always be reloaded and tested.

        Returns true if any new types were added, and all the types found in this
        file (even if it was already known and hasn't been re-scanned) get returned
        in the array.
    */
    bool scanAndAddFile (const String& possiblePluginFileOrIdentifier,
                         bool dontRescanIfAlreadyInList,
                         OwnedArray<PluginDescription>& typesFound,
                         AudioPluginFormat& formatToUse);

    /** Tells a custom scanner that a scan has finished, and it can release any resources. */
    void scanFinished();

    /** Returns true if the specified file is already known about and if it
        hasn't been modified since our entry was created.
    */
    bool isListingUpToDate (const String& possiblePluginFileOrIdentifier,
                            AudioPluginFormat& formatToUse) const;

    /** Scans and adds a bunch of files that might have been dragged-and-dropped.
        If any types are found in the files, their descriptions are returned in the array.
    */
    void scanAndAddDragAndDroppedFiles (AudioPluginFormatManager& formatManager,
                                        const StringArray& filenames,
                                        OwnedArray<PluginDescription>& typesFound);

    //==============================================================================
    /** Returns the list of blacklisted files. */
    const StringArray& getBlacklistedFiles() const;

    /** Adds a plugin ID to the black-list. */
    void addToBlacklist (const String& pluginID);

    /** Removes a plugin ID from the black-list. */
    void removeFromBlacklist (const String& pluginID);

    /** Clears all the blacklisted files. */
    void clearBlacklistedFiles();

    //==============================================================================
    /** Sort methods used to change the order of the plugins in the list.
    */
    enum SortMethod
    {
        defaultOrder = 0,
        sortAlphabetically,
        sortByCategory,
        sortByManufacturer,
        sortByFormat,
        sortByFileSystemLocation,
        sortByInfoUpdateTime
    };

    //==============================================================================
    /** Adds all the plugin types to a popup menu so that the user can select one.

        Depending on the sort method, it may add sub-menus for categories,
        manufacturers, etc.

        Use getIndexChosenByMenu() to find out the type that was chosen.
    */
    void addToMenu (PopupMenu& menu, SortMethod sortMethod,
                    const String& currentlyTickedPluginID = String()) const;

    /** Converts a menu item index that has been chosen into its index in this list.
        Returns -1 if it's not an ID that was used.
        @see addToMenu
    */
    int getIndexChosenByMenu (int menuResultCode) const;

    //==============================================================================
    /** Sorts the list. */
    void sort (SortMethod method, bool forwards);

    //==============================================================================
    /** Creates some XML that can be used to store the state of this list. */
    XmlElement* createXml() const;

    /** Recreates the state of this list from its stored XML format. */
    void recreateFromXml (const XmlElement& xml);

    //==============================================================================
    /** A structure that recursively holds a tree of plugins.
        @see KnownPluginList::createTree()
    */
    struct PluginTree
    {
        String folder; /**< The name of this folder in the tree */
        OwnedArray<PluginTree> subFolders;
        Array<const PluginDescription*> plugins;
    };

    /** Creates a PluginTree object containing all the known plugins. */
    PluginTree* createTree (const SortMethod sortMethod) const;

    //==============================================================================
    class CustomScanner
    {
    public:
        CustomScanner();
        virtual ~CustomScanner();

        /** Attempts to load the given file and find a list of plugins in it.
            @returns true if the plugin loaded, false if it crashed
        */
        virtual bool findPluginTypesFor (AudioPluginFormat& format,
                                         OwnedArray<PluginDescription>& result,
                                         const String& fileOrIdentifier) = 0;

        /** Called when a scan has finished, to allow clean-up of resources. */
        virtual void scanFinished();

        /** Returns true if the current scan should be abandoned.
            Any blocking methods should check this value repeatedly and return if
            if becomes true.
        */
        bool shouldExit() const noexcept;
    };

    /** Supplies a custom scanner to be used in future scans.
        The KnownPluginList will take ownership of the object passed in.
    */
    void setCustomScanner (CustomScanner*);

private:
    //==============================================================================
    OwnedArray<PluginDescription> types;
    StringArray blacklist;
    ScopedPointer<CustomScanner> scanner;
    CriticalSection scanLock, typesArrayLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnownPluginList)
};

} // namespace juce
