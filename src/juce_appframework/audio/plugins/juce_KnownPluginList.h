/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_KNOWNPLUGINLIST_JUCEHEADER__
#define __JUCE_KNOWNPLUGINLIST_JUCEHEADER__

#include "juce_PluginDescription.h"
#include "juce_AudioPluginFormat.h"
#include "../../events/juce_ChangeBroadcaster.h"
#include "../../gui/components/menus/juce_PopupMenu.h"


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
    /** Creates an empty list.
    */
    KnownPluginList();

    /** Destructor. */
    ~KnownPluginList();

    //==============================================================================
    /** Clears the list. */
    void clear();

    /** Returns the number of types currently in the list.
        @see getType
    */
    int getNumTypes() const throw()                                 { return types.size(); }

    /** Returns one of the types.
        @see getNumTypes
    */
    PluginDescription* getType (const int index) const throw()      { return types [index]; }

    /** Looks for a type in the list which comes from this file.
    */
    PluginDescription* getTypeForFile (const String& fileOrIdentifier) const throw();

    /** Looks for a type in the list which matches a plugin type ID.

        The identifierString parameter must have been created by
        PluginDescription::createIdentifierString().
    */
    PluginDescription* getTypeForIdentifierString (const String& identifierString) const throw();

    /** Adds a type manually from its description. */
    bool addType (const PluginDescription& type);

    /** Removes a type. */
    void removeType (const int index) throw();

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
                         const bool dontRescanIfAlreadyInList,
                         OwnedArray <PluginDescription>& typesFound,
                         AudioPluginFormat& formatToUse);

    /** Returns true if the specified file is already known about and if it
        hasn't been modified since our entry was created.
    */
    bool isListingUpToDate (const String& possiblePluginFileOrIdentifier) const throw();

    /** Scans and adds a bunch of files that might have been dragged-and-dropped.

        If any types are found in the files, their descriptions are returned in the array.
    */
    void scanAndAddDragAndDroppedFiles (const StringArray& filenames,
                                        OwnedArray <PluginDescription>& typesFound);

    //==============================================================================
    /** Sort methods used to change the order of the plugins in the list.
    */
    enum SortMethod
    {
        defaultOrder = 0,
        sortAlphabetically,
        sortByCategory,
        sortByManufacturer,
        sortByFileSystemLocation
    };

    //==============================================================================
    /** Adds all the plugin types to a popup menu so that the user can select one.

        Depending on the sort method, it may add sub-menus for categories,
        manufacturers, etc.

        Use getIndexChosenByMenu() to find out the type that was chosen.
    */
    void addToMenu (PopupMenu& menu,
                    const SortMethod sortMethod) const;

    /** Converts a menu item index that has been chosen into its index in this list.

        Returns -1 if it's not an ID that was used.

        @see addToMenu
    */
    int getIndexChosenByMenu (const int menuResultCode) const;

    //==============================================================================
    /** Sorts the list. */
    void sort (const SortMethod method);

    //==============================================================================
    /** Creates some XML that can be used to store the state of this list.
    */
    XmlElement* createXml() const;

    /** Recreates the state of this list from its stored XML format.
    */
    void recreateFromXml (const XmlElement& xml);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <PluginDescription> types;

    KnownPluginList (const KnownPluginList&);
    const KnownPluginList& operator= (const KnownPluginList&);
};


#endif   // __JUCE_KNOWNPLUGINLIST_JUCEHEADER__
