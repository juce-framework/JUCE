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
#include "../../utility/jucer_ColourPropertyComponent.h"
#include "../../utility/jucer_FontPropertyComponent.h"

//==============================================================================
class ControlPointPropertyComp  : public CoordinatePropertyComponent
{
public:
    ControlPointPropertyComp (DrawableTypeInstance& item_, ControlPoint* cp, const String& name, bool isHorizontal_, UndoManager* undoManager)
        : CoordinatePropertyComponent (0, name, Value (new CoordExtractor (cp->getPositionValue (undoManager), isHorizontal_)), isHorizontal_),
          item (item_)
    {
        nameSource = &item;
    }

    ~ControlPointPropertyComp()
    {
    }

    const String pickMarker (TextButton* button, const String& currentMarker, bool isAnchor1)
    {
        RelativeCoordinate coord (getCoordinate());

        PopupMenu m;
        item.getDocument().getMarkerList (isHorizontal).addMarkerMenuItems (ValueTree::invalid, coord, m, isAnchor1);

        const int r = m.showAt (button);

        if (r > 0)
            return item.getDocument().getMarkerList (isHorizontal).getChosenMarkerMenuItem (coord, r);

        return String::empty;
    }

    DrawableTypeInstance item;


    //==============================================================================
    class CoordExtractor   : public Value::ValueSource,
                             public Value::Listener
    {
    public:
        CoordExtractor (const Value& sourceValue_, const bool isX_)
           : sourceValue (sourceValue_), isX (isX_)
        {
            sourceValue.addListener (this);
        }

        ~CoordExtractor() {}

        const var getValue() const
        {
            RelativePoint p (sourceValue.toString());
            return getCoord (p).toString();
        }

        void setValue (const var& newValue)
        {
            RelativePoint p (sourceValue.toString());
            RelativeCoordinate& coord = getCoord (p);
            coord = RelativeCoordinate (newValue.toString(), isX);

            const String newVal (p.toString());
            if (sourceValue != newVal)
                sourceValue = newVal;
        }

        void valueChanged (Value&)
        {
            sendChangeMessage (true);
        }

        //==============================================================================
        juce_UseDebuggingNewOperator

    protected:
        Value sourceValue;
        bool isX;

        RelativeCoordinate& getCoord (RelativePoint& p) const
        {
            return isX ? p.x : p.y;
        }

        CoordExtractor (const CoordExtractor&);
        const CoordExtractor& operator= (const CoordExtractor&);
    };
};

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
            : FillTypePropertyComponent (item_.getDocument().getUndoManager(), name, fill,
                                         &item_.getDocument(), item_.getProject()),
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

        props.add (StrokeThicknessValueSource::create (wrapper, item.getUndoManager()));
        props.add (StrokeJoinStyleValueSource::create (wrapper, item.getUndoManager()));
        props.add (StrokeCapStyleValueSource::create (wrapper, item.getUndoManager()));
        props.add (new DrawablePathFillPropComp (item, "Stroke", wrapper.getStrokeFillState()));
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    //==============================================================================
    class GradientControlPoint  : public ControlPoint
    {
    public:
        GradientControlPoint (const String& id_, const ValueTree& item_,
                              const bool isStart_, const bool isStroke_)
            : ControlPoint (id_), item (item_), isStart (isStart_), isStroke (isStroke_)
        {}

        ~GradientControlPoint() {}

        const RelativePoint getPosition()
        {
            DrawablePath::ValueTreeWrapper wrapper (item);

            RelativePoint p;
            const FillType fill (Drawable::ValueTreeWrapperBase::readFillType (isStroke ? wrapper.getStrokeFillState() : wrapper.getMainFillState(),
                                                                               isStart ? &p : 0,
                                                                               isStart ? 0 : &p, 0,
                                                                               0));
            jassert (fill.isGradient());
            return p;
        }

        void setPosition (const RelativePoint& newPoint, UndoManager* undoManager)
        {
            DrawablePath::ValueTreeWrapper wrapper (item);

            RelativePoint p1, p2;
            ValueTree fillState (isStroke ? wrapper.getStrokeFillState() : wrapper.getMainFillState());
            const FillType fill (Drawable::ValueTreeWrapperBase::readFillType (fillState, &p1, &p2, 0, 0));
            jassert (fill.isGradient());

            if (isStart)
                p1 = newPoint;
            else
                p2 = newPoint;

            Drawable::ValueTreeWrapperBase::writeFillType (fillState, fill, &p1, &p2, 0, undoManager);
        }

        bool hasLine()                  { return isStart; }
        RelativePoint getEndOfLine()
        {
            RelativePoint p;
            DrawablePath::ValueTreeWrapper wrapper (item);
            ValueTree fillState (isStroke ? wrapper.getStrokeFillState() : wrapper.getMainFillState());
            Drawable::ValueTreeWrapperBase::readFillType (fillState, 0, &p, 0, 0);
            return p;
        }

        const Value getPositionValue (UndoManager* undoManager)
        {
            DrawablePath::ValueTreeWrapper wrapper (item);
            ValueTree fillState (isStroke ? wrapper.getStrokeFillState() : wrapper.getMainFillState());
            return fillState.getPropertyAsValue (isStart ? Drawable::ValueTreeWrapperBase::gradientPoint1 : Drawable::ValueTreeWrapperBase::gradientPoint2, undoManager);
        }

        void createProperties (DrawableDocument& document, Array <PropertyComponent*>& props)
        {
            DrawableTypeInstance instance (document, item);
            props.add (new ControlPointPropertyComp (instance, this, "X", true, document.getUndoManager()));
            props.add (new ControlPointPropertyComp (instance, this, "Y", false, document.getUndoManager()));
        }

    private:
        ValueTree item;
        bool isStart, isStroke;
    };

    //==============================================================================
    class PathControlPoint  : public ControlPoint
    {
    public:
        PathControlPoint (const String& id_,
                          const DrawablePath::ValueTreeWrapper::Element& element_,
                          const DrawablePath::ValueTreeWrapper::Element& previousElement_, const int cpNum_, const int numCps_)
            : ControlPoint (id_), element (element_), previousElement (previousElement_), cpNum (cpNum_), numCps (numCps_)
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

        const Value getPositionValue (UndoManager* undoManager)
        {
            return element.getControlPointValue (cpNum, undoManager);
        }

        bool hasLine()                  { return numCps > 1 && cpNum == 0 || cpNum == 1; }

        RelativePoint getEndOfLine()
        {
            if (cpNum == 0)
                return previousElement.getEndPoint();
            else
                return element.getControlPoint (2);
        }

        void createProperties (DrawableDocument& document, Array <PropertyComponent*>& props)
        {
            DrawableTypeInstance instance (document, element.getParent().getState());
            props.add (new ControlPointPropertyComp (instance, this, "X", true, document.getUndoManager()));
            props.add (new ControlPointPropertyComp (instance, this, "Y", false, document.getUndoManager()));
        }

    private:
        DrawablePath::ValueTreeWrapper::Element element, previousElement;
        int cpNum, numCps;
    };

    void getGradientControlPoints (DrawablePath::ValueTreeWrapper& wrapper, DrawableTypeInstance& item,
                                   OwnedArray <ControlPoint>& points, const String& itemId)
    {
        const FillType fill (Drawable::ValueTreeWrapperBase::readFillType (wrapper.getMainFillState(), 0, 0, 0, 0));

        if (fill.isGradient())
        {
            points.add (new GradientControlPoint (itemId + "/gf1", item.getState(), true, false));
            points.add (new GradientControlPoint (itemId + "/gf2", item.getState(), false, false));
        }

        const FillType stroke (Drawable::ValueTreeWrapperBase::readFillType (wrapper.getStrokeFillState(), 0, 0, 0, 0));

        if (stroke.isGradient())
        {
            points.add (new GradientControlPoint (itemId + "/gs1", item.getState(), true, true));
            points.add (new GradientControlPoint (itemId + "/gs2", item.getState(), false, true));
        }
    }

    void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points)
    {
        DrawablePath::ValueTreeWrapper wrapper (item.getState());

        const ValueTree pathTree (wrapper.getPathState());
        const int numElements = pathTree.getNumChildren();
        const String itemId (item.getID());

        if (numElements > 0)
        {
            DrawablePath::ValueTreeWrapper::Element last (pathTree.getChild(0));

            for (int i = 0; i < numElements; ++i)
            {
                const DrawablePath::ValueTreeWrapper::Element e (pathTree.getChild(i));
                const int numCps = e.getNumControlPoints();

                for (int j = 0; j < numCps; ++j)
                    points.add (new PathControlPoint (itemId + "/" + String(i) + "/" + String(j), e, last, j, numCps));

                last = e;
            }
        }

        getGradientControlPoints (wrapper, item, points, itemId);
    }

    void getVisibleControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points, const EditorCanvasBase::SelectedItems& selection)
    {
        DrawablePath::ValueTreeWrapper wrapper (item.getState());

        const ValueTree pathTree (wrapper.getPathState());
        const int numElements = pathTree.getNumChildren();
        const String itemId (item.getID());

        if (numElements > 0)
        {
            DrawablePath::ValueTreeWrapper::Element last (pathTree.getChild(0));
            bool lastWasSelected = false;

            for (int i = 0; i < numElements; ++i)
            {
                const String elementIdRoot (itemId + "/" + String(i) + "/");
                const DrawablePath::ValueTreeWrapper::Element e (pathTree.getChild(i));
                int numCps = e.getNumControlPoints();
                bool pointIsSelected = false;

                for (int k = numCps; --k >= 0;)
                {
                    if (selection.isSelected (elementIdRoot + String (k)))
                    {
                        pointIsSelected = true;
                        break;
                    }
                }

                if (numCps > 1)
                {
                    if (pointIsSelected || lastWasSelected)
                    {
                        for (int j = 0; j < numCps; ++j)
                            points.add (new PathControlPoint (elementIdRoot + String(j), e, last, j, numCps));
                    }
                    else
                    {
                        points.add (new PathControlPoint (elementIdRoot + String (numCps - 1), e, last, numCps - 1, numCps));
                    }
                }
                else
                {
                    for (int j = 0; j < numCps; ++j)
                        points.add (new PathControlPoint (elementIdRoot + String(j), e, last, j, numCps));
                }

                last = e;
                lastWasSelected = pointIsSelected;
            }
        }

        getGradientControlPoints (wrapper, item, points, itemId);
    }

    //==============================================================================
    class StrokeValueSourceBase    : public Value::ValueSource,
                                     public ValueTree::Listener
    {
    public:
        StrokeValueSourceBase (const DrawablePath::ValueTreeWrapper& wrapper_, UndoManager* undoManager_)
           : wrapper (wrapper_), undoManager (undoManager_)
        {
            wrapper.getState().addListener (this);
        }

        ~StrokeValueSourceBase() {}

        void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property) { sendChangeMessage (true); }
        void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged) {}
        void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged) {}

    protected:
        DrawablePath::ValueTreeWrapper wrapper;
        UndoManager* undoManager;
    };

    //==============================================================================
    class StrokeThicknessValueSource    : public StrokeValueSourceBase
    {
    public:
        StrokeThicknessValueSource (const DrawablePath::ValueTreeWrapper& wrapper_, UndoManager* undoManager_)
           : StrokeValueSourceBase (wrapper_, undoManager_)
        {}

        const var getValue() const
        {
            return wrapper.getStrokeType().getStrokeThickness();
        }

        void setValue (const var& newValue)
        {
            PathStrokeType s (wrapper.getStrokeType());
            s.setStrokeThickness (newValue);
            wrapper.setStrokeType (s, undoManager);
        }

        static PropertyComponent* create (const DrawablePath::ValueTreeWrapper& wrapper, UndoManager* undoManager)
        {
            return new SliderPropertyComponent (Value (new StrokeThicknessValueSource (wrapper, undoManager)),
                                                "Stroke Thickness", 0, 50.0, 0.1);
        }
    };

    //==============================================================================
    class StrokeJoinStyleValueSource    : public StrokeValueSourceBase
    {
    public:
        StrokeJoinStyleValueSource (const DrawablePath::ValueTreeWrapper& wrapper_, UndoManager* undoManager_)
           : StrokeValueSourceBase (wrapper_, undoManager_)
        {}

        const var getValue() const
        {
            return (int) wrapper.getStrokeType().getJointStyle();
        }

        void setValue (const var& newValue)
        {
            PathStrokeType s (wrapper.getStrokeType());
            s.setJointStyle ((PathStrokeType::JointStyle) (int) newValue);
            wrapper.setStrokeType (s, undoManager);
        }

        static PropertyComponent* create (const DrawablePath::ValueTreeWrapper& wrapper, UndoManager* undoManager)
        {
            const char* types[] = { "Miter", "Curved", "Bevel", 0 };
            const int mappings[] =  { PathStrokeType::mitered, PathStrokeType::curved, PathStrokeType::beveled };
            return new ChoicePropertyComponent (Value (new StrokeJoinStyleValueSource (wrapper, undoManager)),
                                                "Joint Style", StringArray (types), Array<var> (mappings, numElementsInArray (mappings)));
        }
    };

    //==============================================================================
    class StrokeCapStyleValueSource    : public StrokeValueSourceBase
    {
    public:
        StrokeCapStyleValueSource (const DrawablePath::ValueTreeWrapper& wrapper_, UndoManager* undoManager_)
           : StrokeValueSourceBase (wrapper_, undoManager_)
        {}

        const var getValue() const
        {
            return (int) wrapper.getStrokeType().getEndStyle();
        }

        void setValue (const var& newValue)
        {
            PathStrokeType s (wrapper.getStrokeType());
            s.setEndStyle ((PathStrokeType::EndCapStyle) (int) newValue);
            wrapper.setStrokeType (s, undoManager);
        }

        static PropertyComponent* create (const DrawablePath::ValueTreeWrapper& wrapper, UndoManager* undoManager)
        {
            const char* types[] = { "Butt", "Square", "Round", 0 };
            const int mappings[] =  { PathStrokeType::butt, PathStrokeType::square, PathStrokeType::rounded };
            return new ChoicePropertyComponent (Value (new StrokeCapStyleValueSource (wrapper, undoManager)),
                                                "Cap Style", StringArray (types), Array<var> (mappings, numElementsInArray (mappings)));
        }
    };
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
        di.setBoundingBox (RelativeParallelogram (approxPosition,
                                                  approxPosition + Point<float> (100.0f, 0.0f),
                                                  approxPosition + Point<float> (0.0f, 100.0f)));
        return di.createValueTree (&document);
    }

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
        DrawableImage::ValueTreeWrapper wrapper (item.getState());

        if (item.getDocument().getProject() != 0)
        {
            OwnedArray<Project::Item> images;
            item.getDocument().getProject()->findAllImageItems (images);

            StringArray choices;
            Array<var> ids;

            for (int i = 0; i < images.size(); ++i)
            {
                choices.add (images.getUnchecked(i)->getName().toString());
                ids.add (images.getUnchecked(i)->getImageFileID());
            }

            props.add (new ChoicePropertyComponent (wrapper.getImageIdentifierValue (item.getUndoManager()),
                                                    "Image", choices, ids));
        }

        props.add (new SliderPropertyComponent (wrapper.getOpacityValue (item.getUndoManager()),
                                                "Opacity", 0, 1.0, 0.001));

        props.add (new ColourPropertyComponent (item.getUndoManager(), "Overlay Colour",
                                                wrapper.getOverlayColourValue (item.getUndoManager()),
                                                Colours::transparentBlack, true));

        props.add (new ResetButtonPropertyComponent (item, wrapper));
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    //==============================================================================
    class ImageControlPoint  : public ControlPoint
    {
    public:
        ImageControlPoint (const String& id_, const DrawableTypeInstance& item_, const int cpNum_)
            : ControlPoint (id_), item (item_), cpNum (cpNum_)
        {}

        ~ImageControlPoint() {}

        const RelativePoint getPosition()
        {
            DrawableImage::ValueTreeWrapper wrapper (item.getState());
            const RelativeParallelogram bounds (wrapper.getBoundingBox());

            switch (cpNum)
            {
                case 0: return bounds.topLeft;
                case 1: return bounds.topRight;
                case 2: return bounds.bottomLeft;
                default: jassertfalse; break;
            }

            return RelativePoint();
        }

        void setPosition (const RelativePoint& newPoint, UndoManager* undoManager)
        {
            DrawableImage::ValueTreeWrapper wrapper (item.getState());
            RelativeParallelogram bounds (wrapper.getBoundingBox());

            switch (cpNum)
            {
                case 0: bounds.topLeft = newPoint; break;
                case 1: bounds.topRight = newPoint; break;
                case 2: bounds.bottomLeft = newPoint; break;
                default: jassertfalse; break;
            }

            wrapper.setBoundingBox (bounds, undoManager);
        }

        const Value getPositionValue (UndoManager* undoManager)
        {
            DrawableImage::ValueTreeWrapper wrapper (item.getState());

            switch (cpNum)
            {
                case 0: return item.getState().getPropertyAsValue (DrawableImage::ValueTreeWrapper::topLeft, undoManager);
                case 1: return item.getState().getPropertyAsValue (DrawableImage::ValueTreeWrapper::topRight, undoManager);
                case 2: return item.getState().getPropertyAsValue (DrawableImage::ValueTreeWrapper::bottomLeft, undoManager);
                default: jassertfalse; break;
            }
            return Value();
        }

        bool hasLine()                  { return false; }
        RelativePoint getEndOfLine()    { return RelativePoint(); }

        void createProperties (DrawableDocument& document, Array <PropertyComponent*>& props)
        {
            props.add (new ControlPointPropertyComp (item, this, "X", true, document.getUndoManager()));
            props.add (new ControlPointPropertyComp (item, this, "Y", false, document.getUndoManager()));
        }

    private:
        DrawableTypeInstance item;
        int cpNum;
    };

    void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points)
    {
        const String itemIDRoot (item.getID() + "/");

        for (int i = 0; i < 3; ++i)
            points.add (new ImageControlPoint (itemIDRoot + String (i), item, i));
    }

    void getVisibleControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points, const EditorCanvasBase::SelectedItems&)
    {
        return getAllControlPoints (item, points);
    }

    //==============================================================================
    class ResetButtonPropertyComponent  : public ButtonPropertyComponent
    {
    public:
        ResetButtonPropertyComponent (DrawableTypeInstance& item_,
                                      const DrawableImage::ValueTreeWrapper& wrapper_)
            : ButtonPropertyComponent ("Reset", false),
              item (item_), wrapper (wrapper_)
        {
        }

        const String getButtonText() const      { return "Reset to Original Size"; }

        void buttonClicked()
        {
            Image im (item.getDocument().getImageForIdentifier (wrapper.getImageIdentifier()));

            if (im.isValid())
            {
                RelativeParallelogram bounds (wrapper.getBoundingBox());

                bounds.topRight.moveToAbsolute (bounds.topLeft.resolve (&item) + Point<float> ((float) im.getWidth(), 0.0f), &item);
                bounds.bottomLeft.moveToAbsolute (bounds.topLeft.resolve (&item) + Point<float> (0.0f, (float) im.getHeight()), &item);

                wrapper.setBoundingBox (bounds, item.getUndoManager());
            }
        }

    private:
        DrawableTypeInstance item;
        DrawableImage::ValueTreeWrapper wrapper;
    };
};

