/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce.h"
#include "juce_KnownPluginList.h"


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

PluginDescription* KnownPluginList::getTypeForFile (const File& file) const throw()
{
    for (int i = 0; i < types.size(); ++i)
        if (types.getUnchecked(i)->file == file)
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

bool KnownPluginList::isListingUpToDate (const File& possiblePluginFile) const throw()
{
    if (getTypeForFile (possiblePluginFile) == 0)
        return false;

    for (int i = types.size(); --i >= 0;)
    {
        const PluginDescription* const d = types.getUnchecked(i);

        if (d->file == possiblePluginFile
             && d->lastFileModTime != possiblePluginFile.getLastModificationTime())
        {
            return false;
        }
    }

    return true;
}

bool KnownPluginList::scanAndAddFile (const File& possiblePluginFile,
                                      const bool dontRescanIfAlreadyInList,
                                      OwnedArray <PluginDescription>& typesFound)
{
    bool addedOne = false;

    if (dontRescanIfAlreadyInList
         && getTypeForFile (possiblePluginFile) != 0)
    {
        bool needsRescanning = false;

        for (int i = types.size(); --i >= 0;)
        {
            const PluginDescription* const d = types.getUnchecked(i);

            if (d->file == possiblePluginFile)
            {
                if (d->lastFileModTime != possiblePluginFile.getLastModificationTime())
                    needsRescanning = true;
                else
                    typesFound.add (new PluginDescription (*d));
            }
        }

        if (! needsRescanning)
            return false;
    }

    for (int i = 0; i < AudioPluginFormatManager::getInstance()->getNumFormats(); ++i)
    {
        AudioPluginFormat* const format = AudioPluginFormatManager::getInstance()->getFormat (i);

        OwnedArray <PluginDescription> found;
        format->findAllTypesForFile (found, possiblePluginFile);

        for (int i = 0; i < found.size(); ++i)
        {
            PluginDescription* const desc = found.getUnchecked(i);
            jassert (desc != 0);

            if (addType (*desc))
                addedOne = true;

            typesFound.add (new PluginDescription (*desc));
        }
    }

    return addedOne;
}

void KnownPluginList::scanAndAddDragAndDroppedFiles (const StringArray& files,
                                                     OwnedArray <PluginDescription>& typesFound)
{
    for (int i = 0; i < files.size(); ++i)
    {
        const File f (files [i]);

        if (! scanAndAddFile (f, true, typesFound))
        {
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
         || sortMethod == KnownPluginList::sortByManufacturer)
    {
        String lastSubMenuName;
        PopupMenu sub;

        for (int i = 0; i < sorted.size(); ++i)
        {
            const PluginDescription* const pd = sorted.getUnchecked(i);
            String thisSubMenuName (sortMethod == sortByCategory ? pd->category
                                                                 : pd->manufacturerName);

            if (thisSubMenuName.trim().isEmpty())
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
