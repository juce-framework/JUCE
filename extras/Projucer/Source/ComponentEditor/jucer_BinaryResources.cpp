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

#include "../jucer_Headers.h"
#include "jucer_JucerDocument.h"


//==============================================================================
BinaryResources::BinaryResources()
{
}

BinaryResources::~BinaryResources()
{
}

BinaryResources& BinaryResources::operator= (const BinaryResources& other)
{
    for (int i = 0; i < other.resources.size(); ++i)
        add (other.resources[i]->name,
             other.resources[i]->originalFilename,
             other.resources[i]->data);

    return *this;
}

void BinaryResources::changed()
{
    if (document != nullptr)
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

StringArray BinaryResources::getResourceNames() const
{
    StringArray s;

    for (int i = 0; i < resources.size(); ++i)
        s.add (resources.getUnchecked(i)->name);

    return s;
}

BinaryResources::BinaryResource* BinaryResources::findResource (const String& name) const noexcept
{
    for (int i = resources.size(); --i >= 0;)
        if (resources.getUnchecked(i)->name == name)
            return resources.getUnchecked(i);

    return nullptr;
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

    return nullptr;
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

    if (r == nullptr)
    {
        resources.add (r = new BinaryResource());
        r->name = name;
    }

    r->originalFilename = originalFileName;
    r->data = data;
    r->drawable = nullptr;

    changed();
}

bool BinaryResources::reload (const int index)
{
    return resources[index] != 0
            && add (resources [index]->name,
                    File (resources [index]->originalFilename));
}

String BinaryResources::browseForResource (const String& title,
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

            name.clear();
        }

        return name;
    }

    return String();
}

String BinaryResources::findUniqueName (const String& rootName) const
{
    String nameRoot (CodeHelpers::makeValidIdentifier (rootName, true, true, false));
    String name (nameRoot);

    const StringArray names (getResourceNames());

    int suffix = 1;

    while (names.contains (name))
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
    if (BinaryResources::BinaryResource* const res = const_cast<BinaryResources::BinaryResource*> (getResource (name)))
    {
        if (res->drawable == nullptr && res->data.getSize() > 0)
            res->drawable = Drawable::createFromImageData (res->data.getData(),
                                                           res->data.getSize());

        return res->drawable;
    }

    return nullptr;
}

Image BinaryResources::getImageFromCache (const String& name) const
{
    if (const BinaryResources::BinaryResource* const res = getResource (name))
        if (res->data.getSize() > 0)
            return ImageCache::getFromMemory (res->data.getData(), (int) res->data.getSize());

    return Image();
}

void BinaryResources::loadFromCpp (const File& cppFileLocation, const String& cppFile)
{
    StringArray cpp;
    cpp.addLines (cppFile);

    clear();

    for (int i = 0; i < cpp.size(); ++i)
    {
        if (cpp[i].contains ("JUCER_RESOURCE:"))
        {
            StringArray tokens;
            tokens.addTokens (cpp[i].fromFirstOccurrenceOf (":", false, false), ",", "\"'");
            tokens.trim();
            tokens.removeEmptyStrings();

            const String resourceName (tokens[0]);
            const int resourceSize = tokens[1].getIntValue();
            const String originalFileName (cppFileLocation.getSiblingFile (tokens[2].unquoted()).getFullPathName());

            jassert (resourceName.isNotEmpty() && resourceSize > 0);

            if (resourceName.isNotEmpty() && resourceSize > 0)
            {
                const int firstLine = i;

                while (i < cpp.size())
                    if (cpp [i++].contains ("}"))
                        break;

                const String dataString (cpp.joinIntoString (" ", firstLine, i - firstLine)
                                            .fromFirstOccurrenceOf ("{", false, false));

                MemoryOutputStream out;
                String::CharPointerType t (dataString.getCharPointer());
                int n = 0;

                while (! t.isEmpty())
                {
                    const juce_wchar c = t.getAndAdvance();

                    if (c >= '0' && c <= '9')
                        n = n * 10 + (c - '0');
                    else if (c == ',')
                    {
                        out.writeByte ((char) n);
                        n = 0;
                    }
                    else if (c == '}')
                        break;
                }

                jassert (resourceSize < (int) out.getDataSize() && resourceSize > (int) out.getDataSize() - 2);

                MemoryBlock mb (out.getData(), out.getDataSize());
                mb.setSize ((size_t) resourceSize);

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

        MemoryOutputStream defs;

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

            defs << "// JUCER_RESOURCE: " << name << ", " << (int) mb.getSize()
                << ", \""
                << File (resources[i]->originalFilename)
                    .getRelativePathFrom (code.document->getCppFile())
                    .replaceCharacter ('\\', '/')
                << "\"\n";

            String line1;
            line1 << "static const unsigned char resource_"
                  << code.className << "_" << name << "[] = { ";

            defs << line1;

            int charsOnLine = line1.length();

            for (size_t j = 0; j < mb.getSize(); ++j)
            {
                const int num = (int) (unsigned char) mb[j];
                defs << num << ',';

                charsOnLine += 2;
                if (num >= 10)   ++charsOnLine;
                if (num >= 100)  ++charsOnLine;

                if (charsOnLine >= 200)
                {
                    charsOnLine = 0;
                    defs << '\n';
                }
            }

            defs
              << "0,0};\n\n"
                 "const char* " << code.className << "::" << name
              << " = (const char*) resource_" << code.className << "_" << name
              << ";\nconst int "
              << code.className << "::" << name << "Size = "
              << (int) mb.getSize()
              << ";\n\n";
        }

        code.staticMemberDefinitions << defs.toString();
    }
}
