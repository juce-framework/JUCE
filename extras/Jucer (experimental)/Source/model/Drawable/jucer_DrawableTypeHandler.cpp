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

#include "jucer_DrawableTypeHandler.h"


//==============================================================================
class DrawablePathHandler : public DrawableTypeHandler
{
public:
    DrawablePathHandler()  : DrawableTypeHandler ("Polygon", DrawablePath::valueTreeType) {}
    ~DrawablePathHandler() {}

    static const ValueTree createNewPath (DrawableDocument& document, const Path& p)
    {
        DrawablePath dp;
        dp.setPath (p);
        dp.setFill (Colours::lightblue.withHue (Random::getSystemRandom().nextFloat()));
        return dp.createValueTree (0);
    }

    static const ValueTree createNewTriangle (DrawableDocument& document, const Point<float>& approxPosition)
    {
        Path p;
        p.addTriangle (approxPosition.getX(), approxPosition.getY() - 50.0f,
                       approxPosition.getX() + 50.0f, approxPosition.getY() + 20.0f,
                       approxPosition.getX() - 50.0f, approxPosition.getY() + 20.0f);

        return createNewPath (document, p);
    }

    static const ValueTree createNewRectangle (DrawableDocument& document, const Point<float>& approxPosition)
    {
        Path p;
        p.addRectangle (approxPosition.getX() - 50.0f, approxPosition.getY() - 50.0f,
                        100.0f, 100.0f);

        return createNewPath (document, p);
    }

    static const ValueTree createNewEllipse (DrawableDocument& document, const Point<float>& approxPosition)
    {
        Path p;
        p.addEllipse (approxPosition.getX() - 50.0f, approxPosition.getY() - 50.0f,
                      100.0f, 100.0f);

        return createNewPath (document, p);
    }

    class DrawablePathFillPropComp  : public FillTypePropertyComponent
    {
    public:
        DrawablePathFillPropComp (DrawableTypeInstance& item_, const String& name, const ValueTree& fill)
            : FillTypePropertyComponent (item_.getDocument().getUndoManager(), name, fill),
              item (item_)
        {}

        const ColourGradient getDefaultGradient()
        {
            const Rectangle<float> bounds (item.getBounds());

            return ColourGradient (Colours::blue,
                                   bounds.getX() + bounds.getWidth() * 0.3f,
                                   bounds.getY() + bounds.getHeight() * 0.3f,
                                   Colours::red,
                                   bounds.getX() + bounds.getWidth() * 0.7f,
                                   bounds.getY() + bounds.getHeight() * 0.7f,
                                   false);
        }

    private:
        DrawableTypeInstance item;
    };

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
        DrawablePath::ValueTreeWrapper wrapper (item.getState());

        props.add (new DrawablePathFillPropComp (item, "Fill", wrapper.getMainFillState()));
        props.add (new DrawablePathFillPropComp (item, "Stroke", wrapper.getStrokeFillState()));
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    //==============================================================================
    class GradientControlPoint  : public ControlPoint
    {
    public:
        GradientControlPoint (const ValueTree& item_,
                              const bool isStart_, const bool isStroke_)
            : item (item_), isStart (isStart_), isStroke (isStroke_)
        {}

        ~GradientControlPoint() {}

        const RelativePoint getPosition()
        {
            DrawablePath::ValueTreeWrapper wrapper (item);

            RelativePoint p;
            const FillType fill (Drawable::ValueTreeWrapperBase::readFillType (isStroke ? wrapper.getStrokeFillState() : wrapper.getMainFillState(),
                                                                               isStart ? &p : 0,
                                                                               isStart ? 0 : &p, 0));
            jassert (fill.isGradient());
            return p;
        }

        void setPosition (const RelativePoint& newPoint, UndoManager* undoManager)
        {
            DrawablePath::ValueTreeWrapper wrapper (item);

            RelativePoint p1, p2;
            ValueTree fillState (isStroke ? wrapper.getStrokeFillState() : wrapper.getMainFillState());
            const FillType fill (Drawable::ValueTreeWrapperBase::readFillType (fillState, &p1, &p2, 0));
            jassert (fill.isGradient());

            if (isStart)
                p1 = newPoint;
            else
                p2 = newPoint;

            Drawable::ValueTreeWrapperBase::writeFillType (fillState, fill, &p1, &p2, undoManager);
        }

        bool hasLine()                  { return false; }
        RelativePoint getEndOfLine()    { return RelativePoint(); }

    private:
        ValueTree item;
        bool isStart, isStroke;
    };

    //==============================================================================
    class PathControlPoint  : public ControlPoint
    {
    public:
        PathControlPoint (const DrawablePath::ValueTreeWrapper::Element& element_, const int cpNum_)
            : element (element_), cpNum (cpNum_)
        {}

