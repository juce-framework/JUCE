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

namespace juce
{

//==============================================================================
/**
    A drawable object which renders a filled or outlined shape.

    For details on how to change the fill and stroke, see the DrawableShape class.

    @see Drawable, DrawableShape
*/
class JUCE_API  DrawablePath  : public DrawableShape
{
public:
    //==============================================================================
    /** Creates a DrawablePath. */
    DrawablePath();
    DrawablePath (const DrawablePath&);

    /** Destructor. */
    ~DrawablePath();

    //==============================================================================
    /** Changes the path that will be drawn.
        @see setFillColour, setStrokeType
    */
    void setPath (const Path& newPath);

    /** Sets the path using a RelativePointPath.
        Calling this will set up a Component::Positioner to automatically update the path
        if any of the points in the source path are dynamic.
    */
    void setPath (const RelativePointPath& newPath);

    /** Returns the current path. */
    const Path& getPath() const;

    /** Returns the current path for the outline. */
    const Path& getStrokePath() const;

    //==============================================================================
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;

    //==============================================================================
    /** Internally-used class for wrapping a DrawablePath's state into a ValueTree. */
    class ValueTreeWrapper   : public DrawableShape::FillAndStrokeState
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        bool usesNonZeroWinding() const;
        void setUsesNonZeroWinding (bool b, UndoManager* undoManager);

        class Element
        {
        public:
            explicit Element (const ValueTree& state);
            ~Element();

            const Identifier getType() const noexcept   { return state.getType(); }
            int getNumControlPoints() const noexcept;

            RelativePoint getControlPoint (int index) const;
            Value getControlPointValue (int index, UndoManager*);
            RelativePoint getStartPoint() const;
            RelativePoint getEndPoint() const;
            void setControlPoint (int index, const RelativePoint& point, UndoManager*);
            float getLength (Expression::Scope*) const;

            ValueTreeWrapper getParent() const;
            Element getPreviousElement() const;

            String getModeOfEndPoint() const;
            void setModeOfEndPoint (const String& newMode, UndoManager*);

            void convertToLine (UndoManager*);
            void convertToCubic (Expression::Scope*, UndoManager*);
            void convertToPathBreak (UndoManager* undoManager);
            ValueTree insertPoint (Point<float> targetPoint, Expression::Scope*, UndoManager*);
            void removePoint (UndoManager* undoManager);
            float findProportionAlongLine (Point<float> targetPoint, Expression::Scope*) const;

            static const Identifier mode, startSubPathElement, closeSubPathElement,
                                    lineToElement, quadraticToElement, cubicToElement;
            static const char* cornerMode;
            static const char* roundedMode;
            static const char* symmetricMode;

            ValueTree state;
        };

        ValueTree getPathState();

        void readFrom (const RelativePointPath& relativePath, UndoManager* undoManager);
        void writeTo (RelativePointPath& relativePath) const;

        static const Identifier nonZeroWinding, point1, point2, point3;
    };

private:
    //==============================================================================
    ScopedPointer<RelativePointPath> relativePath;

    class RelativePositioner;
    friend class RelativePositioner;
    void applyRelativePath (const RelativePointPath&, Expression::Scope*);

    DrawablePath& operator= (const DrawablePath&);
    JUCE_LEAK_DETECTOR (DrawablePath)
};

} // namespace juce
