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

#include "../jucer_Headers.h"
#include "jucer_TestComponent.h"
#include "../model/jucer_ObjectTypes.h"


static Array <TestComponent*> testComponents;

//==============================================================================
TestComponent::TestComponent (JucerDocument* const ownerDocument_,
                              JucerDocument* const loadedDocument_,
                              const bool alwaysFillBackground_)
    : ownerDocument (ownerDocument_),
      loadedDocument (loadedDocument_),
      alwaysFillBackground (alwaysFillBackground_)
{
    setToInitialSize();
    updateContents();
    testComponents.add (this);
}

TestComponent::~TestComponent()
{
    testComponents.removeValue (this);
    deleteAllChildren();
    delete loadedDocument;
}

//==============================================================================
void TestComponent::reloadAll()
{
    for (int i = testComponents.size(); --i >= 0;)
        testComponents.getUnchecked(i)->reload();
}

void TestComponent::reload()
{
    if (findFile().exists() && lastModificationTime != findFile().getLastModificationTime())
        setFilename (filename);
}

//==============================================================================
static StringArray recursiveFiles;

const File TestComponent::findFile() const
{
    if (filename.isEmpty())
        return File::nonexistent;

    if (ownerDocument != 0)
        return ownerDocument->getFile().getSiblingFile (filename);

    return File::getCurrentWorkingDirectory().getChildFile (filename);
}

void TestComponent::setFilename (const String& newName)
{
    File newFile;

    if (newName.isNotEmpty())
    {
        if (ownerDocument != 0)
            newFile = ownerDocument->getFile().getSiblingFile (newName);
        else
            newFile = File::getCurrentWorkingDirectory().getChildFile (newName);
    }

    if (! recursiveFiles.contains (newFile.getFullPathName()))
    {
        recursiveFiles.add (newFile.getFullPathName());

        deleteAndZero (loadedDocument);

        filename = newName;
        lastModificationTime = findFile().getLastModificationTime();

        loadedDocument = ObjectTypes::loadDocumentFromFile (findFile(), false);

        updateContents();
        repaint();

        recursiveFiles.remove (recursiveFiles.size() - 1);
    }
}

void TestComponent::setConstructorParams (const String& newParams)
{
    constructorParams = newParams;
}

void TestComponent::updateContents()
{
    deleteAllChildren();
    repaint();

    if (loadedDocument != 0)
    {
        addAndMakeVisible (loadedDocument->createTestComponent (alwaysFillBackground));
        resized();
    }
}

void TestComponent::setToInitialSize()
{
    if (loadedDocument != 0)
    {
        setSize (loadedDocument->getInitialWidth(),
                 loadedDocument->getInitialHeight());
    }
    else
    {
        setSize (100, 100);
    }
}

//==============================================================================
void TestComponent::paint (Graphics& g)
{
    if (loadedDocument == 0)
    {
        g.fillAll (Colours::white.withAlpha (0.25f));

        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawRect (0, 0, getWidth(), getHeight());
        g.drawLine (0.0f, 0.0f, (float) getWidth(), (float) getHeight());
        g.drawLine (0.0f, (float) getHeight(), (float) getWidth(), 0.0f);

        g.setFont (14.0f);
        g.drawText (T("Jucer Component"),
                    0, 0, getWidth(), getHeight() / 2,
                    Justification::centred, true);
        g.drawText (T("(no file loaded)"),
                    0, getHeight() / 2, getWidth(), getHeight() / 2,
                    Justification::centred, true);
    }
}

void TestComponent::resized()
{
    Component* const c = getChildComponent (0);

    if (c != 0)
    {
        setOpaque (c->isOpaque());
        c->setBounds (0, 0, getWidth(), getHeight());
    }
}

//==============================================================================
void TestComponent::showInDialogBox (JucerDocument& document)
{
    TooltipWindow tooltipWindow (0, 400);

    TestComponent testComp (0, document.createCopy(), true);

    DialogWindow::showModalDialog (T("Testing: ") + document.getClassName(),
                                   &testComp, 0,
                                   Colours::azure,
                                   true, true);
}
