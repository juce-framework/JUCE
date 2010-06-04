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

#ifndef __JUCER_DRAWABLETYPEHANDLER_H_7FB02E2F__
#define __JUCER_DRAWABLETYPEHANDLER_H_7FB02E2F__

#include "jucer_DrawableDocument.h"
#include "../../utility/jucer_FillTypePropertyComponent.h"
#include "../../ui/Editor Base/jucer_EditorCanvas.h"
class DrawableTypeHandler;


//==============================================================================
class ControlPoint
{
public:
    ControlPoint (const String& pointID_) : pointID (pointID_) {}
    virtual ~ControlPoint() {}

    const String& getID() const throw()         { return pointID; }

    virtual const RelativePoint getPosition() = 0;
    virtual void setPosition (const RelativePoint& newPoint, UndoManager* undoManager) = 0;

    virtual bool hasLine() = 0;
    virtual RelativePoint getEndOfLine() = 0;

    virtual const Value getPositionValue (UndoManager* undoManager) = 0;
    virtual void createProperties (DrawableDocument& document, Array <PropertyComponent*>& props) = 0;

private:
    const String pointID;

    ControlPoint (const ControlPoint&);
    ControlPoint& operator= (const ControlPoint&);
};


//==============================================================================
class DrawableTypeInstance  : public RelativeCoordinate::NamedCoordinateFinder
{
public:
    DrawableTypeInstance (DrawableDocument& document_, const ValueTree& state_);

    //==============================================================================
    DrawableDocument& getDocument() throw()             { return document; }
    Project* getProject()                               { return document.getProject(); }
    ValueTree& getState() throw()                       { return state; }
    const String getID() const                          { return Drawable::ValueTreeWrapperBase (state).getID(); }

    Value getValue (const Identifier& name) const;
    void createProperties (Array <PropertyComponent*>& props);
    const Rectangle<float> getBounds();
    void setBounds (Drawable* drawable, const Rectangle<float>& newBounds);
    void applyTransform (Drawable* drawable, const AffineTransform& transform);
    void getAllControlPoints (OwnedArray <ControlPoint>& points);
    void getVisibleControlPoints (OwnedArray <ControlPoint>& points, const EditorCanvasBase::SelectedItems& selection);

    const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const;

    //==============================================================================
    DrawableTypeHandler* getHandler() const;

private:
    //==============================================================================
    DrawableDocument& document;
    ValueTree state;

    DrawableTypeInstance& operator= (const DrawableTypeInstance&);
};

//==============================================================================
class DrawableTypeHandler
{
public:
    DrawableTypeHandler (const String& displayName_, const Identifier& valueTreeType_)
        : displayName (displayName_), valueTreeType (valueTreeType_)
    {
    }

    virtual ~DrawableTypeHandler() {}

    virtual void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props) = 0;
    virtual void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item) = 0;
    virtual void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points) = 0;
    virtual void getVisibleControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points, const EditorCanvasBase::SelectedItems& selection) = 0;

    const String& getDisplayName() const        { return displayName; }
    const Identifier& getValueTreeType() const  { return valueTreeType; }

    void setBounds (DrawableTypeInstance& item, Drawable* drawable, Rectangle<float> newBounds)
    {
        const Rectangle<float> oldBounds (drawable->getBounds());
        if (oldBounds.isEmpty())
            return;

        newBounds.setSize (jmax (1.0f, newBounds.getWidth()),
                           jmax (1.0f, newBounds.getHeight()));

        const double tolerance = 0.001;

        double xScale = newBounds.getWidth() / (double) oldBounds.getWidth();
        double yScale = newBounds.getHeight() / (double) oldBounds.getHeight();

        if (std::abs (xScale - 1.0) < tolerance)  xScale = 1.0;
        if (std::abs (yScale - 1.0) < tolerance)  yScale = 1.0;

        if (xScale == 1.0 && yScale == 1.0
             && std::abs (newBounds.getX() - oldBounds.getX()) < tolerance
             && std::abs (newBounds.getY() - oldBounds.getY()) < tolerance)
            return;

        const double xOffset = newBounds.getX() - xScale * oldBounds.getX();
        const double yOffset = newBounds.getY() - yScale * oldBounds.getY();

        OwnedArray<ControlPoint> points;
        getAllControlPoints (item, points);

        RelativeCoordinate::NamedCoordinateFinder* const nameFinder = drawable->getParent();
        UndoManager* undoManager = item.getDocument().getUndoManager();

        for (int i = 0; i < points.size(); ++i)
        {
            ControlPoint* cp = points.getUnchecked(i);
            RelativePoint point (cp->getPosition());
            const Point<float> p (point.resolve (nameFinder));

            point.moveToAbsolute (Point<float> ((float) (xOffset + xScale * p.getX()),
                                                (float) (yOffset + yScale * p.getY())), nameFinder);

            cp->setPosition (point, undoManager);
        }
    }

private:
    const String displayName;
    const Identifier valueTreeType;

    DrawableTypeHandler& operator= (const DrawableTypeHandler&);
};


//==============================================================================
class DrawableTypeManager   : public DeletedAtShutdown
{
public:
    DrawableTypeManager();
    ~DrawableTypeManager();

    juce_DeclareSingleton_SingleThreaded_Minimal (DrawableTypeManager);

    //==============================================================================
    int getNumHandlers() const                                      { return handlers.size(); }
    DrawableTypeHandler* getHandler (const int index) const         { return handlers [index]; }

    DrawableTypeHandler* getHandlerFor (const Identifier& type);

    const StringArray getNewItemList();
    const ValueTree createNewItem (const int index, DrawableDocument& document, const Point<float>& approxPosition);

private:
    OwnedArray <DrawableTypeHandler> handlers;
};


#endif  // __JUCER_DRAWABLETYPEHANDLER_H_7FB02E2F__
