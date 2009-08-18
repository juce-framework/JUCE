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

#ifndef __JUCER_TESTCOMPONENT_JUCEHEADER__
#define __JUCER_TESTCOMPONENT_JUCEHEADER__

#include "../model/jucer_JucerDocument.h"


//==============================================================================
/**
*/
class TestComponent  : public Component
{
public:
    //==============================================================================
    TestComponent (JucerDocument* const ownerDocument_,
                   JucerDocument* const loadedDocument_,
                   const bool alwaysFillBackground);

    ~TestComponent();

    //==============================================================================
    void setFilename (const String& fn);
    const String& getFilename() const throw()                   { return filename; }

    void setConstructorParams (const String& newParams);
    const String& getConstructorParams() const throw()          { return constructorParams; }

    const File findFile() const;

    JucerDocument* getDocument() const throw()                  { return loadedDocument; }
    JucerDocument* getOwnerDocument() const throw()             { return ownerDocument; }

    void setToInitialSize();

    //==============================================================================
    void paint (Graphics& g);
    void resized();

    static void showInDialogBox (JucerDocument& design);

    // reloads any test comps that need to do so
    static void reloadAll();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    JucerDocument* ownerDocument;
    JucerDocument* loadedDocument;
    String filename, constructorParams;
    Time lastModificationTime;
    const bool alwaysFillBackground;

    void updateContents();
    void reload();
};


#endif   // __JUCER_TESTCOMPONENT_JUCEHEADER__
