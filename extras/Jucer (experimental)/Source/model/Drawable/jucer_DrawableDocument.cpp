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
#include "jucer_DrawableTypeHandler.h"


//==============================================================================
namespace Tags
{
    const Identifier drawableTag ("DRAWABLE");
    const Identifier markersGroupXTag ("MARKERS_X");
    const Identifier markersGroupYTag ("MARKERS_Y");
}

//==============================================================================
DrawableDocument::DrawableDocument (Project* project_)
    : project (project_),
      root (Tags::drawableTag),
      saveAsXml (true),
      needsSaving (false)
{
    DrawableComposite dc;
    root.addChild (dc.createValueTree (0), -1, 0);

    setName ("Drawable");
    checkRootObject();

    root.addListener (this);
}

DrawableDocument::~DrawableDocument()
{
    root.removeListener (this);
}

void DrawableDocument::recursivelyUpdateIDs (Drawable::ValueTreeWrapperBase& d, StringArray& recentlyUsedIdCache)
{
    if (d.getID().isEmpty())
        d.setID (createUniqueID (d.getState().getType().toString().toLowerCase() + "1", recentlyUsedIdCache), 0);

    if (d.getState().getType() == DrawableComposite::valueTreeType)
    {
        const DrawableComposite::ValueTreeWrapper composite (d.getState());

        for (int i = 0; i < composite.getNumDrawables(); ++i)
        {
            Drawable::ValueTreeWrapperBase child (composite.getDrawableState (i));
            recursivelyUpdateIDs (child, recentlyUsedIdCache);
        }
    }
}

void DrawableDocument::checkRootObject()
{
    if (! root.hasProperty (Ids::id_))
        root.setProperty (Ids::id_, createAlphaNumericUID(), 0);

    if (markersX == 0)
        markersX = new MarkerList (*this, true);

    if (markersY == 0)
        markersY = new MarkerList (*this, false);

    DrawableComposite::ValueTreeWrapper rootObject (getRootDrawableNode());
    StringArray idCache;
    recursivelyUpdateIDs (rootObject, idCache);
}

const String DrawableDocument::getUniqueId() const
{
    return root [Ids::id_];
}

//==============================================================================
void DrawableDocument::setName (const String& name)
{
    root.setProperty (Ids::name, name, getUndoManager());
}

const String DrawableDocument::getName() const
{
    return root [Ids::name];
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
    ValueTree loadedTree;

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

    if (loadedTree.hasType (Tags::drawableTag))
    {
        root.removeListener (this);
        root = loadedTree;
        root.addListener (this);
        markersX = 0;
        markersY = 0;

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
}

DrawableComposite::ValueTreeWrapper DrawableDocument::getRootDrawableNode() const
{
    return DrawableComposite::ValueTreeWrapper (root.getChild (0));
}

ValueTree DrawableDocument::findDrawableState (const String& objectId, bool recursive) const
{
    return getRootDrawableNode().getDrawableWithId (objectId, recursive);
}

const String DrawableDocument::createUniqueID (const String& name, StringArray& recentlyUsedIdCache) const
{
    String n (CodeHelpers::makeValidIdentifier (name, false, true, false));
    int suffix = 2;
    int cacheIndex = -1;

    const String withoutNumbers (n.trimCharactersAtEnd ("0123456789"));

    for (int i = 0; i < recentlyUsedIdCache.size(); ++i)
    {
        if (recentlyUsedIdCache[i].startsWith (withoutNumbers))
        {
            cacheIndex = i;
            suffix = jmax (suffix, recentlyUsedIdCache[i].substring (withoutNumbers.length()).getIntValue() + 1);
            n = withoutNumbers + String (suffix++);
            break;
        }
    }

    while (markersX->getMarkerNamed (n).isValid() || markersY->getMarkerNamed (n).isValid()
            || findDrawableState (n, true).isValid())
        n = withoutNumbers + String (suffix++);

    if (cacheIndex >= 0)
        recentlyUsedIdCache.set (cacheIndex, n);
    else
        recentlyUsedIdCache.add (n);

    return n;
}

bool DrawableDocument::createItemProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree drawable (findDrawableState (itemId.upToFirstOccurrenceOf ("/", false, false), false));

    if (drawable.isValid())
    {
        DrawableTypeInstance item (*this, drawable);

        if (itemId.containsChar ('/'))
        {
            OwnedArray <ControlPoint> points;
            item.getAllControlPoints (points);

            for (int i = 0; i < points.size(); ++i)
                if (points.getUnchecked(i)->getID() == itemId)
                    points.getUnchecked(i)->createProperties (*this, props);
        }
        else
        {
            item.createProperties (props);
        }

        return true;
    }

    if (markersX->createProperties (props, itemId)
         || markersY->createProperties (props, itemId))
        return true;

    return false;
}

