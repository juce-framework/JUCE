/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "jucer_DrawableDocument.h"
#include "jucer_ResourceFile.h"

//==============================================================================
static const char* const drawableTag = "DRAWABLE";


//==============================================================================
DrawableDocument::DrawableDocument (Project* project_, const File& drawableFile_)
    : project (project_),
      drawableFile (drawableFile_),
      drawableRoot (drawableTag),
      saveAsXml (true), needsSaving (false)
{
    DrawableComposite dc;
    drawableRoot.addChild (dc.createValueTree(), -1, 0);

    setName ("Drawable");

    drawableRoot.addListener (this);
}

DrawableDocument::~DrawableDocument()
{
    if (needsSaving)
        save();

    drawableRoot.removeListener (this);
}

ValueTree DrawableDocument::getRootDrawableNode() const
{
    return drawableRoot.getChild (0);
}

//==============================================================================
void DrawableDocument::setName (const String& name)
{
    drawableRoot.setProperty ("name", name, getUndoManager());
}

const String DrawableDocument::getName() const
{
    return drawableRoot ["name"];
}

void DrawableDocument::addMissingIds (ValueTree tree) const
{
    if (! tree.hasProperty ("id"))
        tree.setProperty ("id", createAlphaNumericUID(), 0);

    for (int i = tree.getNumChildren(); --i >= 0;)
        addMissingIds (tree.getChild(i));
}

bool DrawableDocument::hasChangedSinceLastSave() const
{
    return needsSaving;
}

bool DrawableDocument::reload()
{
    ScopedPointer <InputStream> stream (drawableFile.createInputStream());

    if (stream != 0 && load (*stream))
    {
        undoManager.clearUndoHistory();
        needsSaving = false;
        return true;
    }

    return false;
}

bool DrawableDocument::save()
{
    TemporaryFile tempFile (drawableFile);
    ScopedPointer <OutputStream> out (tempFile.getFile().createOutputStream());

    if (out == 0)
        return false;

    save (*out);

    needsSaving = ! tempFile.overwriteTargetFileWithTemporary();
    return ! needsSaving;
}

void DrawableDocument::save (OutputStream& output)
{
    if (saveAsXml)
    {
        ScopedPointer <XmlElement> xml (drawableRoot.createXml());
        jassert (xml != 0);

        if (xml != 0)
            xml->writeToStream (output, String::empty, false, false);
    }
    else
    {
        drawableRoot.writeToStream (output);
    }
}

bool DrawableDocument::load (InputStream& input)
{
    int64 originalPos = input.getPosition();
    ValueTree loadedTree ("dummy");

    XmlDocument xmlDoc (input.readEntireStreamAsString());
    ScopedPointer <XmlElement> xml (xmlDoc.getDocumentElement());

    if (xml != 0)
    {
        loadedTree = ValueTree::fromXml (*xml);
    }
    else
    {
        input.setPosition (originalPos);
        loadedTree = ValueTree::readFromStream (input);
    }

    if (loadedTree.hasType (drawableTag))
    {
        addMissingIds (loadedTree);

        drawableRoot.removeListener (this);
        drawableRoot = loadedTree;
        drawableRoot.addListener (this);

        valueTreeParentChanged (loadedTree);

        needsSaving = false;
        undoManager.clearUndoHistory();

        return true;
    }

    return false;
}

void DrawableDocument::changed()
{
    needsSaving = true;
    startTimer (1000);
    sendChangeMessage (this);
}

void DrawableDocument::timerCallback()
{
    stopTimer();
    getUndoManager()->beginNewTransaction();

    //if (needsSaving)
      //  save();
}

//==============================================================================
static const Colour getRandomColour()
{
    return Colours::red.withHue (Random::getSystemRandom().nextFloat());
}

void DrawableDocument::addDrawable (Drawable& d)
{
    DrawableComposite dc;
    dc.insertDrawable (d.createCopy());

    ValueTree dcNode (dc.createValueTree());
    ValueTree subNode (dcNode.getChild(0));
    dcNode.removeChild (subNode, 0);
    addMissingIds (subNode);

    getRootDrawableNode().addChild (subNode, -1, getUndoManager());
}

void DrawableDocument::addRectangle()
{
    Path p;
    p.addRectangle ((float) Random::getSystemRandom().nextInt (500),
                    (float) Random::getSystemRandom().nextInt (500),
                    100.0f, 100.0f);

    DrawablePath d;
    d.setPath (p);
    d.setFill (FillType (getRandomColour()));

    addDrawable (d);
}

void DrawableDocument::addCircle()
{
    Path p;
    p.addEllipse ((float) Random::getSystemRandom().nextInt (500),
                  (float) Random::getSystemRandom().nextInt (500),
                  100.0f, 100.0f);

    DrawablePath d;
    d.setPath (p);
    d.setFill (FillType (getRandomColour()));

    addDrawable (d);
}

void DrawableDocument::addImage (const File& imageFile)
{
    jassertfalse

    DrawableImage d;

    addDrawable (d);
}

//==============================================================================
void DrawableDocument::valueTreePropertyChanged (ValueTree& tree, const var::identifier& name)
{
    changed();
}

void DrawableDocument::valueTreeChildrenChanged (ValueTree& tree)
{
    changed();
}

void DrawableDocument::valueTreeParentChanged (ValueTree& tree)
{
    changed();
}