//==============================================================================
class DrawableCompositeHandler : public DrawableTypeHandler
{
public:
    DrawableCompositeHandler()  : DrawableTypeHandler ("Group", DrawableComposite::valueTreeType) {}
    ~DrawableCompositeHandler() {}

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
        DrawableComposite::ValueTreeWrapper wrapper (item.getState());
        props.add (new ResetButtonPropertyComponent (item, wrapper));
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    //==============================================================================
    class CompositeControlPoint  : public ControlPoint
    {
    public:
        CompositeControlPoint (const String& id_, const ValueTree& item_, const int cpNum_)
            : ControlPoint (id_), item (item_), cpNum (cpNum_)
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

        const Value getPositionValue (UndoManager* undoManager)
        {
            jassertfalse
            return Value();
        }

        bool hasLine()                  { return false; }
        RelativePoint getEndOfLine()    { return RelativePoint(); }

        void createProperties (DrawableDocument& document, Array <PropertyComponent*>& props)
        {
            DrawableTypeInstance instance (document, item);
            props.add (new ControlPointPropertyComp (instance, this, "X", true, document.getUndoManager()));
            props.add (new ControlPointPropertyComp (instance, this, "Y", false, document.getUndoManager()));
        }

    private:
        ValueTree item;
        int cpNum;
    };

