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

#include "../../jucer_Headers.h"
#include "jucer_TestComponent.h"
#include "../jucer_ObjectTypes.h"


static Array <TestComponent*> testComponents;

//==============================================================================
TestComponent::TestComponent (JucerDocument* const doc,
                              JucerDocument* const loaded,
                              const bool alwaysFill)
    : ownerDocument (doc),
      loadedDocument (loaded),
      alwaysFillBackground (alwaysFill)
{
    setToInitialSize();
    updateContents();
    testComponents.add (this);

    setLookAndFeel (&lookAndFeel);
}

TestComponent::~TestComponent()
{
    testComponents.removeFirstMatchingValue (this);
    deleteAllChildren();
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

File TestComponent::findFile() const
{
    if (filename.isEmpty())
        return File();

    if (ownerDocument != nullptr)
        return ownerDocument->getCppFile().getSiblingFile (filename);

    return File::getCurrentWorkingDirectory().getChildFile (filename);
}

void TestComponent::setFilename (const String& newName)
{
    File newFile;

    if (newName.isNotEmpty())
    {
        if (ownerDocument != nullptr)
            newFile = ownerDocument->getCppFile().getSiblingFile (newName);
        else
            newFile = File::getCurrentWorkingDirectory().getChildFile (newName);
    }

    if (! recursiveFiles.contains (newFile.getFullPathName()))
    {
        recursiveFiles.add (newFile.getFullPathName());

        loadedDocument = nullptr;

        filename = newName;
        lastModificationTime = findFile().getLastModificationTime();

        loadedDocument = JucerDocument::createForCppFile (nullptr, findFile());

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

    if (loadedDocument != nullptr)
    {
        addAndMakeVisible (loadedDocument->createTestComponent (alwaysFillBackground));
        resized();
    }
}

void TestComponent::setToInitialSize()
{
    if (loadedDocument != nullptr)
        setSize (loadedDocument->getInitialWidth(),
                 loadedDocument->getInitialHeight());
    else
        setSize (100, 100);
}

//==============================================================================
void TestComponent::paint (Graphics& g)
{
    if (loadedDocument == nullptr)
    {
        g.fillAll (Colours::white.withAlpha (0.25f));

        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawRect (getLocalBounds());
        g.drawLine (0.0f, 0.0f, (float) getWidth(), (float) getHeight());
        g.drawLine (0.0f, (float) getHeight(), (float) getWidth(), 0.0f);

        g.setFont (14.0f);
        g.drawText ("Projucer Component",
                    0, 0, getWidth(), getHeight() / 2,
                    Justification::centred, true);
        g.drawText ("(no file loaded)",
                    0, getHeight() / 2, getWidth(), getHeight() / 2,
                    Justification::centred, true);
    }
}

void TestComponent::resized()
{
    if (Component* const c = getChildComponent (0))
    {
        setOpaque (c->isOpaque());
        c->setBounds (getLocalBounds());
    }
}

//==============================================================================
void TestComponent::showInDialogBox (JucerDocument& document)
{
    DialogWindow::LaunchOptions o;
    o.content.setOwned (new TestComponent (nullptr, document.createCopy(), true));
    o.dialogTitle                   = "Testing: " + document.getClassName();
    o.dialogBackgroundColour        = Colours::azure;
    o.escapeKeyTriggersCloseButton  = true;
    o.useNativeTitleBar             = false;
    o.resizable                     = true;

    o.launchAsync();
}
