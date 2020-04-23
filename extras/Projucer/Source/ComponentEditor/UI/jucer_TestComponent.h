/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