    void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points)
    {
        const String itemIDRoot (item.getID() + "/");

        for (int i = 0; i < 3; ++i)
            points.add (new CompositeControlPoint (itemIDRoot + String(i), item.getState(), i));
    }

    void getVisibleControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points, const EditorCanvasBase::SelectedItems&)
    {
        return getAllControlPoints (item, points);
    }

    //==============================================================================
    class ResetButtonPropertyComponent  : public ButtonPropertyComponent
    {
    public:
        ResetButtonPropertyComponent (DrawableTypeInstance& item_,
                                      const DrawableComposite::ValueTreeWrapper& wrapper_)
            : ButtonPropertyComponent ("Reset", false),
              item (item_), wrapper (wrapper_)
        {
        }

        const String getButtonText() const { return "Reset to Original Size"; }

        void buttonClicked()
        {
            RelativePoint topLeft (wrapper.getTargetPositionForOrigin());
            RelativePoint topRight (wrapper.getTargetPositionForX1Y0());
            RelativePoint bottomLeft (wrapper.getTargetPositionForX0Y1());

            topRight.moveToAbsolute (topLeft.resolve (&item) + Point<float> (1.0f, 0.0f), &item);
            bottomLeft.moveToAbsolute (topLeft.resolve (&item) + Point<float> (0.0f, 1.0f), &item);

            wrapper.setTargetPositionForX1Y0 (topRight, item.getUndoManager());
            wrapper.setTargetPositionForX0Y1 (bottomLeft, item.getUndoManager());
        }

    private:
        DrawableTypeInstance item;
        DrawableComposite::ValueTreeWrapper wrapper;
    };
};

