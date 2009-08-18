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

#include "../jucer_Headers.h"
#include "jucer_JucerDocument.h"


//==============================================================================
BinaryResources::BinaryResources()
{
}

BinaryResources::~BinaryResources()
{
}

const BinaryResources& BinaryResources::operator= (const BinaryResources& other)
{
    for (int i = 0; i < other.resources.size(); ++i)
        add (other.resources[i]->name,
             other.resources[i]->originalFilename,
             other.resources[i]->data);

    return *this;
}

void BinaryResources::changed()
{
    if (document != 0)
    {
        document->changed();
        document->refreshAllPropertyComps();
    }
}

//==============================================================================
void BinaryResources::clear()
{
    if (resources.size() > 0)
    {
        resources.clear();
        changed();
    }
}

const StringArray BinaryResources::getResourceNames() const
{
    StringArray s;

    for (int i = 0; i < resources.size(); ++i)
        s.add (resources.getUnchecked(i)->name);

    return s;
}

BinaryResources::BinaryResource* BinaryResources::findResource (const String& name) const throw()
{
    for (int i = resources.size(); --i >= 0;)
        if (resources.getUnchecked(i)->name == name)
            return resources.getUnchecked(i);

    return 0;
}

const BinaryResources::BinaryResource* BinaryResources::getResource (const String& name) const
{
    return findResource (name);
}

const BinaryResources::BinaryResource* BinaryResources::getResourceForFile (const File& file) const
{
    for (int i = resources.size(); --i >= 0;)
        if (resources.getUnchecked(i)->originalFilename == file.getFullPathName())
            return resources.getUnchecked(i);

    return 0;
}

bool BinaryResources::add (const String& name, const File& file)
{
    MemoryBlock mb;

    if (! file.loadFileAsData (mb))
        return false;

    add (name, file.getFullPathName(), mb);
    return true;
}

void BinaryResources::add (const String& name, const String& originalFileName, const MemoryBlock& data)
{
    BinaryResource* r = findResource (name);

    if (r == 0)
    {
        resources.add (r = new BinaryResource());
        r->name = name;
    }

    r->originalFilename = originalFileName;
    r->data = data;
    deleteAndZero (r->drawable);

    changed();
}

bool BinaryResources::reload (const int index)
{
    return resources[index] != 0
            && add (resources [index]->name,
                    File (resources [index]->originalFilename));
}

const String BinaryResources::browseForResource (const String& title,
                                                 const String& wildcard,
                                                 const File& fileToStartFrom,
                                                 const String& resourceToReplace)
{
    FileChooser fc (title, fileToStartFrom, wildcard);

    if (fc.browseForFileToOpen())
    {
        String name (resourceToReplace);

        if (name.isEmpty())
            name = findUniqueName (fc.getResult().getFileName());

        if (! add (name, fc.getResult()))
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS("Adding Resource"),
                                         TRANS("Failed to load the file!"));

            name = String::empty;
        }

        return name;
    }

    return String::empty;
}

const String BinaryResources::findUniqueName (const String& rootName) const
{
    String nameRoot (makeValidCppIdentifier (rootName, true, true, false));
    String name (nameRoot);

    const StringArray resources (getResourceNames());

    int suffix = 1;

    while (resources.contains (name))
        name = nameRoot + String (++suffix);

    return name;
}

void BinaryResources::remove (const int i)
{
    if (resources[i] != 0)
    {
        resources.remove (i);
        changed();
    }
}

const Drawable* BinaryResources::getDrawable (const String& name) const
{
    BinaryResources::BinaryResource* const res = const_cast <BinaryResources::BinaryResource*> (getResource (name));

    if (res == 0)
        return 0;

    if (res->drawable == 0 && res->data.getSize() > 0)
        res->drawable = Drawable::createFromImageData (res->data.getData(), res->data.getSize());

    return res->drawable;
}

Image* BinaryResources::getImageFromCache (const String& name) const
{
    const BinaryResources::BinaryResource* const res = getResource (name);

    if (res != 0 && res->data.getSize() > 0)
        return ImageCache::getFromMemory (res->data.getData(), res->data.getSize());

    return 0;
}

