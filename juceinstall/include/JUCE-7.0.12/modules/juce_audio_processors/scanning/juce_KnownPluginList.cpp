/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

KnownPluginList::KnownPluginList()  {}
KnownPluginList::~KnownPluginList() {}

void KnownPluginList::clear()
{
    ScopedLock lock (typesArrayLock);

    if (! types.isEmpty())
    {
        types.clear();
        sendChangeMessage();
    }
}

int KnownPluginList::getNumTypes() const noexcept
{
    ScopedLock lock (typesArrayLock);
    return types.size();
}

Array<PluginDescription> KnownPluginList::getTypes() const
{
    ScopedLock lock (typesArrayLock);
    return types;
}

Array<PluginDescription> KnownPluginList::getTypesForFormat (AudioPluginFormat& format) const
{
    Array<PluginDescription> result;

    for (auto& d : getTypes())
        if (d.pluginFormatName == format.getName())
            result.add (d);

    return result;
}

std::unique_ptr<PluginDescription> KnownPluginList::getTypeForFile (const String& fileOrIdentifier) const
{
    ScopedLock lock (typesArrayLock);

    for (auto& desc : types)
        if (desc.fileOrIdentifier == fileOrIdentifier)
            return std::make_unique<PluginDescription> (desc);

    return {};
}

std::unique_ptr<PluginDescription> KnownPluginList::getTypeForIdentifierString (const String& identifierString) const
{
    ScopedLock lock (typesArrayLock);

    for (auto& desc : types)
        if (desc.matchesIdentifierString (identifierString))
            return std::make_unique<PluginDescription> (desc);

    return {};
}

bool KnownPluginList::addType (const PluginDescription& type)
{
    {
        ScopedLock lock (typesArrayLock);

        for (auto& desc : types)
        {
            if (desc.isDuplicateOf (type))
            {
                // strange - found a duplicate plugin with different info..
                jassert (desc.name == type.name);
                jassert (desc.isInstrument == type.isInstrument);

                desc = type;
                return false;
            }
        }

        types.insert (0, type);
    }

    sendChangeMessage();
    return true;
}

void KnownPluginList::removeType (const PluginDescription& type)
{
    {
        ScopedLock lock (typesArrayLock);

        for (int i = types.size(); --i >= 0;)
            if (types.getUnchecked (i).isDuplicateOf (type))
                types.remove (i);
    }

    sendChangeMessage();
}

bool KnownPluginList::isListingUpToDate (const String& fileOrIdentifier,
                                         AudioPluginFormat& formatToUse) const
{
    if (getTypeForFile (fileOrIdentifier) == nullptr)
        return false;

    ScopedLock lock (typesArrayLock);

    for (auto& d : types)
        if (d.fileOrIdentifier == fileOrIdentifier && formatToUse.pluginNeedsRescanning (d))
            return false;

    return true;
}