//==============================================================================
class DrawableTextHandler : public DrawableTypeHandler
{
public:
    DrawableTextHandler()  : DrawableTypeHandler ("Text", DrawableText::valueTreeType) {}
    ~DrawableTextHandler() {}

    static const ValueTree createNewInstance (DrawableDocument& document, const Point<float>& approxPosition)
    {
        DrawableText dt;
        dt.setText ("Text");
        dt.setBoundingBox (RelativeParallelogram (approxPosition,
                                                  approxPosition + Point<float> (100.0f, 0.0f),
                                                  approxPosition + Point<float> (0.0f, 100.0f)));
        dt.setFont (Font (25.0f), true);
        return dt.createValueTree (&document);
    }

    void createPropertyEditors (DrawableTypeInstance& item, Array <PropertyComponent*>& props)
    {
        DrawableText::ValueTreeWrapper wrapper (item.getState());
        props.add (new TextPropertyComponent (wrapper.getTextValue (item.getUndoManager()), "Text", 16384, true));

        Value v (wrapper.getFontValue (item.getUndoManager()));
        props.add (FontNameValueSource::createProperty ("Font", v));
        props.add (FontStyleValueSource::createProperty ("Font Style", v));
        props.add (new ResetButtonPropertyComponent (item, wrapper));
    }