        ~PathControlPoint() {}

        const RelativePoint getPosition()
        {
            return element.getControlPoint (cpNum);
        }

        void setPosition (const RelativePoint& newPoint, UndoManager* undoManager)
        {
            element.setControlPoint (cpNum, newPoint, undoManager);
        }

        bool hasLine()                  { return false; }
        RelativePoint getEndOfLine()    { return RelativePoint(); }

    private:
        DrawablePath::ValueTreeWrapper::Element element;
        int cpNum;
    };

    void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points)
    {
        DrawablePath::ValueTreeWrapper wrapper (item.getState());

        const ValueTree pathTree (wrapper.getPathState());
        const int numElements = pathTree.getNumChildren();

        for (int i = 0; i < numElements; ++i)
        {
            const DrawablePath::ValueTreeWrapper::Element e (pathTree.getChild(i));
            const int numCps = e.getNumControlPoints();

            for (int j = 0; j < numCps; ++j)
                points.add (new PathControlPoint (e, j));
        }

        const FillType fill (Drawable::ValueTreeWrapperBase::readFillType (wrapper.getMainFillState(), 0, 0, 0));

        if (fill.isGradient())
        {
            points.add (new GradientControlPoint (item.getState(), true, false));
            points.add (new GradientControlPoint (item.getState(), false, false));
        }

        const FillType stroke (Drawable::ValueTreeWrapperBase::readFillType (wrapper.getStrokeFillState(), 0, 0, 0));

        if (stroke.isGradient())
        {
            points.add (new GradientControlPoint (item.getState(), true, true));
            points.add (new GradientControlPoint (item.getState(), false, true));
        }
    }
};

//==============================================================================
class DrawableImageHandler : public DrawableTypeHandler
{
public:
    DrawableImageHandler()  : DrawableTypeHandler ("Image", DrawableImage::valueTreeType) {}
    ~DrawableImageHandler() {}

    static const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition)
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

    //==============================================================================
    class ImageControlPoint  : public ControlPoint
    {
    public:
        ImageControlPoint (const DrawableTypeInstance& item_, const int cpNum_)
            : item (item_), cpNum (cpNum_)
        {}

        ~ImageControlPoint() {}

        const RelativePoint getPosition()
        {
            DrawableImage::ValueTreeWrapper wrapper (item.getState());

            switch (cpNum)
            {
                case 0: return wrapper.getTargetPositionForTopLeft();
                case 1: return wrapper.getTargetPositionForTopRight();
                case 2: return wrapper.getTargetPositionForBottomLeft();
                default: jassertfalse; break;
            }

            return RelativePoint();
        }

        void setPosition (const RelativePoint& newPoint, UndoManager* undoManager)
        {
            DrawableImage::ValueTreeWrapper wrapper (item.getState());

            switch (cpNum)
            {
                case 0: wrapper.setTargetPositionForTopLeft (newPoint, undoManager); break;
                case 1: wrapper.setTargetPositionForTopRight (newPoint, undoManager); break;
                case 2: wrapper.setTargetPositionForBottomLeft (newPoint, undoManager); break;
                default: jassertfalse; break;
            }
        }

        bool hasLine()                  { return false; }
        RelativePoint getEndOfLine()    { return RelativePoint(); }

    private:
        DrawableTypeInstance item;
        int cpNum;
    };

    void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points)
    {
        for (int i = 0; i < 3; ++i)
            points.add (new ImageControlPoint (item, i));
    }
};

//==============================================================================
class DrawableCompositeHandler : public DrawableTypeHandler
{
public:
    DrawableCompositeHandler()  : DrawableTypeHandler ("Group", DrawableComposite::valueTreeType) {}
    ~DrawableCompositeHandler() {}

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    const RelativeCoordinate findNamedCoordinate (const DrawableTypeInstance& item, const String& objectName, const String& edge) const
    {
        DrawableComposite::ValueTreeWrapper wrapper (const_cast <DrawableTypeInstance&> (item).getState());

        ValueTree markerState (wrapper.getMarkerState (true, objectName));
        if (markerState.isValid())
            return wrapper.getMarker (true, markerState).position;

        markerState = wrapper.getMarkerState (false, objectName);
        if (markerState.isValid())
            return wrapper.getMarker (false, markerState).position;

        return RelativeCoordinate();
    }

    //==============================================================================
    class CompositeControlPoint  : public ControlPoint
    {
    public:
        CompositeControlPoint (const ValueTree& item_, const int cpNum_)
            : item (item_), cpNum (cpNum_)
        {}

        ~CompositeControlPoint() {}

