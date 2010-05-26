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
class DrawableTypeHandler;


//==============================================================================
class DrawableTypeInstance
{
public:
    DrawableTypeInstance (DrawableDocument& document_, const ValueTree& state_);

    //==============================================================================
    DrawableDocument& getDocument() throw()             { return document; }
    ValueTree& getState() throw()                       { return state; }

    Value getValue (const Identifier& name) const;
    void createProperties (Array <PropertyComponent*>& props);
    void setBounds (Drawable* drawable, const Rectangle<float>& newBounds);
    void getAllControlPoints (Array <RelativePoint>& points);

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
    DrawableTypeHandler (const String& displayName_, const Identifier& valueTreeType_, bool canBeCreated_)
        : displayName (displayName_), valueTreeType (valueTreeType_), canBeCreated (canBeCreated_)
    {
    }

    virtual ~DrawableTypeHandler() {}

    virtual const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition) = 0;
    virtual void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props) = 0;
    virtual void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item) = 0;
    virtual void setBounds (DrawableTypeInstance& item, Drawable* drawable, const Rectangle<float>& newBounds) = 0;
    virtual void getAllControlPoints (DrawableTypeInstance& item, Array <RelativePoint>& points) = 0;

    const String& getDisplayName() const        { return displayName; }
    const Identifier& getValueTreeType() const  { return valueTreeType; }

    const bool canBeCreated;

protected:
    static bool rescalePoints (RelativePoint* const points, const int numPoints,
                               const Rectangle<float>& oldBounds, Rectangle<float> newBounds,
                               RelativeCoordinate::NamedCoordinateFinder* nameFinder)
    {
        if (oldBounds.isEmpty())
            return false;

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
            return false;

        const double xOffset = newBounds.getX() - xScale * oldBounds.getX();
        const double yOffset = newBounds.getY() - yScale * oldBounds.getY();

        for (int i = 0; i < numPoints; ++i)
        {
            const Point<float> p (points[i].resolve (nameFinder));

            points[i].moveToAbsolute (Point<float> ((float) (xOffset + xScale * p.getX()),
                                                    (float) (yOffset + yScale * p.getY())), nameFinder);
        }

        return true;
    }

private:
    const String displayName;
    const Identifier valueTreeType;

    DrawableTypeHandler& operator= (const DrawableTypeHandler&);
};

//==============================================================================
class DrawablePathHandler : public DrawableTypeHandler
{
public:
    DrawablePathHandler()  : DrawableTypeHandler ("Polygon", DrawablePath::valueTreeType, true) {}
    ~DrawablePathHandler() {}

    const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition)
    {
        Path p;
        p.addTriangle (approxPosition.getX(), approxPosition.getY() - 50.0f,
                       approxPosition.getX() + 50.0f, approxPosition.getY() + 20.0f,
                       approxPosition.getX() - 50.0f, approxPosition.getY() + 20.0f);

        DrawablePath dp;
        dp.setPath (p);
        dp.setFill (Colours::lightblue.withHue (Random::getSystemRandom().nextFloat()));
        return dp.createValueTree (0);
    }

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
        DrawablePath::ValueTreeWrapper wrapper (item.getState());

        props.add (new FillTypePropertyComponent (item.getDocument().getUndoManager(),
                                                  "Fill", wrapper.getMainFillState()));

        props.add (new FillTypePropertyComponent (item.getDocument().getUndoManager(),
                                                  "Stroke", wrapper.getStrokeFillState()));
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    void setBounds (DrawableTypeInstance& item, Drawable* drawable, const Rectangle<float>& newBounds)
    {
        DrawablePath::ValueTreeWrapper wrapper (item.getState());

        RelativePointPath path;
        wrapper.getPath (path);

        Array <RelativePoint> points;
        int i;
        for (i = 0; i < path.elements.size(); ++i)
        {
            int numPoints;
            RelativePoint* elementPoints = path.elements.getUnchecked(i)->getControlPoints (numPoints);

            for (int j = 0; j < numPoints; ++j)
                points.add (elementPoints[j]);
        }

        if (rescalePoints (points.getRawDataPointer(), points.size(),
                           drawable->getBounds(), newBounds, drawable->getParent()))
        {
            int n = 0;
            for (i = 0; i < path.elements.size(); ++i)
            {
                int numPoints;
                RelativePoint* elementPoints = path.elements.getUnchecked(i)->getControlPoints (numPoints);

                for (int j = 0; j < numPoints; ++j)
                    elementPoints[j] = points [n++];
            }

            wrapper.setPath (path.toString(), item.getDocument().getUndoManager());
        }
    }

    void getAllControlPoints (DrawableTypeInstance& item, Array <RelativePoint>& points)
    {
        DrawablePath::ValueTreeWrapper wrapper (item.getState());

        RelativePointPath path;
        wrapper.getPath (path);

        for (int i = 0; i < path.elements.size(); ++i)
        {
            int numPoints;
            RelativePoint* elementPoints = path.elements.getUnchecked(i)->getControlPoints (numPoints);

            for (int j = 0; j < numPoints; ++j)
                points.add (elementPoints[j]);
        }
    }
};

