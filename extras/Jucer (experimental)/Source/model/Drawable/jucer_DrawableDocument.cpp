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


//==============================================================================
static const char* const drawableTag        = "DRAWABLE";
static const char* const markersGroupXTag   = "MARKERS_X";
static const char* const markersGroupYTag   = "MARKERS_Y";


//==============================================================================
DrawableDocument::DrawableDocument (Project* project_)
    : project (project_),
      root (drawableTag),
      saveAsXml (true),
      needsSaving (false)
{
    DrawableComposite dc;
    root.addChild (dc.createValueTree(), -1, 0);

    setName ("Drawable");
    checkRootObject();

    root.addListener (this);
}

DrawableDocument::~DrawableDocument()
{
    root.removeListener (this);
}

ValueTree DrawableDocument::getRootDrawableNode() const
{
    return root.getChild (0);
}

void DrawableDocument::checkRootObject()
{
    if (markersX == 0)
        markersX = new MarkerList (*this, true);

    if (markersY == 0)
        markersY = new MarkerList (*this, false);

    if ((int) getCanvasWidth().getValue() <= 0)
        getCanvasWidth() = 500;

    if ((int) getCanvasHeight().getValue() <= 0)
        getCanvasHeight() = 500;
}

const String DrawableDocument::getIdFor (const ValueTree& object)
{
    return object ["id"];
}

//==============================================================================
void DrawableDocument::setName (const String& name)
{
    root.setProperty ("name", name, getUndoManager());
}

const String DrawableDocument::getName() const
{
    return root ["name"];
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

bool DrawableDocument::reload (const File& drawableFile)
{
    ScopedPointer <InputStream> stream (drawableFile.createInputStream());

    if (stream != 0 && load (*stream))
    {
        checkRootObject();
        undoManager.clearUndoHistory();
        needsSaving = false;
        return true;
    }

    return false;
}

bool DrawableDocument::save (const File& drawableFile)
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
        ScopedPointer <XmlElement> xml (root.createXml());
        jassert (xml != 0);

        if (xml != 0)
            xml->writeToStream (output, String::empty, false, false);
    }
    else
    {
        root.writeToStream (output);
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

        root.removeListener (this);
        root = loadedTree;
        root.addListener (this);

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
    sendChangeMessage (this);
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

//==============================================================================
const Coordinate DrawableDocument::findMarker (const String& name, bool isHorizontal) const
{
    if (name == Coordinate::parentRightMarkerName)     return Coordinate ((double) getCanvasWidth().getValue(), isHorizontal);
    if (name == Coordinate::parentBottomMarkerName)    return Coordinate ((double) getCanvasHeight().getValue(), isHorizontal);

    if (name.containsChar ('.'))
    {
        const String compName (name.upToFirstOccurrenceOf (".", false, false).trim());
        const String edge (name.fromFirstOccurrenceOf (".", false, false).trim());

        if (compName.isNotEmpty() && edge.isNotEmpty())
        {
/*            const ValueTree comp (getComponentWithMemberName (compName));

            if (comp.isValid())
            {
                const RectangleCoordinates coords (getCoordsFor (comp));

                if (edge == "left")   return coords.left;
                if (edge == "right")  return coords.right;
                if (edge == "top")    return coords.top;
                if (edge == "bottom") return coords.bottom;
            }*/
        }
    }

    const ValueTree marker (getMarkerList (isHorizontal).getMarkerNamed (name));
    if (marker.isValid())
        return getMarkerList (isHorizontal).getCoordinate (marker);

    return Coordinate (isHorizontal);
}

DrawableDocument::MarkerList::MarkerList (DrawableDocument& document_, bool isX_)
    : MarkerListBase (document_.getRoot().getChildWithName (isX_ ? markersGroupXTag : markersGroupYTag), isX_),
      document (document_)
{
}

const Coordinate DrawableDocument::MarkerList::findMarker (const String& name, bool isHorizontal_) const
{
    if (isHorizontal_ == isX)
    {
        if (name == Coordinate::parentRightMarkerName)   return Coordinate ((double) document.getCanvasWidth().getValue(), isX);
        if (name == Coordinate::parentBottomMarkerName)  return Coordinate ((double) document.getCanvasHeight().getValue(), isX);

        const ValueTree marker (document.getMarkerList (isX).getMarkerNamed (name));
        if (marker.isValid())
            return document.getMarkerList (isX).getCoordinate (marker);
    }

    return Coordinate (isX);
}

bool DrawableDocument::MarkerList::createProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree marker (group.getChildWithProperty (getIdProperty(), itemId));

    if (marker.isValid())
    {
        props.add (new TextPropertyComponent (getNameAsValue (marker), "Marker Name", 256, false));
//        props.add (new MarkerPositionComponent (document, "Position", marker,
  //                                              marker.getPropertyAsValue (markerPosProperty, document.getUndoManager()),
    //                                            contains (marker)));
        return true;
    }

    return false;
}