        const RelativePoint getPosition()
        {
            DrawableComposite::ValueTreeWrapper wrapper (item);

            switch (cpNum)
            {
                case 0: return wrapper.getTargetPositionForOrigin();
                case 1: return wrapper.getTargetPositionForX1Y0();
                case 2: return wrapper.getTargetPositionForX0Y1();
                default: jassertfalse; break;
            }

            return RelativePoint();
        }

        void setPosition (const RelativePoint& newPoint, UndoManager* undoManager)
        {
            DrawableComposite::ValueTreeWrapper wrapper (item);

            switch (cpNum)
            {
                case 0: wrapper.setTargetPositionForOrigin (newPoint, undoManager); break;
                case 1: wrapper.setTargetPositionForX1Y0 (newPoint, undoManager); break;
                case 2: wrapper.setTargetPositionForX0Y1 (newPoint, undoManager); break;
                default: jassertfalse; break;
            }
        }

        bool hasLine()                  { return false; }
        RelativePoint getEndOfLine()    { return RelativePoint(); }

    private:
        ValueTree item;
        int cpNum;
    };

    void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points)
    {
        for (int i = 0; i < 3; ++i)
            points.add (new CompositeControlPoint (item.getState(), i));
    }
};


//==============================================================================
DrawableTypeManager::DrawableTypeManager()
{
    handlers.add (new DrawablePathHandler());
    handlers.add (new DrawableImageHandler());
    handlers.add (new DrawableCompositeHandler());
}

DrawableTypeManager::~DrawableTypeManager()
{
}

DrawableTypeHandler* DrawableTypeManager::getHandlerFor (const Identifier& type)
{
    for (int i = handlers.size(); --i >= 0;)
        if (handlers.getUnchecked(i)->getValueTreeType() == type)
            return handlers.getUnchecked(i);

    jassertfalse;
    return 0;
}

const StringArray DrawableTypeManager::getNewItemList()
{
    const char* types[] = { "New Triangle", "New Rectangle", "New Ellipse", "New Image", 0 };
    return StringArray (types);
}

const ValueTree DrawableTypeManager::createNewItem (const int index, DrawableDocument& document, const Point<float>& approxPosition)
{
    switch (index)
    {
        case 0: return DrawablePathHandler::createNewTriangle (document, approxPosition);
        case 1: return DrawablePathHandler::createNewRectangle (document, approxPosition);
        case 2: return DrawablePathHandler::createNewEllipse (document, approxPosition);
        case 3: return DrawableImageHandler::createNewInstance (document, approxPosition);
        default: jassertfalse; break;
    }

    return ValueTree::invalid;
}

juce_ImplementSingleton_SingleThreaded (DrawableTypeManager);


//==============================================================================
DrawableTypeInstance::DrawableTypeInstance (DrawableDocument& document_, const ValueTree& state_)
    : document (document_), state (state_)
{
}

Value DrawableTypeInstance::getValue (const Identifier& name) const
{
    return state.getPropertyAsValue (name, document.getUndoManager());
}

void DrawableTypeInstance::createProperties (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getValue (Drawable::ValueTreeWrapperBase::idProperty), "Object ID", 128, false));

    getHandler()->createPropertyEditors (*this, props);
}

DrawableTypeHandler* DrawableTypeInstance::getHandler() const
{
    DrawableTypeHandler* h = DrawableTypeManager::getInstance()->getHandlerFor (state.getType());
    jassert (h != 0);
    return h;
}

const RelativeCoordinate DrawableTypeInstance::findNamedCoordinate (const String& objectName, const String& edge) const
{
    return getHandler()->findNamedCoordinate (*this, objectName, edge);
}

const Rectangle<float> DrawableTypeInstance::getBounds()
{
    OwnedArray <ControlPoint> points;
    getAllControlPoints (points);

    if (points.size() < 2)
        return Rectangle<float>();

    DrawableTypeInstance parent (document, state.getParent());
    const Point<float> p1 (points.getUnchecked(0)->getPosition().resolve (&parent));
    Rectangle<float> r (p1, points.getUnchecked(1)->getPosition().resolve (&parent));

    for (int i = 2; i < points.size(); ++i)
        r = r.getUnion (Rectangle<float> (p1, points.getUnchecked(i)->getPosition().resolve (&parent)));

    return r;
}

void DrawableTypeInstance::setBounds (Drawable* drawable, const Rectangle<float>& newBounds)
{
    return getHandler()->setBounds (*this, drawable, newBounds);
}

void DrawableTypeInstance::getAllControlPoints (OwnedArray <ControlPoint>& points)
{
    return getHandler()->getAllControlPoints (*this, points);
}
