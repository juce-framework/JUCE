/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

KnownPluginList::KnownPluginList()  {}
KnownPluginList::~KnownPluginList() {}

void KnownPluginList::clear()
{
    ScopedLock lock (typesArrayLock);

    if (types.size() > 0)
    {
        types.clear();
        sendChangeMessage();
    }
}

PluginDescription* KnownPluginList::getTypeForFile (const String& fileOrIdentifier) const
{
    ScopedLock lock (typesArrayLock);

    for (int i = 0; i < types.size(); ++i)
        if (types.getUnchecked(i)->fileOrIdentifier == fileOrIdentifier)
            return types.getUnchecked(i);

    return nullptr;
}

PluginDescription* KnownPluginList::getTypeForIdentifierString (const String& identifierString) const
{
    ScopedLock lock (typesArrayLock);

    for (int i = 0; i < types.size(); ++i)
        if (types.getUnchecked(i)->matchesIdentifierString (identifierString))
            return types.getUnchecked(i);

    return nullptr;
}

bool KnownPluginList::addType (const PluginDescription& type)
{
    {
        ScopedLock lock (typesArrayLock);

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

        types.insert (0, new PluginDescription (type));
    }

    sendChangeMessage();
    return true;
}

void KnownPluginList::removeType (const int index)
{
    {
        ScopedLock lock (typesArrayLock);

        types.remove (index);
    }

    sendChangeMessage();
}

bool KnownPluginList::isListingUpToDate (const String& fileOrIdentifier,
                                         AudioPluginFormat& formatToUse) const
{
    if (getTypeForFile (fileOrIdentifier) == nullptr)
        return false;

    ScopedLock lock (typesArrayLock);

    for (int i = types.size(); --i >= 0;)
    {
        const PluginDescription* const d = types.getUnchecked(i);

        if (d->fileOrIdentifier == fileOrIdentifier
             && formatToUse.pluginNeedsRescanning (*d))
            return false;
    }

    return true;
}

void KnownPluginList::setCustomScanner (CustomScanner* newScanner)
{
    scanner = newScanner;
}

bool KnownPluginList::scanAndAddFile (const String& fileOrIdentifier,
                                      const bool dontRescanIfAlreadyInList,
                                      OwnedArray<PluginDescription>& typesFound,
                                      AudioPluginFormat& format)
{
    const ScopedLock sl (scanLock);

    if (dontRescanIfAlreadyInList
         && getTypeForFile (fileOrIdentifier) != nullptr)
    {
        bool needsRescanning = false;

        ScopedLock lock (typesArrayLock);

        for (int i = types.size(); --i >= 0;)
        {
            const PluginDescription* const d = types.getUnchecked(i);

            if (d->fileOrIdentifier == fileOrIdentifier && d->pluginFormatName == format.getName())
            {
                if (format.pluginNeedsRescanning (*d))
                    needsRescanning = true;
                else
                    typesFound.add (new PluginDescription (*d));
            }
        }

        if (! needsRescanning)
            return false;
    }

    if (blacklist.contains (fileOrIdentifier))
        return false;

    OwnedArray<PluginDescription> found;

    {
        const ScopedUnlock sl2 (scanLock);

        if (scanner != nullptr)
        {
            if (! scanner->findPluginTypesFor (format, found, fileOrIdentifier))
                addToBlacklist (fileOrIdentifier);
        }
        else
        {
            format.findAllTypesForFile (found, fileOrIdentifier);
        }
    }

    for (int i = 0; i < found.size(); ++i)
    {
        PluginDescription* const desc = found.getUnchecked(i);
        jassert (desc != nullptr);

        addType (*desc);
        typesFound.add (new PluginDescription (*desc));
    }

    return found.size() > 0;
}