    void itemDoubleClicked (const MouseEvent& e, DrawableTypeInstance& item)
    {
    }

    //==============================================================================
    class TextControlPoint  : public ControlPoint
    {
    public:
        TextControlPoint (const String& id_, const ValueTree& item_, const int cpNum_)
            : ControlPoint (id_), item (item_), cpNum (cpNum_)
        {}

        ~TextControlPoint() {}

        const RelativePoint getPosition()
        {
            DrawableText::ValueTreeWrapper wrapper (item);
            RelativeParallelogram bounds (wrapper.getBoundingBox());

            switch (cpNum)
            {
                case 0: return wrapper.getBoundingBox().topLeft;
                case 1: return wrapper.getBoundingBox().topRight;
                case 2: return wrapper.getBoundingBox().bottomLeft;
                case 3: return wrapper.getFontSizeAndScaleAnchor();
                default: jassertfalse; break;
            }

            return RelativePoint();
        }

        void setPosition (const RelativePoint& newPoint, UndoManager* undoManager)
        {
            DrawableText::ValueTreeWrapper wrapper (item);

            if (cpNum == 3)
            {
                wrapper.setFontSizeAndScaleAnchor (newPoint, undoManager);
            }
            else
            {
                RelativeParallelogram bounds (wrapper.getBoundingBox());

                switch (cpNum)
                {
                    case 0: bounds.topLeft = newPoint; break;
                    case 1: bounds.topRight = newPoint; break;
                    case 2: bounds.bottomLeft = newPoint; break;
                    default: jassertfalse; break;
                }

                wrapper.setBoundingBox (bounds, undoManager);
            }
        }