void KnownPluginList::setCustomScanner (std::unique_ptr<CustomScanner> newScanner)
{
    if (scanner != newScanner)
        scanner = std::move (newScanner);
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

        for (auto& d : types)
        {
            if (d.fileOrIdentifier == fileOrIdentifier && d.pluginFormatName == format.getName())
            {
                if (format.pluginNeedsRescanning (d))
                    needsRescanning = true;
                else
                    typesFound.add (new PluginDescription (d));
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

    for (auto* desc : found)
    {
        if (desc == nullptr)
        {
            jassertfalse;
            continue;
        }

        addType (*desc);
        typesFound.add (new PluginDescription (*desc));
    }

    return ! found.isEmpty();
}

void KnownPluginList::scanAndAddDragAndDroppedFiles (AudioPluginFormatManager& formatManager,
                                                     const StringArray& files,
                                                     OwnedArray<PluginDescription>& typesFound)
{
    for (const auto& filenameOrID : files)
    {
        bool found = false;

        for (auto format : formatManager.getFormats())
        {
            if (format->fileMightContainThisPluginType (filenameOrID)
                 && scanAndAddFile (filenameOrID, true, typesFound, *format))
            {
                found = true;
                break;
            }
        }

        if (! found)
        {
            File f (filenameOrID);

            if (f.isDirectory())
            {
                StringArray s;

                for (auto& subFile : f.findChildFiles (File::findFilesAndDirectories, false))
                    s.add (subFile.getFullPathName());

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

    bool operator() (const PluginDescription& first, const PluginDescription& second) const
    {
        int diff = 0;

        switch (method)
        {
            case KnownPluginList::sortByCategory:           diff = first.category.compareNatural (second.category, false); break;
            case KnownPluginList::sortByManufacturer:       diff = first.manufacturerName.compareNatural (second.manufacturerName, false); break;
            case KnownPluginList::sortByFormat:             diff = first.pluginFormatName.compare (second.pluginFormatName); break;
            case KnownPluginList::sortByFileSystemLocation: diff = lastPathPart (first.fileOrIdentifier).compare (lastPathPart (second.fileOrIdentifier)); break;
            case KnownPluginList::sortByInfoUpdateTime:     diff = compare (first.lastInfoUpdateTime, second.lastInfoUpdateTime); break;
            case KnownPluginList::sortAlphabetically:
            case KnownPluginList::defaultOrder:
            default: break;
        }

        if (diff == 0)
            diff = first.name.compareNatural (second.name, false);

        return diff * direction < 0;
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

    KnownPluginList::SortMethod method;
    int direction;
};

void KnownPluginList::sort (const SortMethod method, bool forwards)
{
    if (method != defaultOrder)
    {
        Array<PluginDescription> oldOrder, newOrder;

        {
            ScopedLock lock (typesArrayLock);

            oldOrder.addArray (types);
            std::stable_sort (types.begin(), types.end(), PluginSorter (method, forwards));
            newOrder.addArray (types);
        }

        auto hasOrderChanged = [&]
        {
            for (int i = 0; i < oldOrder.size(); ++i)
                 if (! oldOrder[i].isDuplicateOf (newOrder[i]))
                     return true;

            return false;
        }();

        if (hasOrderChanged)
            sendChangeMessage();
    }
}

//==============================================================================
std::unique_ptr<XmlElement> KnownPluginList::createXml() const
{
    auto e = std::make_unique<XmlElement> ("KNOWNPLUGINS");

    {
        ScopedLock lock (typesArrayLock);

        for (int i = types.size(); --i >= 0;)
            e->prependChildElement (types.getUnchecked (i).createXml().release());
    }

    for (auto& b : blacklist)
        e->createNewChildElement ("BLACKLISTED")->setAttribute ("id", b);

    return e;
}

void KnownPluginList::recreateFromXml (const XmlElement& xml)
{
    clear();
    clearBlacklistedFiles();

    if (xml.hasTagName ("KNOWNPLUGINS"))
    {
        for (auto* e : xml.getChildIterator())
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

    static void buildTreeByFolder (KnownPluginList::PluginTree& tree, const Array<PluginDescription>& allPlugins)
    {
        for (auto& pd : allPlugins)
        {
            auto path = pd.fileOrIdentifier.replaceCharacter ('\\', '/')
                                           .upToLastOccurrenceOf ("/", false, false);

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
            auto& sub = *tree.subFolders.getUnchecked (i);
            optimiseFolders (sub, concatenateName || (tree.subFolders.size() > 1));

            if (sub.plugins.isEmpty())
            {
                for (auto* s : sub.subFolders)
                {
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
                                     const Array<PluginDescription>& sorted,
                                     const KnownPluginList::SortMethod sortMethod)
    {
        String lastType;
        auto current = std::make_unique<KnownPluginList::PluginTree>();

        for (auto& pd : sorted)
        {
            auto thisType = (sortMethod == KnownPluginList::sortByCategory ? pd.category
                                                                           : pd.manufacturerName);

            if (! thisType.containsNonWhitespaceChars())
                thisType = "Other";

            if (! thisType.equalsIgnoreCase (lastType))
            {
                if (current->plugins.size() + current->subFolders.size() > 0)
                {
                    current->folder = lastType;
                    tree.subFolders.add (std::move (current));
                    current = std::make_unique<KnownPluginList::PluginTree>();
                }

                lastType = thisType;
            }

            current->plugins.add (pd);
        }

        if (current->plugins.size() + current->subFolders.size() > 0)
        {
            current->folder = lastType;
            tree.subFolders.add (std::move (current));
        }
    }

    static void addPlugin (KnownPluginList::PluginTree& tree, PluginDescription pd, String path)
    {
       #if JUCE_MAC
        if (path.containsChar (':'))
            path = path.fromFirstOccurrenceOf (":", false, false); // avoid the special AU formatting nonsense on Mac..
       #endif

        if (path.isEmpty())
        {
            tree.plugins.add (pd);
        }
        else
        {
            auto firstSubFolder = path.upToFirstOccurrenceOf ("/", false, false);
            auto remainingPath  = path.fromFirstOccurrenceOf ("/", false, false);

            for (int i = tree.subFolders.size(); --i >= 0;)
            {
                auto& subFolder = *tree.subFolders.getUnchecked (i);

                if (subFolder.folder.equalsIgnoreCase (firstSubFolder))
                {
                    addPlugin (subFolder, pd, remainingPath);
                    return;
                }
            }

            auto* newFolder = new KnownPluginList::PluginTree();
            newFolder->folder = firstSubFolder;
            tree.subFolders.add (newFolder);
            addPlugin (*newFolder, pd, remainingPath);
        }
    }

    static bool containsDuplicateNames (const Array<PluginDescription>& plugins, const String& name)
    {
        int matches = 0;

        for (auto& p : plugins)
            if (p.name == name && ++matches > 1)
                return true;

        return false;
    }

    static bool addToMenu (const KnownPluginList::PluginTree& tree, PopupMenu& m,
                           const Array<PluginDescription>& allPlugins,
                           const String& currentlyTickedPluginID)
    {
        bool isTicked = false;

        for (auto* sub : tree.subFolders)
        {
            PopupMenu subMenu;
            auto isItemTicked = addToMenu (*sub, subMenu, allPlugins, currentlyTickedPluginID);
            isTicked = isTicked || isItemTicked;

            m.addSubMenu (sub->folder, subMenu, true, nullptr, isItemTicked, 0);
        }

        auto getPluginMenuIndex = [&] (const PluginDescription& d)
        {
            int i = 0;

            for (auto& p : allPlugins)
            {
                if (p.isDuplicateOf (d))
                    return i + menuIdBase;

                ++i;
            }

            return 0;
        };

        for (auto& plugin : tree.plugins)
        {
            auto name = plugin.name;

            if (containsDuplicateNames (tree.plugins, name))
                name << " (" << plugin.pluginFormatName << ')';

            auto isItemTicked = plugin.matchesIdentifierString (currentlyTickedPluginID);
            isTicked = isTicked || isItemTicked;

            m.addItem (getPluginMenuIndex (plugin), name, true, isItemTicked);
        }

        return isTicked;
    }
};

std::unique_ptr<KnownPluginList::PluginTree> KnownPluginList::createTree (const Array<PluginDescription>& types, SortMethod sortMethod)
{
    Array<PluginDescription> sorted;
    sorted.addArray (types);

    std::stable_sort (sorted.begin(), sorted.end(), PluginSorter (sortMethod, true));

    auto tree = std::make_unique<PluginTree>();

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
        for (auto& p : sorted)
            tree->plugins.add (p);
    }

    return tree;
}

//==============================================================================
void KnownPluginList::addToMenu (PopupMenu& menu, const Array<PluginDescription>& types, SortMethod sortMethod,
                                 const String& currentlyTickedPluginID)
{
    auto tree = createTree (types, sortMethod);
    PluginTreeUtils::addToMenu (*tree, menu, types, currentlyTickedPluginID);
}

int KnownPluginList::getIndexChosenByMenu (const Array<PluginDescription>& types, int menuResultCode)
{
    auto i = menuResultCode - PluginTreeUtils::menuIdBase;
    return isPositiveAndBelow (i, types.size()) ? i : -1;
}

//==============================================================================
KnownPluginList::CustomScanner::CustomScanner() {}
KnownPluginList::CustomScanner::~CustomScanner() {}

void KnownPluginList::CustomScanner::scanFinished() {}

bool KnownPluginList::CustomScanner::shouldExit() const noexcept
{
    if (auto* job = ThreadPoolJob::getCurrentThreadPoolJob())
        return job->shouldExit();

    return false;
}

//==============================================================================
void KnownPluginList::addToMenu (PopupMenu& menu, SortMethod sortMethod, const String& currentlyTickedPluginID) const
{
    addToMenu (menu, getTypes(), sortMethod, currentlyTickedPluginID);
}

int KnownPluginList::getIndexChosenByMenu (int menuResultCode) const
{
    return getIndexChosenByMenu (getTypes(), menuResultCode);
}

std::unique_ptr<KnownPluginList::PluginTree> KnownPluginList::createTree (const SortMethod sortMethod) const
{
    return createTree (getTypes(), sortMethod);
}


} // namespace juce
