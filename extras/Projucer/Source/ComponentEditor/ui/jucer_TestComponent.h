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

#ifndef JUCER_TESTCOMPONENT_H_INCLUDED
#define JUCER_TESTCOMPONENT_H_INCLUDED

#include "../jucer_JucerDocument.h"


//==============================================================================
/**
*/
class TestComponent  : public Component
{
public:
    //==============================================================================
    TestComponent (JucerDocument* const ownerDocument,
                   JucerDocument* const loadedDocument,
                   const bool alwaysFillBackground);

    ~TestComponent();

    //==============================================================================
    void setFilename (const String& fn);
    const String& getFilename() const noexcept                  { return filename; }

    void setConstructorParams (const String& newParams);
    const String& getConstructorParams() const noexcept         { return constructorParams; }

    File findFile() const;

    JucerDocument* getDocument() const noexcept                 { return loadedDocument; }
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
    ScopedPointer<JucerDocument> loadedDocument;
    String filename, constructorParams;
    Time lastModificationTime;
    LookAndFeel_V2 lookAndFeel;
    const bool alwaysFillBackground;

    void updateContents();
    void reload();
};


#endif   // JUCER_TESTCOMPONENT_H_INCLUDED