        const Value getPositionValue (UndoManager* undoManager)
        {
            DrawableText::ValueTreeWrapper wrapper (item);

            switch (cpNum)
            {
                case 0: return item.getPropertyAsValue (DrawableText::ValueTreeWrapper::topLeft, undoManager);
                case 1: return item.getPropertyAsValue (DrawableText::ValueTreeWrapper::topRight, undoManager);
                case 2: return item.getPropertyAsValue (DrawableText::ValueTreeWrapper::bottomLeft, undoManager);
                case 3: return item.getPropertyAsValue (DrawableText::ValueTreeWrapper::fontSizeAnchor, undoManager);
                default: jassertfalse; break;
            }
            return Value();
        }

        bool hasLine()                  { return false; }
        RelativePoint getEndOfLine()    { return RelativePoint(); }

        void createProperties (DrawableDocument& document, Array <PropertyComponent*>& props)
        {
            DrawableTypeInstance instance (document, item);
            props.add (new ControlPointPropertyComp (instance, this, "X", true, document.getUndoManager()));
            props.add (new ControlPointPropertyComp (instance, this, "Y", false, document.getUndoManager()));
        }

    private:
        ValueTree item;
        int cpNum;
    };

    void getAllControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points)
    {
        const String itemIDRoot (item.getID() + "/");

        for (int i = 0; i < 4; ++i)
            points.add (new TextControlPoint (itemIDRoot + String(i), item.getState(), i));
    }

    void getVisibleControlPoints (DrawableTypeInstance& item, OwnedArray <ControlPoint>& points, const EditorCanvasBase::SelectedItems&)
    {
        return getAllControlPoints (item, points);
    }

    //==============================================================================
    class ResetButtonPropertyComponent  : public ButtonPropertyComponent
    {
    public:
        ResetButtonPropertyComponent (DrawableTypeInstance& item_,
                                      const DrawableText::ValueTreeWrapper& wrapper_)
            : ButtonPropertyComponent ("Reset", false),
              item (item_), wrapper (wrapper_)
        {
        }

        const String getButtonText() const      { return "Make Perpendicular"; }

        void buttonClicked()
        {
            RelativeParallelogram bounds (wrapper.getBoundingBox());

            const AffineTransform t (bounds.resetToPerpendicular (&item));

            RelativePoint fontPos (wrapper.getFontSizeAndScaleAnchor());
            fontPos.moveToAbsolute (fontPos.resolve (&item).transformedBy (t), &item);

            wrapper.setBoundingBox (bounds, item.getUndoManager());
            wrapper.setFontSizeAndScaleAnchor (fontPos, item.getUndoManager());
        }

    private:
        DrawableTypeInstance item;
        DrawableText::ValueTreeWrapper wrapper;
    };
};


