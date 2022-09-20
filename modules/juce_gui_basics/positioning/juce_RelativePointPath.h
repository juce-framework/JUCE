/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
    A path object that consists of RelativePoint coordinates rather than the normal fixed ones.

    One of these paths can be converted into a Path object for drawing and manipulation, but
    unlike a Path, its points can be dynamic instead of just fixed.

    @see RelativePoint, RelativeCoordinate

    @tags{GUI}
*/
class JUCE_API  RelativePointPath
{
public:
    //==============================================================================
    RelativePointPath();
    RelativePointPath (const RelativePointPath&);
    explicit RelativePointPath (const Path& path);
    ~RelativePointPath();

    bool operator== (const RelativePointPath&) const noexcept;
    bool operator!= (const RelativePointPath&) const noexcept;

    //==============================================================================
    /** Resolves this points in this path and adds them to a normal Path object. */
    void createPath (Path& path, Expression::Scope* scope) const;

    /** Returns true if the path contains any non-fixed points. */
    bool containsAnyDynamicPoints() const;

    /** Quickly swaps the contents of this path with another. */
    void swapWith (RelativePointPath&) noexcept;

    //==============================================================================
    /** The types of element that may be contained in this path.
        @see RelativePointPath::ElementBase
    */
    enum ElementType
    {
        nullElement,
        startSubPathElement,
        closeSubPathElement,
        lineToElement,
        quadraticToElement,
        cubicToElement
    };

    //==============================================================================
    /** Base class for the elements that make up a RelativePointPath.
    */
    class JUCE_API  ElementBase
    {
    public:
        ElementBase (ElementType type);
        virtual ~ElementBase() = default;
        virtual void addToPath (Path& path, Expression::Scope*) const = 0;
        virtual RelativePoint* getControlPoints (int& numPoints) = 0;
        virtual ElementBase* clone() const = 0;
        bool isDynamic();

        const ElementType type;

    private:
        JUCE_DECLARE_NON_COPYABLE (ElementBase)
    };

    //==============================================================================
    /** Class for the start sub path element */
    class JUCE_API  StartSubPath  : public ElementBase
    {
    public:
        StartSubPath (const RelativePoint& pos);
        void addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (int& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint startPos;

    private:
        JUCE_DECLARE_NON_COPYABLE (StartSubPath)
    };

    //==============================================================================
    /** Class for the close sub path element */
    class JUCE_API  CloseSubPath  : public ElementBase
    {
    public:
        CloseSubPath();
        void addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (int& numPoints) override;
        ElementBase* clone() const override;

    private:
        JUCE_DECLARE_NON_COPYABLE (CloseSubPath)
    };

    //==============================================================================
    /** Class for the line to element */
    class JUCE_API  LineTo  : public ElementBase
    {
    public:
        LineTo (const RelativePoint& endPoint);
        void addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (int& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint endPoint;

    private:
        JUCE_DECLARE_NON_COPYABLE (LineTo)
    };

    //==============================================================================
    /** Class for the quadratic to element */
    class JUCE_API  QuadraticTo  : public ElementBase
    {
    public:
        QuadraticTo (const RelativePoint& controlPoint, const RelativePoint& endPoint);
        ValueTree createTree() const;
        void addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (int& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint controlPoints[2];

    private:
        JUCE_DECLARE_NON_COPYABLE (QuadraticTo)
    };

    //==============================================================================
    /** Class for the cubic to element */
    class JUCE_API  CubicTo  : public ElementBase
    {
    public:
        CubicTo (const RelativePoint& controlPoint1, const RelativePoint& controlPoint2, const RelativePoint& endPoint);
        ValueTree createTree() const;
        void addToPath (Path& path, Expression::Scope*) const override;
        RelativePoint* getControlPoints (int& numPoints) override;
        ElementBase* clone() const override;

        RelativePoint controlPoints[3];

    private:
        JUCE_DECLARE_NON_COPYABLE (CubicTo)
    };

    //==============================================================================
    void addElement (ElementBase* newElement);

    //==============================================================================
    OwnedArray<ElementBase> elements;
    bool usesNonZeroWinding;

private:
    class Positioner;
    friend class Positioner;
    bool containsDynamicPoints;

    void applyTo (DrawablePath& path) const;

    RelativePointPath& operator= (const RelativePointPath&);
    JUCE_LEAK_DETECTOR (RelativePointPath)
};

} // namespace juce