void KnownPluginList::scanAndAddDragAndDroppedFiles (AudioPluginFormatManager& formatManager,
                                                     const StringArray& files,
                                                     OwnedArray<PluginDescription>& typesFound)
{
    for (int i = 0; i < files.size(); ++i)
    {
        const String filenameOrID (files[i]);
        bool found = false;

        for (int j = 0; j < formatManager.getNumFormats(); ++j)
        {
            AudioPluginFormat* const format = formatManager.getFormat (j);

            if (format->fileMightContainThisPluginType (filenameOrID)
                 && scanAndAddFile (filenameOrID, true, typesFound, *format))
            {
                found = true;
                break;
            }
        }

        if (! found)
        {
            const File f (filenameOrID);

            if (f.isDirectory())
            {
                StringArray s;

                {
                    Array<File> subFiles;
                    f.findChildFiles (subFiles, File::findFilesAndDirectories, false);

                    for (int j = 0; j < subFiles.size(); ++j)
                        s.add (subFiles.getReference(j).getFullPathName());
                }

                scanAndAddDragAndDroppedFiles (formatManager, s, typesFound);
            }
        }
    }

    scanFinished();
}

void KnownPluginList::scanFinished()
{
    if (scanner != nullptr)
        scanner->scanFinished();
}

const StringArray& KnownPluginList::getBlacklistedFiles() const
{
    return blacklist;
}

void KnownPluginList::addToBlacklist (const String& pluginID)
{
    if (! blacklist.contains (pluginID))
    {
        blacklist.add (pluginID);
        sendChangeMessage();
    }
}

void KnownPluginList::removeFromBlacklist (const String& pluginID)
{
    const int index = blacklist.indexOf (pluginID);

    if (index >= 0)
    {
        blacklist.remove (index);
        sendChangeMessage();
    }
}

void KnownPluginList::clearBlacklistedFiles()
{
    if (blacklist.size() > 0)
    {
        blacklist.clear();
        sendChangeMessage();
    }
}

//==============================================================================
struct PluginSorter
{
    PluginSorter (KnownPluginList::SortMethod sortMethod, bool forwards) noexcept
        : method (sortMethod), direction (forwards ? 1 : -1) {}

    int compareElements (const PluginDescription* const first,
                         const PluginDescription* const second) const
    {
        int diff = 0;

        switch (method)
        {
            case KnownPluginList::sortByCategory:           diff = first->category.compareNatural (second->category); break;
            case KnownPluginList::sortByManufacturer:       diff = first->manufacturerName.compareNatural (second->manufacturerName); break;
            case KnownPluginList::sortByFormat:             diff = first->pluginFormatName.compare (second->pluginFormatName); break;
            case KnownPluginList::sortByFileSystemLocation: diff = lastPathPart (first->fileOrIdentifier).compare (lastPathPart (second->fileOrIdentifier)); break;
            case KnownPluginList::sortByInfoUpdateTime:     diff = compare (first->lastInfoUpdateTime, second->lastInfoUpdateTime); break;
            default: break;
        }

        if (diff == 0)
            diff = first->name.compareNatural (second->name);

        return diff * direction;
    }

private:
    static String lastPathPart (const String& path)
    {
        return path.replaceCharacter ('\\', '/').upToLastOccurrenceOf ("/", false, false);
    }

    static int compare (Time a, Time b) noexcept
    {
        if (a < b)   return -1;
        if (b < a)   return 1;

        return 0;
    }

    const KnownPluginList::SortMethod method;
    const int direction;

    JUCE_DECLARE_NON_COPYABLE (PluginSorter)
};

void KnownPluginList::sort (const SortMethod method, bool forwards)
{
    if (method != defaultOrder)
    {
        Array<PluginDescription*> oldOrder, newOrder;

        {
            ScopedLock lock (typesArrayLock);

            oldOrder.addArray (types);

            PluginSorter sorter (method, forwards);
            types.sort (sorter, true);

            newOrder.addArray (types);
        }

        if (oldOrder != newOrder)
            sendChangeMessage();
    }
}

//==============================================================================
XmlElement* KnownPluginList::createXml() const
{
    XmlElement* const e = new XmlElement ("KNOWNPLUGINS");

    {
        ScopedLock lock (typesArrayLock);

        for (int i = types.size(); --i >= 0;)
            e->prependChildElement (types.getUnchecked(i)->createXml());
    }

    for (int i = 0; i < blacklist.size(); ++i)
        e->createNewChildElement ("BLACKLISTED")->setAttribute ("id", blacklist[i]);

    return e;
}