//==============================================================================
class DrawableImageHandler : public DrawableTypeHandler
{
public:
    DrawableImageHandler()  : DrawableTypeHandler ("Image", DrawableImage::valueTreeType, true) {}
    ~DrawableImageHandler() {}

    const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition)
    {
        Image tempImage (Image::ARGB, 100, 100, true);

        {
            Graphics g (tempImage);
            g.fillAll (Colours::grey.withAlpha (0.3f));
            g.setColour (Colours::red);
            g.setFont (40.0f);
            g.drawText ("?", 0, 0, 100, 100, Justification::centred, false);
        }

        DrawableImage di;
        di.setTransform (RelativePoint (approxPosition),
                         RelativePoint (approxPosition + Point<float> (100.0f, 0.0f)),
                         RelativePoint (approxPosition + Point<float> (0.0f, 100.0f)));
        return di.createValueTree (&document);
    }

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    void setBounds (DrawableTypeInstance& item, Drawable* drawable, const Rectangle<float>& newBounds)
    {
        DrawableImage::ValueTreeWrapper wrapper (item.getState());

        RelativePoint points[3] = { wrapper.getTargetPositionForTopLeft(),
                                    wrapper.getTargetPositionForTopRight(),
                                    wrapper.getTargetPositionForBottomLeft() };

        if (rescalePoints (points, 3, drawable->getBounds(), newBounds, drawable->getParent()))
        {
            UndoManager* undoManager = item.getDocument().getUndoManager();
            wrapper.setTargetPositionForTopLeft (points[0], undoManager);
            wrapper.setTargetPositionForTopRight (points[1], undoManager);
            wrapper.setTargetPositionForBottomLeft (points[2], undoManager);
        }
    }

    void getAllControlPoints (DrawableTypeInstance& item, Array <RelativePoint>& points)
    {
        DrawableImage::ValueTreeWrapper wrapper (item.getState());

        points.add (wrapper.getTargetPositionForTopLeft());
        points.add (wrapper.getTargetPositionForTopRight());
        points.add (wrapper.getTargetPositionForBottomLeft());
    }
};

//==============================================================================
class DrawableCompositeHandler : public DrawableTypeHandler
{
public:
    DrawableCompositeHandler()  : DrawableTypeHandler ("Group", DrawableComposite::valueTreeType, false) {}
    ~DrawableCompositeHandler() {}

    const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition)
    {
        return ValueTree::invalid;
    }

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    void setBounds (DrawableTypeInstance& item, Drawable* drawable, const Rectangle<float>& newBounds)
    {
        DrawableComposite::ValueTreeWrapper wrapper (item.getState());

        RelativePoint points[3] = { wrapper.getTargetPositionForOrigin(),
                                    wrapper.getTargetPositionForX1Y0(),
                                    wrapper.getTargetPositionForX0Y1() };

        if (rescalePoints (points, 3, drawable->getBounds(), newBounds, drawable->getParent()))
        {
            UndoManager* undoManager = item.getDocument().getUndoManager();
            wrapper.setTargetPositionForOrigin (points[0], undoManager);
            wrapper.setTargetPositionForX1Y0 (points[1], undoManager);
            wrapper.setTargetPositionForX0Y1 (points[2], undoManager);
        }
    }

    void getAllControlPoints (DrawableTypeInstance& item, Array <RelativePoint>& points)
    {
        DrawableComposite::ValueTreeWrapper wrapper (item.getState());

        points.add (wrapper.getTargetPositionForOrigin());
        points.add (wrapper.getTargetPositionForX1Y0());
        points.add (wrapper.getTargetPositionForX0Y1());
    }
};

#endif  // __JUCER_DRAWABLETYPEHANDLER_H_7FB02E2F__
