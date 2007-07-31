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