void DrawableDocument::addMarkerMenuItem (int i, const Coordinate& coord, const String& name, PopupMenu& menu,
                                          bool isAnchor1, const String& fullCoordName)
{
    Coordinate requestedCoord (findMarker (name, coord.isHorizontal()));

//    menu.addItem (i, name,
  //                ! (name == fullCoordName || requestedCoord.referencesIndirectly (fullCoordName, *this)),
    //              name == (isAnchor1 ? coord.getAnchor1() : coord.getAnchor2()));
}

void DrawableDocument::MarkerList::addMarkerMenuItems (const ValueTree& markerState, const Coordinate& coord, PopupMenu& menu, bool isAnchor1)
{
    const String fullCoordName (getName (markerState));

    if (coord.isHorizontal())
    {
        document.addMarkerMenuItem (1, coord, Coordinate::parentLeftMarkerName, menu, isAnchor1, fullCoordName);
        document.addMarkerMenuItem (2, coord, Coordinate::parentRightMarkerName, menu, isAnchor1, fullCoordName);
    }
    else
    {
        document.addMarkerMenuItem (1, coord, Coordinate::parentTopMarkerName, menu, isAnchor1, fullCoordName);
        document.addMarkerMenuItem (2, coord, Coordinate::parentBottomMarkerName, menu, isAnchor1, fullCoordName);
    }

    menu.addSeparator();
    const MarkerList& markerList = document.getMarkerList (coord.isHorizontal());

    for (int i = 0; i < markerList.size(); ++i)
        document.addMarkerMenuItem (100 + i, coord, markerList.getName (markerList.getMarker (i)), menu, isAnchor1, fullCoordName);
}

const String DrawableDocument::MarkerList::getChosenMarkerMenuItem (const Coordinate& coord, int i) const
{
    if (i == 1)  return coord.isHorizontal() ? Coordinate::parentLeftMarkerName : Coordinate::parentTopMarkerName;
    if (i == 2)  return coord.isHorizontal() ? Coordinate::parentRightMarkerName : Coordinate::parentBottomMarkerName;

    const MarkerList& markerList = document.getMarkerList (coord.isHorizontal());

    if (i >= 100 && i < 10000)
        return markerList.getName (markerList.getMarker (i - 100));

    jassertfalse;
    return String::empty;
}

UndoManager* DrawableDocument::MarkerList::getUndoManager() const
{
    return document.getUndoManager();
}

const String DrawableDocument::MarkerList::getNonexistentMarkerName (const String& name)
{
    return document.getNonexistentMarkerName (name);
}

const String DrawableDocument::getNonexistentMarkerName (const String& name)
{
    String n (makeValidCppIdentifier (name, false, true, false));
    int suffix = 2;

    while (markersX->getMarkerNamed (n).isValid() || markersY->getMarkerNamed (n).isValid())
        n = n.trimCharactersAtEnd ("0123456789") + String (suffix++);

    return n;
}

void DrawableDocument::MarkerList::renameAnchor (const String& oldName, const String& newName)
{
    document.renameAnchor (oldName, newName);
}

void DrawableDocument::renameAnchor (const String& oldName, const String& newName)
{
}
