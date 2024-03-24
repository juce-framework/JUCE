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

#include "../jucer_JucerDocument.h"

//==============================================================================
class TestComponent  : public Component
{
public:
    //==============================================================================
    TestComponent (JucerDocument* const ownerDocument,
                   JucerDocument* const loadedDocument,
                   bool alwaysFillBackground);

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

    void handleResize();
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
