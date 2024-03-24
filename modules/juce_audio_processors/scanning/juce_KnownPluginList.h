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
/**
    Manages a list of plugin types.

    This can be easily edited, saved and loaded, and used to create instances of
    the plugin types in it.

    @see PluginListComponent

    @tags{Audio}
*/
class JUCE_API  KnownPluginList   : public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an empty list. */
    KnownPluginList();

    /** Destructor. */
    ~KnownPluginList() override;

    //==============================================================================
    /** Clears the list. */
    void clear();

    /** Adds a type manually from its description. */
    bool addType (const PluginDescription& type);

    /** Removes a type. */
    void removeType (const PluginDescription& type);

    /** Returns the number of types currently in the list. */
    int getNumTypes() const noexcept;

    /** Returns a copy of the current list. */
    Array<PluginDescription> getTypes() const;

    /** Returns the subset of plugin types for a given format. */
    Array<PluginDescription> getTypesForFormat (AudioPluginFormat&) const;

    /** Looks for a type in the list which comes from this file. */
    std::unique_ptr<PluginDescription> getTypeForFile (const String& fileOrIdentifier) const;

    /** Looks for a type in the list which matches a plugin type ID.

        The identifierString parameter must have been created by
        PluginDescription::createIdentifierString().
    */
    std::unique_ptr<PluginDescription> getTypeForIdentifierString (const String& identifierString) const;

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
    /** Adds the plug-in types to a popup menu so that the user can select one.

        Depending on the sort method, it may add sub-menus for categories,
        manufacturers, etc.

        Use getIndexChosenByMenu() to find out the type that was chosen.
    */
    static void addToMenu (PopupMenu& menu, const Array<PluginDescription>& types,
                           SortMethod sortMethod, const String& currentlyTickedPluginID = {});

    /** Converts a menu item index that has been chosen into its index in the list.
        Returns -1 if it's not an ID that was used.
        @see addToMenu
    */
    static int getIndexChosenByMenu (const Array<PluginDescription>& types, int menuResultCode);

    //==============================================================================
    /** Sorts the list. */
    void sort (SortMethod method, bool forwards);

    //==============================================================================
    /** Creates some XML that can be used to store the state of this list. */
    std::unique_ptr<XmlElement> createXml() const;

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
        Array<PluginDescription> plugins;
    };

    /** Creates a PluginTree object representing the list of plug-ins. */
    static std::unique_ptr<PluginTree> createTree (const Array<PluginDescription>& types, SortMethod sortMethod);

    //==============================================================================
    /** Class to define a custom plugin scanner */
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
    void setCustomScanner (std::unique_ptr<CustomScanner> newScanner);

    //==============================================================================
   #ifndef DOXYGEN
    // These methods have been deprecated! When getting the list of plugin types you should instead use
    // the getTypes() method which returns a copy of the internal PluginDescription array and can be accessed
    // in a thread-safe way.
    [[deprecated]] PluginDescription* getType (int index)  noexcept            { return &types.getReference (index); }
    [[deprecated]] const PluginDescription* getType (int index) const noexcept { return &types.getReference (index); }
    [[deprecated]] PluginDescription** begin() noexcept                        { jassertfalse; return nullptr; }
    [[deprecated]] PluginDescription* const* begin() const noexcept            { jassertfalse; return nullptr; }
    [[deprecated]] PluginDescription** end() noexcept                          { jassertfalse; return nullptr; }
    [[deprecated]] PluginDescription* const* end() const noexcept              { jassertfalse; return nullptr; }

    // These methods have been deprecated in favour of their static counterparts. You should call getTypes()
    // to store the plug-in list at a point in time and use it when calling these methods.
    [[deprecated]] void addToMenu (PopupMenu& menu, SortMethod sortMethod, const String& currentlyTickedPluginID = {}) const;
    [[deprecated]] int getIndexChosenByMenu (int menuResultCode) const;
    [[deprecated]] std::unique_ptr<PluginTree> createTree (SortMethod sortMethod) const;
   #endif

private:
    //==============================================================================
    Array<PluginDescription> types;
    StringArray blacklist;
    std::unique_ptr<CustomScanner> scanner;
    CriticalSection scanLock, typesArrayLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnownPluginList)
};

} // namespace juce
