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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_KnownPluginList.h"
#include "juce_AudioPluginFormatManager.h"


//==============================================================================
KnownPluginList::KnownPluginList()
{
}

KnownPluginList::~KnownPluginList()
{
}

void KnownPluginList::clear()
{
    if (types.size() > 0)
    {
        types.clear();
        sendChangeMessage (this);
    }
}

PluginDescription* KnownPluginList::getTypeForFile (const String& fileOrIdentifier) const throw()
{
    for (int i = 0; i < types.size(); ++i)
        if (types.getUnchecked(i)->fileOrIdentifier == fileOrIdentifier)
            return types.getUnchecked(i);

    return 0;
}

PluginDescription* KnownPluginList::getTypeForIdentifierString (const String& identifierString) const throw()
{
    for (int i = 0; i < types.size(); ++i)
        if (types.getUnchecked(i)->createIdentifierString() == identifierString)
            return types.getUnchecked(i);

    return 0;
}

bool KnownPluginList::addType (const PluginDescription& type)
{
    for (int i = types.size(); --i >= 0;)
    {
        if (types.getUnchecked(i)->isDuplicateOf (type))
        {
            // strange - found a duplicate plugin with different info..
            jassert (types.getUnchecked(i)->name == type.name);
            jassert (types.getUnchecked(i)->isInstrument == type.isInstrument);

            *types.getUnchecked(i) = type;
            return false;
        }
    }

    types.add (new PluginDescription (type));
    sendChangeMessage (this);
    return true;
}

void KnownPluginList::removeType (const int index) throw()
{
    types.remove (index);
    sendChangeMessage (this);
}

static Time getFileModTime (const String& fileOrIdentifier) throw()
{
    if (fileOrIdentifier.startsWithChar (T('/'))
        || fileOrIdentifier[1] == T(':'))
    {
        return File (fileOrIdentifier).getLastModificationTime();
    }

    return Time (0);
}

static bool timesAreDifferent (const Time& t1, const Time& t2) throw()
{
    return t1 != t2 || t1 == Time (0);
}

bool KnownPluginList::isListingUpToDate (const String& fileOrIdentifier) const throw()
{
    if (getTypeForFile (fileOrIdentifier) == 0)
        return false;

    for (int i = types.size(); --i >= 0;)
    {
        const PluginDescription* const d = types.getUnchecked(i);

        if (d->fileOrIdentifier == fileOrIdentifier
             && timesAreDifferent (d->lastFileModTime, getFileModTime (fileOrIdentifier)))
        {
            return false;
        }
    }

    return true;
}

bool KnownPluginList::scanAndAddFile (const String& fileOrIdentifier,
                                      const bool dontRescanIfAlreadyInList,
                                      OwnedArray <PluginDescription>& typesFound,
                                      AudioPluginFormat& format)
{
    bool addedOne = false;

    if (dontRescanIfAlreadyInList
         && getTypeForFile (fileOrIdentifier) != 0)
    {
        bool needsRescanning = false;

        for (int i = types.size(); --i >= 0;)
        {
            const PluginDescription* const d = types.getUnchecked(i);

            if (d->fileOrIdentifier == fileOrIdentifier)
            {
                if (timesAreDifferent (d->lastFileModTime, getFileModTime (fileOrIdentifier)))
                    needsRescanning = true;
                else
                    typesFound.add (new PluginDescription (*d));
            }
        }

        if (! needsRescanning)
            return false;
    }

    OwnedArray <PluginDescription> found;
    format.findAllTypesForFile (found, fileOrIdentifier);

    for (int i = 0; i < found.size(); ++i)
    {
        PluginDescription* const desc = found.getUnchecked(i);
        jassert (desc != 0);

        if (addType (*desc))
            addedOne = true;

        typesFound.add (new PluginDescription (*desc));
    }

    return addedOne;
}