void KnownPluginList::recreateFromXml (const XmlElement& xml)
{
    clear();
    clearBlacklistedFiles();

    if (xml.hasTagName ("KNOWNPLUGINS"))
    {
        forEachXmlChildElement (xml, e)
        {
            PluginDescription info;

            if (e->hasTagName ("BLACKLISTED"))
                blacklist.add (e->getStringAttribute ("id"));
            else if (info.loadFromXml (*e))
                addType (info);
        }
    }
}

//==============================================================================
struct PluginTreeUtils
{
    enum { menuIdBase = 0x324503f4 };

    static void buildTreeByFolder (KnownPluginList::PluginTree& tree, const Array<PluginDescription*>& allPlugins)
    {
        for (int i = 0; i < allPlugins.size(); ++i)
        {
            PluginDescription* const pd = allPlugins.getUnchecked (i);

            String path (pd->fileOrIdentifier.replaceCharacter ('\\', '/')
                                             .upToLastOccurrenceOf ("/", false, false));

            if (path.substring (1, 2) == ":")
                path = path.substring (2);

            addPlugin (tree, pd, path);
        }

        optimiseFolders (tree, false);
    }

    static void optimiseFolders (KnownPluginList::PluginTree& tree, bool concatenateName)
    {
        for (int i = tree.subFolders.size(); --i >= 0;)
        {
            KnownPluginList::PluginTree& sub = *tree.subFolders.getUnchecked(i);
            optimiseFolders (sub, concatenateName || (tree.subFolders.size() > 1));

            if (sub.plugins.size() == 0)
            {
                for (int j = 0; j < sub.subFolders.size(); ++j)
                {
                    KnownPluginList::PluginTree* const s = sub.subFolders.getUnchecked(j);

                    if (concatenateName)
                        s->folder = sub.folder + "/" + s->folder;

                    tree.subFolders.add (s);
                }

                sub.subFolders.clear (false);
                tree.subFolders.remove (i);
            }
        }
    }

    static void buildTreeByCategory (KnownPluginList::PluginTree& tree,
                                     const Array<PluginDescription*>& sorted,
                                     const KnownPluginList::SortMethod sortMethod)
    {
        String lastType;
        ScopedPointer<KnownPluginList::PluginTree> current (new KnownPluginList::PluginTree());

        for (int i = 0; i < sorted.size(); ++i)
        {
            const PluginDescription* const pd = sorted.getUnchecked(i);
            String thisType (sortMethod == KnownPluginList::sortByCategory ? pd->category
                                                                           : pd->manufacturerName);

            if (! thisType.containsNonWhitespaceChars())
                thisType = "Other";

            if (thisType != lastType)
            {
                if (current->plugins.size() + current->subFolders.size() > 0)
                {
                    current->folder = lastType;
                    tree.subFolders.add (current.release());
                    current = new KnownPluginList::PluginTree();
                }

                lastType = thisType;
            }

            current->plugins.add (pd);
        }

        if (current->plugins.size() + current->subFolders.size() > 0)
        {
            current->folder = lastType;
            tree.subFolders.add (current.release());
        }
    }

    static void addPlugin (KnownPluginList::PluginTree& tree, PluginDescription* const pd, String path)
    {
        if (path.isEmpty())
        {
            tree.plugins.add (pd);
        }
        else
        {
           #if JUCE_MAC
            if (path.containsChar (':'))
                path = path.fromFirstOccurrenceOf (":", false, false); // avoid the special AU formatting nonsense on Mac..
           #endif

            const String firstSubFolder (path.upToFirstOccurrenceOf ("/", false, false));
            const String remainingPath  (path.fromFirstOccurrenceOf ("/", false, false));

            for (int i = tree.subFolders.size(); --i >= 0;)
            {
                KnownPluginList::PluginTree& subFolder = *tree.subFolders.getUnchecked(i);

                if (subFolder.folder.equalsIgnoreCase (firstSubFolder))
                {
                    addPlugin (subFolder, pd, remainingPath);
                    return;
                }
            }

            KnownPluginList::PluginTree* const newFolder = new KnownPluginList::PluginTree();
            newFolder->folder = firstSubFolder;
            tree.subFolders.add (newFolder);
            addPlugin (*newFolder, pd, remainingPath);
        }
    }

