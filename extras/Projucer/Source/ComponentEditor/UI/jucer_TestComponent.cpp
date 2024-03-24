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
        testComponents.getUnchecked (i)->reload();
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

        loadedDocument.reset();

        filename = newName;
        lastModificationTime = findFile().getLastModificationTime();

        loadedDocument.reset (JucerDocument::createForCppFile (nullptr, findFile()));

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
        handleResize();
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

void TestComponent::handleResize()
{
    if (Component* const c = getChildComponent (0))
    {
        setOpaque (c->isOpaque());
        c->setBounds (getLocalBounds());
    }
}

void TestComponent::resized()
{
    handleResize();
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
