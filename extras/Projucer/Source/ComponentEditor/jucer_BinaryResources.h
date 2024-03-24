/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

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
    BinaryResources& operator= (const BinaryResources& other);

    void loadFromCpp (const File& cppFileLocation, const String& cpp);

    //==============================================================================
    struct BinaryResource
    {
        String name;
        String originalFilename;
        MemoryBlock data;
        std::unique_ptr<Drawable> drawable;
    };

    void clear();
    bool add (const String& name, const File& file);
    void add (const String& name, const String& originalFileName, const MemoryBlock& data);
    void remove (int index);
    bool reload (int index);
    void browseForResource (const String& title, const String& wildcard,
                            const File& fileToStartFrom, const String& resourceToReplace,
                            std::function<void (String)> callback);

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
    BinaryResource* findResource (const String& name) const noexcept;
    void changed();

    //==============================================================================
    JucerDocument* document;
    OwnedArray<BinaryResource> resources;
    std::unique_ptr<FileChooser> chooser;
    ScopedMessageBox messageBox;

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (BinaryResources)
};
