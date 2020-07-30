/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../jucer_JucerDocument.h"

//==============================================================================
class TestComponent  : public Component
{
public:
    //==============================================================================
    TestComponent (JucerDocument* const ownerDocument,
                   JucerDocument* const loadedDocument,
                   const bool alwaysFillBackground);

    ~TestComponent() override;

    //==============================================================================
    void setFilename (const String& fn);
    const String& getFilename() const noexcept                  { return filename; }

    void setConstructorParams (const String& newParams);
    const String& getConstructorParams() const noexcept         { return constructorParams; }

    File findFile() const;

    JucerDocument* getDocument() const noexcept                 { return loadedDocument.get(); }
    JucerDocument* getOwnerDocument() const noexcept            { return ownerDocument; }

    void setToInitialSize();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    static void showInDialogBox (JucerDocument&);

    // reloads any test comps that need to do so
    static void reloadAll();

private:
    JucerDocument* ownerDocument;
    std::unique_ptr<JucerDocument> loadedDocument;
    String filename, constructorParams;
    Time lastModificationTime;
    const bool alwaysFillBackground;

    void updateContents();
    void reload();
};
