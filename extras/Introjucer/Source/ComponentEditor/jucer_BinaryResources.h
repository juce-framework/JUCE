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

#ifndef JUCER_BINARYRESOURCES_H_INCLUDED
#define JUCER_BINARYRESOURCES_H_INCLUDED

class JucerDocument;


//==============================================================================
/**
    Manages a list of binary data objects that a JucerDocument wants to embed in
    the code it generates.
*/
class BinaryResources
{
public:
    //==============================================================================
    BinaryResources();
    ~BinaryResources();

    BinaryResources& operator= (const BinaryResources& other);

    void loadFromCpp (const File& cppFileLocation, const String& cpp);

    //==============================================================================
    struct BinaryResource
    {
        String name;
        String originalFilename;
        MemoryBlock data;
        ScopedPointer<Drawable> drawable;
    };

    void clear();
    bool add (const String& name, const File& file);
    void add (const String& name, const String& originalFileName, const MemoryBlock& data);
    void remove (const int index);
    bool reload (const int index);
    String browseForResource (const String& title, const String& wildcard,
                              const File& fileToStartFrom, const String& resourceToReplace);

    String findUniqueName (const String& rootName) const;

    int size() const noexcept                                            { return resources.size(); }
    const BinaryResource* operator[] (const int index) const noexcept    { return resources [index]; }

    const BinaryResource* getResource (const String& resourceName) const;
    const BinaryResource* getResourceForFile (const File& file) const;

    StringArray getResourceNames() const;

    const Drawable* getDrawable (const String& name) const;
    Image getImageFromCache (const String& name) const;

    template <class ElementComparator>
    void sort (ElementComparator& sorter)
    {
        resources.sort (sorter, true);
        changed();
    }

    //==============================================================================
    void setDocument (JucerDocument* const doc)                   { document = doc; }
    JucerDocument* getDocument() const noexcept                   { return document; }

    void fillInGeneratedCode (GeneratedCode& code) const;


private:
    //==============================================================================
    JucerDocument* document;
    OwnedArray <BinaryResource> resources;

    BinaryResource* findResource (const String& name) const noexcept;
    void changed();
};


#endif   // JUCER_BINARYRESOURCES_H_INCLUDED