void KnownPluginList::scanAndAddDragAndDroppedFiles (const StringArray& files,
                                                     OwnedArray <PluginDescription>& typesFound)
{
    for (int i = 0; i < files.size(); ++i)
    {
        bool loaded = false;

        for (int j = 0; j < AudioPluginFormatManager::getInstance()->getNumFormats(); ++j)
        {
            AudioPluginFormat* const format = AudioPluginFormatManager::getInstance()->getFormat (j);

            if (scanAndAddFile (files[i], true, typesFound, *format))
                loaded = true;
        }

        if (! loaded)
        {
            const File f (files[i]);

            if (f.isDirectory())
            {
                StringArray s;

                {
                    OwnedArray <File> subFiles;
                    f.findChildFiles (subFiles, File::findFilesAndDirectories, false);

                    for (int j = 0; j < subFiles.size(); ++j)
                        s.add (subFiles.getUnchecked (j)->getFullPathName());
                }

                scanAndAddDragAndDroppedFiles (s, typesFound);
            }
        }
    }
}

//==============================================================================
class PluginSorter
{
public:
    KnownPluginList::SortMethod method;

    PluginSorter() throw() {}

    int compareElements (const PluginDescription* const first,
                         const PluginDescription* const second) const throw()
    {
        int diff = 0;

        if (method == KnownPluginList::sortByCategory)
            diff = first->category.compareLexicographically (second->category);
        else if (method == KnownPluginList::sortByManufacturer)
            diff = first->manufacturerName.compareLexicographically (second->manufacturerName);
        else if (method == KnownPluginList::sortByFileSystemLocation)
            diff = first->fileOrIdentifier.replaceCharacter (T('\\'), T('/'))
                                          .upToLastOccurrenceOf (T("/"), false, false)
                                          .compare (second->fileOrIdentifier.replaceCharacter (T('\\'), T('/'))
                                                                            .upToLastOccurrenceOf (T("/"), false, false));

        if (diff == 0)
            diff = first->name.compareLexicographically (second->name);

        return diff;
    }
};

void KnownPluginList::sort (const SortMethod method)
{
    if (method != defaultOrder)
    {
        PluginSorter sorter;
        sorter.method = method;
        types.sort (sorter, true);

        sendChangeMessage (this);
    }
}

//==============================================================================
XmlElement* KnownPluginList::createXml() const
{
    XmlElement* const e = new XmlElement (T("KNOWNPLUGINS"));

    for (int i = 0; i < types.size(); ++i)
        e->addChildElement (types.getUnchecked(i)->createXml());

    return e;
}

void KnownPluginList::recreateFromXml (const XmlElement& xml)
{
    clear();

    if (xml.hasTagName (T("KNOWNPLUGINS")))
    {
        forEachXmlChildElement (xml, e)
        {
            PluginDescription info;

            if (info.loadFromXml (*e))
                addType (info);
        }
    }
}

//==============================================================================
const int menuIdBase = 0x324503f4;

// This is used to turn a bunch of paths into a nested menu structure.
struct PluginFilesystemTree
{
private:
    String folder;
    OwnedArray <PluginFilesystemTree> subFolders;
    Array <PluginDescription*> plugins;

    void addPlugin (PluginDescription* const pd, const String& path)
    {
        if (path.isEmpty())
        {
            plugins.add (pd);
        }
        else
        {
            const String firstSubFolder (path.upToFirstOccurrenceOf (T("/"), false, false));
            const String remainingPath (path.fromFirstOccurrenceOf (T("/"), false, false));

            for (int i = subFolders.size(); --i >= 0;)
            {
                if (subFolders.getUnchecked(i)->folder.equalsIgnoreCase (firstSubFolder))
                {
                    subFolders.getUnchecked(i)->addPlugin (pd, remainingPath);
                    return;
                }
            }

            PluginFilesystemTree* const newFolder = new PluginFilesystemTree();
            newFolder->folder = firstSubFolder;
            subFolders.add (newFolder);

            newFolder->addPlugin (pd, remainingPath);
        }
    }

