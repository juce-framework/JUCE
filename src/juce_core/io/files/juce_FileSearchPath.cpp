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

#include "../../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_FileSearchPath.h"


//==============================================================================
FileSearchPath::FileSearchPath()
{
}

FileSearchPath::FileSearchPath (const String& path)
{
    init (path);
}

FileSearchPath::FileSearchPath (const FileSearchPath& other)
  : directories (other.directories)
{
}

FileSearchPath::~FileSearchPath()
{
}

const FileSearchPath& FileSearchPath::operator= (const String& path)
{
    init (path);
    return *this;
}

void FileSearchPath::init (const String& path)
{
    directories.clear();
    directories.addTokens (path, T(";"), T("\""));
    directories.trim();
    directories.removeEmptyStrings();

    for (int i = directories.size(); --i >= 0;)
        directories.set (i, directories[i].unquoted());
}

int FileSearchPath::getNumPaths() const
{
    return directories.size();
}

const File FileSearchPath::operator[] (const int index) const
{
    return File (directories [index]);
}

const String FileSearchPath::toString() const
{
    StringArray directories2 (directories);
    for (int i = directories2.size(); --i >= 0;)
        if (directories2[i].containsChar (T(';')))
            directories2.set (i, directories2[i].quoted());

    return directories2.joinIntoString (T(";"));
}

void FileSearchPath::add (const File& dir, const int insertIndex)
{
    directories.insert (insertIndex, dir.getFullPathName());
}

void FileSearchPath::addIfNotAlreadyThere (const File& dir)
{
    for (int i = 0; i < directories.size(); ++i)
        if (File (directories[i]) == dir)
            return;

    add (dir);
}

void FileSearchPath::remove (const int index)
{
    directories.remove (index);
}

void FileSearchPath::addPath (const FileSearchPath& other)
{
    for (int i = 0; i < other.getNumPaths(); ++i)
        addIfNotAlreadyThere (other[i]);
}

void FileSearchPath::removeRedundantPaths()
{
    for (int i = directories.size(); --i >= 0;)
    {
        const File d1 (directories[i]);

        for (int j = directories.size(); --j >= 0;)
        {
            const File d2 (directories[j]);

            if ((i != j) && (d1.isAChildOf (d2) || d1 == d2))
            {
                directories.remove (i);
                break;
            }
        }
    }
}

void FileSearchPath::removeNonExistentPaths()
{
    for (int i = directories.size(); --i >= 0;)
        if (! File (directories[i]).isDirectory())
            directories.remove (i);
}

int FileSearchPath::findChildFiles (OwnedArray<File>& results,
                                    const int whatToLookFor,
                                    const bool searchRecursively,
                                    const String& wildCardPattern) const
{
    int total = 0;

    for (int i = 0; i < directories.size(); ++i)
        total += operator[] (i).findChildFiles (results,
                                                whatToLookFor,
                                                searchRecursively,
                                                wildCardPattern);

    return total;
}

bool FileSearchPath::isFileInPath (const File& fileToCheck,
                                   const bool checkRecursively) const
{
    for (int i = directories.size(); --i >= 0;)
    {
        const File d (directories[i]);

        if (checkRecursively)
        {
            if (fileToCheck.isAChildOf (d))
                return true;
        }
        else
        {
            if (fileToCheck.getParentDirectory() == d)
                return true;
        }
    }

    return false;
}

END_JUCE_NAMESPACE