//==============================================================================
DrawableTypeManager::DrawableTypeManager()
{
    handlers.add (new DrawablePathHandler());
    handlers.add (new DrawableImageHandler());
    handlers.add (new DrawableCompositeHandler());
    handlers.add (new DrawableTextHandler());
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
    const char* types[] = { "New Triangle", "New Rectangle", "New Ellipse", "New Image", "New Text Object", 0 };
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
        case 4: return DrawableTextHandler::createNewInstance (document, approxPosition);
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
    ValueTree v (state);
    while (v.getParent().isValid() && ! v.hasType (DrawableComposite::valueTreeType))
        v = v.getParent();

    DrawableComposite::ValueTreeWrapper wrapper (v);

    ValueTree markerState (wrapper.getMarkerState (true, objectName));
    if (markerState.isValid())
        return wrapper.getMarker (true, markerState).position;

    markerState = wrapper.getMarkerState (false, objectName);
    if (markerState.isValid())
        return wrapper.getMarker (false, markerState).position;

    return RelativeCoordinate();
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

void DrawableTypeInstance::applyTransform (Drawable* drawable, const AffineTransform& transform)
{
    OwnedArray <ControlPoint> points;
    getAllControlPoints (points);

    for (int i = points.size(); --i >= 0;)
    {
        RelativePoint rp (points.getUnchecked(i)->getPosition());
        Point<float> p (rp.resolve (drawable->getParent()));
        p.applyTransform (transform);
        rp.moveToAbsolute (p, drawable->getParent());

        points.getUnchecked(i)->setPosition (rp, document.getUndoManager());
    }
}

void DrawableTypeInstance::getAllControlPoints (OwnedArray <ControlPoint>& points)
{
    return getHandler()->getAllControlPoints (*this, points);
}

void DrawableTypeInstance::getVisibleControlPoints (OwnedArray <ControlPoint>& points, const EditorCanvasBase::SelectedItems& selection)
{
    return getHandler()->getVisibleControlPoints (*this, points, selection);
}