    // removes any deeply nested folders that don't contain any actual plugins
    void optimise()
    {
        for (int i = subFolders.size(); --i >= 0;)
        {
            PluginFilesystemTree* const sub = subFolders.getUnchecked(i);

            sub->optimise();

            if (sub->plugins.size() == 0)
            {
                for (int j = 0; j < sub->subFolders.size(); ++j)
                    subFolders.add (sub->subFolders.getUnchecked(j));

                sub->subFolders.clear (false);
                subFolders.remove (i);
            }
        }
    }

public:
    void buildTree (const Array <PluginDescription*>& allPlugins)
    {
        for (int i = 0; i < allPlugins.size(); ++i)
        {
            String path (allPlugins.getUnchecked(i)
                            ->fileOrIdentifier.replaceCharacter (T('\\'), T('/'))
                                              .upToLastOccurrenceOf (T("/"), false, false));

            if (path.substring (1, 2) == T(":"))
                path = path.substring (2);

            addPlugin (allPlugins.getUnchecked(i), path);
        }

        optimise();
    }

    void addToMenu (PopupMenu& m, const OwnedArray <PluginDescription>& allPlugins) const
    {
        int i;
        for (i = 0; i < subFolders.size(); ++i)
        {
            const PluginFilesystemTree* const sub = subFolders.getUnchecked(i);

            PopupMenu subMenu;
            sub->addToMenu (subMenu, allPlugins);

#if JUCE_MAC
            // avoid the special AU formatting nonsense on Mac..
            m.addSubMenu (sub->folder.fromFirstOccurrenceOf (T(":"), false, false), subMenu);
#else
            m.addSubMenu (sub->folder, subMenu);
#endif
        }

        for (i = 0; i < plugins.size(); ++i)
        {
            PluginDescription* const plugin = plugins.getUnchecked(i);

            m.addItem (allPlugins.indexOf (plugin) + menuIdBase,
                       plugin->name, true, false);
        }
    }
};

//==============================================================================
void KnownPluginList::addToMenu (PopupMenu& menu, const SortMethod sortMethod) const
{
    Array <PluginDescription*> sorted;

    {
        PluginSorter sorter;
        sorter.method = sortMethod;

        for (int i = 0; i < types.size(); ++i)
            sorted.addSorted (sorter, types.getUnchecked(i));
    }

    if (sortMethod == sortByCategory
         || sortMethod == sortByManufacturer)
    {
        String lastSubMenuName;
        PopupMenu sub;

        for (int i = 0; i < sorted.size(); ++i)
        {
            const PluginDescription* const pd = sorted.getUnchecked(i);
            String thisSubMenuName (sortMethod == sortByCategory ? pd->category
                                                                 : pd->manufacturerName);

            if (! thisSubMenuName.containsNonWhitespaceChars())
                thisSubMenuName = T("Other");

            if (thisSubMenuName != lastSubMenuName)
            {
                if (sub.getNumItems() > 0)
                {
                    menu.addSubMenu (lastSubMenuName, sub);
                    sub.clear();
                }

                lastSubMenuName = thisSubMenuName;
            }

            sub.addItem (types.indexOf (pd) + menuIdBase, pd->name, true, false);
        }

        if (sub.getNumItems() > 0)
            menu.addSubMenu (lastSubMenuName, sub);
    }
    else if (sortMethod == sortByFileSystemLocation)
    {
        PluginFilesystemTree root;
        root.buildTree (sorted);
        root.addToMenu (menu, types);
    }
    else
    {
        for (int i = 0; i < sorted.size(); ++i)
        {
            const PluginDescription* const pd = sorted.getUnchecked(i);
            menu.addItem (types.indexOf (pd) + menuIdBase, pd->name, true, false);
        }
    }
}

int KnownPluginList::getIndexChosenByMenu (const int menuResultCode) const
{
    const int i = menuResultCode - menuIdBase;

    return (((unsigned int) i) < (unsigned int) types.size()) ? i : -1;
}

END_JUCE_NAMESPACE