void BinaryResources::loadFromCpp (const File& cppFileLocation, const String& cppFile)
{
    StringArray cpp;
    cpp.addLines (cppFile);

    clear();

    for (int i = 0; i < cpp.size(); ++i)
    {
        if (cpp[i].contains (T("JUCER_RESOURCE:")))
        {
            StringArray tokens;
            tokens.addTokens (cpp[i].fromFirstOccurrenceOf (T(":"), false, false), T(","), T("\"'"));
            tokens.trim();
            tokens.removeEmptyStrings();

            const String resourceName (tokens[0]);
            const int size = tokens[1].getIntValue();
            const String originalFileName (cppFileLocation.getSiblingFile (tokens[2].unquoted()).getFullPathName());

            jassert (resourceName.isNotEmpty() && size > 0);

            if (resourceName.isNotEmpty() && size > 0)
            {
                const int firstLine = i;

                while (i < cpp.size())
                    if (cpp [i++].contains (T("}")))
                        break;

                const String dataString (cpp.joinIntoString (T(" "), firstLine, i - firstLine)
                                            .fromFirstOccurrenceOf (T("{"), false, false));

                MemoryOutputStream out;
                const tchar* t = (const tchar*) dataString;
                int n = 0;

                while (*t != 0)
                {
                    const tchar c = *t++;

                    if (c >= T('0') && c <= T('9'))
                        n = n * 10 + (c - T('0'));
                    else if (c == T(','))
                    {
                        out.writeByte ((char) n);
                        n = 0;
                    }
                    else if (c == T('}'))
                        break;
                }

                jassert (size < out.getDataSize() && size > out.getDataSize() - 2);

                MemoryBlock mb (out.getData(), out.getDataSize());
                mb.setSize (size);

                add (resourceName, originalFileName, mb);
            }
        }
    }
}

//==============================================================================
void BinaryResources::fillInGeneratedCode (GeneratedCode& code) const
{
    if (resources.size() > 0)
    {
        code.publicMemberDeclarations << "// Binary resources:\n";

        String defs;
        defs << "//==============================================================================\n";
        defs << "// Binary resources - be careful not to edit any of these sections!\n\n";

        for (int i = 0; i < resources.size(); ++i)
        {
            code.publicMemberDeclarations
                << "static const char* "
                << resources[i]->name
                << ";\nstatic const int "
                << resources[i]->name
                << "Size;\n";

            const String name (resources[i]->name);
            const MemoryBlock& mb = resources[i]->data;

            defs << "// JUCER_RESOURCE: " << name << ", " << mb.getSize()
                << ", \""
                << File (resources[i]->originalFilename)
                    .getRelativePathFrom (code.document->getFile())
                    .replaceCharacter (T('\\'), T('/'))
                << "\"\n";

            String line1;
            line1 << "static const unsigned char resource_"
                  << code.className << "_" << name << "[] = { ";

            defs += line1;

            MemoryOutputStream out (65536, 16384);
            int charsOnLine = line1.length();

            for (int j = 0; j < mb.getSize(); ++j)
            {
                const int num = ((int) (unsigned char) mb[j]);
                out << num << ',';

                charsOnLine += 2;
                if (num >= 10)
                    ++charsOnLine;
                if (num >= 100)
                    ++charsOnLine;

                if (charsOnLine >= 200)
                {
                    charsOnLine = 0;
                    out << '\n';
                }
            }

            out << (char) 0;

            defs
              << out.getData()
              << "0,0};\n\nconst char* "
              << code.className << "::" << name
              << " = (const char*) resource_" << code.className << "_" << name
              << ";\nconst int "
              << code.className << "::" << name << "Size = "
              << mb.getSize()
              << ";\n\n";
        }

        code.staticMemberDefinitions += defs;
    }
}

BinaryResources::BinaryResource::BinaryResource()
    : drawable (0)
{
}

BinaryResources::BinaryResource::~BinaryResource()
{
    deleteAndZero (drawable);
}