void DrawableDocument::createItemProperties (Array <PropertyComponent*>& props, const StringArray& selectedItemIds)
{
    if (selectedItemIds.size() != 1)
        return; //xxx

    for (int i = 0; i < selectedItemIds.size(); ++i)
        createItemProperties (props, selectedItemIds[i]);
}

//==============================================================================
const int menuItemOffset = 0x63451fa4;

void DrawableDocument::addNewItemMenuItems (PopupMenu& menu) const
{
    const StringArray newItems (DrawableTypeManager::getInstance()->getNewItemList());

    for (int i = 0; i < newItems.size(); ++i)
        menu.addItem (i + menuItemOffset, newItems[i]);
}

const ValueTree DrawableDocument::performNewItemMenuItem (int menuResultCode)
{
    const StringArray newItems (DrawableTypeManager::getInstance()->getNewItemList());

    int index = menuResultCode - menuItemOffset;
    if (index >= 0 && index < newItems.size())
    {
        ValueTree state (DrawableTypeManager::getInstance()
                            ->createNewItem (index, *this,
                                             Point<float> (Random::getSystemRandom().nextFloat() * 100.0f + 100.0f,
                                                           Random::getSystemRandom().nextFloat() * 100.0f + 100.0f)));

        Drawable::ValueTreeWrapperBase wrapper (state);
        StringArray idCache;
        recursivelyUpdateIDs (wrapper, idCache);
        getRootDrawableNode().addDrawable (state, -1, getUndoManager());

        return state;
    }

    return ValueTree::invalid;
}

const ValueTree DrawableDocument::insertSVG (const File& file, const Point<float>& position)
{
    ScopedPointer<Drawable> d (Drawable::createFromImageFile (file));
    DrawableComposite* dc = dynamic_cast <DrawableComposite*> (static_cast <Drawable*> (d));

    if (dc != 0)
    {
        ValueTree state (dc->createValueTree (this));

        if (state.isValid())
        {
            Drawable::ValueTreeWrapperBase wrapper (state);
            getRootDrawableNode().addDrawable (state, -1, getUndoManager());
            StringArray idCache;
            recursivelyUpdateIDs (wrapper, idCache);

            return state;
        }
    }

    return ValueTree::invalid;
}

//==============================================================================
const Image DrawableDocument::getImageForIdentifier (const var& imageIdentifier)
{
    const String s (imageIdentifier.toString());

    if (s.startsWithIgnoreCase ("id:"))
    {
        jassert (project != 0);

        if (project != 0)
        {
            Project::Item item (project->getMainGroup().findItemWithID (s.substring (3).trim()));

            if (item.isValid())
            {
                Image im (ImageCache::getFromFile (item.getFile()));

                if (im.isValid())
                {
                    im.setTag (imageIdentifier);
                    return im;
                }
            }
        }
    }

    static Image dummy;

    if (dummy.isNull())
    {
        dummy = Image (Image::ARGB, 128, 128, true);
        Graphics g (dummy);
        g.fillAll (Colours::khaki.withAlpha (0.51f));
        g.setColour (Colours::grey);
        g.drawRect (0, 0, 128, 128);

        for (int i = -128; i < 128; i += 16)
            g.drawLine (i, 0, i + 128, 128);

        g.setColour (Colours::darkgrey);
        g.drawRect (0, 0, 128, 128);
        g.setFont (16.0f, Font::bold);
        g.drawText ("(Image Missing)", 0, 0, 128, 128, Justification::centred, false);
    }

    return dummy;
}

const var DrawableDocument::getIdentifierForImage (const Image& image)
{
    return image.getTag();
}

