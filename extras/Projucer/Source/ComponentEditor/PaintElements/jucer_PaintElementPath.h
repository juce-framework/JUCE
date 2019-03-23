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

#pragma once

#include "jucer_ColouredElement.h"
#include "jucer_ElementSiblingComponent.h"
class PathPointComponent;
class PaintElementPath;

//==============================================================================
class PathPoint
{
public:
    PathPoint (PaintElementPath* const owner);
    PathPoint (const PathPoint& other);
    PathPoint& operator= (const PathPoint& other);
    ~PathPoint();

    PaintElementPath* owner;
    Path::Iterator::PathElementType type;
    RelativePositionedRectangle pos [3];

    int getNumPoints() const;

    void changePointType (const Path::Iterator::PathElementType newType,
                          const Rectangle<int>& parentArea,
                          const bool undoable);

    void deleteFromPath();
    void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected);

private:
    PathPoint withChangedPointType (const Path::Iterator::PathElementType newType,
                                    const Rectangle<int>& parentArea) const;
};


//==============================================================================
class PaintElementPath   : public ColouredElement
{
public:
    PaintElementPath (PaintRoutine* owner);
    ~PaintElementPath() override;

    //==============================================================================
    void setInitialBounds (int parentWidth, int parentHeight) override;
    Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const override;
    void setCurrentBounds (const Rectangle<int>& b, const Rectangle<int>& parentArea, const bool undoable) override;

    //==============================================================================
    bool getPoint (int index, int pointNumber, double& x, double& y, const Rectangle<int>& parentArea) const;
    void movePoint (int index, int pointNumber, double newX, double newY, const Rectangle<int>& parentArea, const bool undoable);

    RelativePositionedRectangle getPoint (int index, int pointNumber) const;
    void setPoint (int index, int pointNumber, const RelativePositionedRectangle& newPoint, const bool undoable);

    int getNumPoints() const noexcept                                    { return points.size(); }
    PathPoint* getPoint (int index) const noexcept                       { return points [index]; }
    int indexOfPoint (PathPoint* const p) const noexcept                 { return points.indexOf (p); }

    PathPoint* addPoint (int pointIndexToAddItAfter, const bool undoable);
    void deletePoint (int pointIndex, const bool undoable);

    void pointListChanged();

    int findSegmentAtXY (int x, int y) const;

    //==============================================================================
    bool isSubpathClosed (int pointIndex) const;
    void setSubpathClosed (int pointIndex, const bool closed, const bool undoable);

    bool isNonZeroWinding() const noexcept                               { return nonZeroWinding; }
    void setNonZeroWinding (const bool nonZero, const bool undoable);

    //==============================================================================
    void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected) override;

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) override;
    void applyCustomPaintSnippets (StringArray& snippets) override;

    //==============================================================================
    static const char* getTagName() noexcept                            { return "PATH"; }
    XmlElement* createXml() const override;
    bool loadFromXml (const XmlElement& xml) override;

    void setToPath (const Path& p);

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea) override;
    void drawExtraEditorGraphics (Graphics& g, const Rectangle<int>& relativeTo) override;

    void resized() override;
    void parentSizeChanged() override;

    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;

    void createSiblingComponents() override;
    void changed() override;

private:
    friend class PathPoint;
    friend class PathPointComponent;
    OwnedArray <PathPoint> points;
    bool nonZeroWinding;
    mutable Path path;
    mutable Rectangle<int> lastPathBounds;
    int mouseDownOnSegment;
    bool mouseDownSelectSegmentStatus;
    String customPaintCode;

    String pathToString() const;
    void restorePathFromString (const String& s);
    void updateStoredPath (const ComponentLayout* layout, const Rectangle<int>& parentArea) const;
    int getBorderSize() const;

    void rescalePoint (RelativePositionedRectangle& pos, int dx, int dy,
                       double scaleX, double scaleY,
                       double scaleStartX, double scaleStartY,
                       const Rectangle<int>& parentArea) const;
};


//==============================================================================
class PathPointComponent    : public ElementSiblingComponent
{
public:
    PathPointComponent (PaintElementPath* const path_,
                        const int index, const int pointNumber);

    ~PathPointComponent();

    void updatePosition();
    void showPopupMenu();

    void paint (Graphics& g);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);

    void changeListenerCallback (ChangeBroadcaster*);

private:
    PaintElementPath* const path;
    PaintRoutine* const routine;
    const int index;
    const int pointNumber;
    int dragX, dragY;
    bool selected, dragging, mouseDownSelectStatus;
};