    static bool containsDuplicateNames (const Array<const PluginDescription*>& plugins, const String& name)
    {
        int matches = 0;

        for (int i = 0; i < plugins.size(); ++i)
            if (plugins.getUnchecked(i)->name == name)
                if (++matches > 1)
                    return true;

        return false;
    }

    static bool addToMenu (const KnownPluginList::PluginTree& tree, PopupMenu& m,
                           const OwnedArray<PluginDescription>& allPlugins,
                           const String& currentlyTickedPluginID)
    {
        bool isTicked = false;

        for (int i = 0; i < tree.subFolders.size(); ++i)
        {
            const KnownPluginList::PluginTree& sub = *tree.subFolders.getUnchecked(i);

            PopupMenu subMenu;
            const bool isItemTicked = addToMenu (sub, subMenu, allPlugins, currentlyTickedPluginID);
            isTicked = isTicked || isItemTicked;

            m.addSubMenu (sub.folder, subMenu, true, nullptr, isItemTicked, 0);
        }

        for (int i = 0; i < tree.plugins.size(); ++i)
        {
            const PluginDescription* const plugin = tree.plugins.getUnchecked(i);

            String name (plugin->name);

            if (containsDuplicateNames (tree.plugins, name))
                name << " (" << plugin->pluginFormatName << ')';

            const bool isItemTicked = plugin->matchesIdentifierString (currentlyTickedPluginID);
            isTicked = isTicked || isItemTicked;

            m.addItem (allPlugins.indexOf (plugin) + menuIdBase, name, true, isItemTicked);
        }

        return isTicked;
    }
};

KnownPluginList::PluginTree* KnownPluginList::createTree (const SortMethod sortMethod) const
{
    Array<PluginDescription*> sorted;

    {
        ScopedLock lock (typesArrayLock);
        PluginSorter sorter (sortMethod, true);

        for (int i = 0; i < types.size(); ++i)
            sorted.addSorted (sorter, types.getUnchecked(i));
    }

    PluginTree* tree = new PluginTree();

    if (sortMethod == sortByCategory || sortMethod == sortByManufacturer || sortMethod == sortByFormat)
    {
        PluginTreeUtils::buildTreeByCategory (*tree, sorted, sortMethod);
    }
    else if (sortMethod == sortByFileSystemLocation)
    {
        PluginTreeUtils::buildTreeByFolder (*tree, sorted);
    }
    else
    {
        for (int i = 0; i < sorted.size(); ++i)
            tree->plugins.add (sorted.getUnchecked(i));
    }

    return tree;
}

//==============================================================================
void KnownPluginList::addToMenu (PopupMenu& menu, const SortMethod sortMethod,
                                 const String& currentlyTickedPluginID) const
{
    ScopedPointer<PluginTree> tree (createTree (sortMethod));
    PluginTreeUtils::addToMenu (*tree, menu, types, currentlyTickedPluginID);
}

int KnownPluginList::getIndexChosenByMenu (const int menuResultCode) const
{
    const int i = menuResultCode - PluginTreeUtils::menuIdBase;
    return isPositiveAndBelow (i, types.size()) ? i : -1;
}

//==============================================================================
KnownPluginList::CustomScanner::CustomScanner() {}
KnownPluginList::CustomScanner::~CustomScanner() {}

void KnownPluginList::CustomScanner::scanFinished() {}

bool KnownPluginList::CustomScanner::shouldExit() const noexcept
{
    if (ThreadPoolJob* job = ThreadPoolJob::getCurrentThreadPoolJob())
        return job->shouldExit();

    return false;
}
