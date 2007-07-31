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

#ifndef __JUCER_BINARYRESOURCES_JUCEHEADER__
#define __JUCER_BINARYRESOURCES_JUCEHEADER__

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

    const BinaryResources& operator= (const BinaryResources& other);

    void loadFromCpp (const String& cpp);

    //==============================================================================
    struct BinaryResource
    {
        BinaryResource();
        ~BinaryResource();

        String name;
        String originalFilename;
        MemoryBlock data;
        Drawable* drawable;
    };

    void clear();
    bool add (const String& name, const File& file);
    void add (const String& name, const String& originalFileName, const MemoryBlock& data);
    void remove (const int index);
    bool reload (const int index);
    const String browseForResource (const String& title, const String& wildcard,
                                    const File& fileToStartFrom, const String& resourceToReplace);

    const String findUniqueName (const String& rootName) const;

    int size() const throw()                                            { return resources.size(); }
    const BinaryResource* operator[] (const int index) const throw()    { return resources [index]; }

    const BinaryResource* getResource (const String& resourceName) const;
    const BinaryResource* getResourceForFile (const File& file) const;

    const StringArray getResourceNames() const;

    const Drawable* getDrawable (const String& name) const;
    Image* getImageFromCache (const String& name) const;

    template <class ElementComparator>
    void sort (ElementComparator& sorter)
    {
        resources.sort (sorter, true);
        changed();
    }

    //==============================================================================
    void setDocument (JucerDocument* const document_)                   { document = document_; }
    JucerDocument* getDocument() const throw()                          { return document; }

    void fillInGeneratedCode (GeneratedCode& code) const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    JucerDocument* document;
    OwnedArray <BinaryResource> resources;

    BinaryResource* findResource (const String& name) const throw();
    void changed();
};


#endif   // __JUCER_BINARYRESOURCES_JUCEHEADER__