//==============================================================================
void DrawableDocument::valueTreePropertyChanged (ValueTree& tree, const Identifier& name)
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
const RelativeCoordinate DrawableDocument::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == "parent")
    {
        jassert (edge != "right" && edge != "bottom"); // drawables don't have a canvas size..
    }

    if (objectName.isNotEmpty() && edge.isNotEmpty())
    {
    }

    {
        const ValueTree marker (getMarkerListX().getMarkerNamed (objectName));
        if (marker.isValid())
            return getMarkerListX().getCoordinate (marker);
    }

    {
        const ValueTree marker (getMarkerListY().getMarkerNamed (objectName));
        if (marker.isValid())
            return getMarkerListY().getCoordinate (marker);
    }

    return RelativeCoordinate();
}

//==============================================================================
DrawableDocument::MarkerList::MarkerList (DrawableDocument& document_, bool isX_)
    : MarkerListBase (isX_),
      document (document_),
      object (document_.getRootDrawableNode())
{
}

const String DrawableDocument::MarkerList::getId (const ValueTree& markerState)
{
    return markerState [DrawableComposite::ValueTreeWrapper::nameProperty];
}

int DrawableDocument::MarkerList::size() const
{
    return object.getNumMarkers (isX);
}

ValueTree DrawableDocument::MarkerList::getMarker (int index) const
{
    return object.getMarkerState (isX, index);
}

ValueTree DrawableDocument::MarkerList::getMarkerNamed (const String& name) const
{
    return object.getMarkerState (isX, name);
}

bool DrawableDocument::MarkerList::contains (const ValueTree& markerState) const
{
    return object.containsMarker (isX, markerState);
}

void DrawableDocument::MarkerList::createMarker (const String& name, int position)
{
    object.setMarker (isX, DrawableComposite::Marker (name, RelativeCoordinate ((double) position, isX)),
                      getUndoManager());
}

void DrawableDocument::MarkerList::deleteMarker (ValueTree& markerState)
{
    object.removeMarker (isX, markerState, getUndoManager());
}

const RelativeCoordinate DrawableDocument::MarkerList::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == "parent")
    {
        jassert (edge != "right" && edge != "bottom"); // drawables don't have a canvas size..
    }

    const ValueTree marker (getMarkerNamed (objectName));
    if (marker.isValid())
        return getCoordinate (marker);

    return RelativeCoordinate();
}

bool DrawableDocument::MarkerList::createProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree marker (getMarkerNamed (itemId));

    if (marker.isValid())
    {
        props.add (new TextPropertyComponent (marker.getPropertyAsValue (DrawableComposite::ValueTreeWrapper::nameProperty, getUndoManager()),
                                              "Marker Name", 256, false));

        props.add (new MarkerListBase::PositionPropertyComponent (*this, "Position", marker,
                                                                  marker.getPropertyAsValue (DrawableComposite::ValueTreeWrapper::posProperty, getUndoManager())));
        return true;
    }

    return false;
}

void DrawableDocument::MarkerList::addMarkerMenuItem (int i, const RelativeCoordinate& coord, const String& name, const String& edge, PopupMenu& menu,
                                                      bool isAnchor1, const String& fullCoordName)
{
    RelativeCoordinate requestedCoord (findNamedCoordinate (name, edge));

    menu.addItem (i, edge.isEmpty() ? name : (name + "." + edge),
                  ! (name == fullCoordName || (fullCoordName.isNotEmpty() && requestedCoord.references (fullCoordName, this))),
                  name == (isAnchor1 ? coord.getAnchorName1() : coord.getAnchorName2()));
}

void DrawableDocument::MarkerList::addMarkerMenuItems (const ValueTree& markerState, const RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1)
{
    const String fullCoordName (getName (markerState));

    if (isHorizontal())
        addMarkerMenuItem (1, coord, "parent", "left", menu, isAnchor1, fullCoordName);
    else
        addMarkerMenuItem (1, coord, "parent", "top", menu, isAnchor1, fullCoordName);

    menu.addSeparator();

    for (int i = 0; i < size(); ++i)
        addMarkerMenuItem (100 + i, coord, getName (getMarker (i)),
                           String::empty, menu, isAnchor1, fullCoordName);
}

const String DrawableDocument::MarkerList::getChosenMarkerMenuItem (const RelativeCoordinate& coord, int i) const
{
    if (i == 1)  return isHorizontal() ? "parent.left" : "parent.top";

    if (i >= 100 && i < 10000)
        return getName (getMarker (i - 100));

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
    String n (CodeHelpers::makeValidIdentifier (name, false, true, false));
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
