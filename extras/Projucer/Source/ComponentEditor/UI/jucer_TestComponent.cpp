/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
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

    setLookAndFeel (&getLookAndFeel());
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
        return {};

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
