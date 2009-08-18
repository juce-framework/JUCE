/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_PAINTELEMENTPATH_JUCEHEADER__
#define __JUCER_PAINTELEMENTPATH_JUCEHEADER__

#include "jucer_ColouredElement.h"
#include "jucer_ElementSiblingComponent.h"
class PathPointComponent;
class PaintElementPath;


//==============================================================================
/**
*/
class PathPoint
{
public:
    //==============================================================================
    PathPoint (PaintElementPath* const owner);
    PathPoint (const PathPoint& other);
    const PathPoint& operator= (const PathPoint& other);
    ~PathPoint();

    //==============================================================================
    PaintElementPath* owner;

    Path::Iterator::PathElementType type;

    RelativePositionedRectangle pos [3];

    //==============================================================================
    int getNumPoints() const;

    void changePointType (const Path::Iterator::PathElementType newType,
                          const Rectangle& parentArea,
                          const bool undoable);

    void deleteFromPath();
    void getEditableProperties (Array <PropertyComponent*>& properties);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    const PathPoint withChangedPointType (const Path::Iterator::PathElementType newType,
                                          const Rectangle& parentArea) const;
};


//==============================================================================
/**
*/
class PaintElementPath   : public ColouredElement
{
public:
    //==============================================================================
    PaintElementPath (PaintRoutine* owner);
    ~PaintElementPath();

    //==============================================================================
    void setInitialBounds (int parentWidth, int parentHeight);
    const Rectangle getCurrentBounds (const Rectangle& parentArea) const;
    void setCurrentBounds (const Rectangle& b, const Rectangle& parentArea, const bool undoable);

    //==============================================================================
    bool getPoint (int index, int pointNumber, double& x, double& y, const Rectangle& parentArea) const;
    void movePoint (int index, int pointNumber, double newX, double newY, const Rectangle& parentArea, const bool undoable);

    const RelativePositionedRectangle getPoint (int index, int pointNumber) const;
    void setPoint (int index, int pointNumber, const RelativePositionedRectangle& newPoint, const bool undoable);

    int getNumPoints() const throw()                                    { return points.size(); }
    PathPoint* getPoint (int index) const throw()                       { return points [index]; }
    int indexOfPoint (PathPoint* const p) const throw()                 { return points.indexOf (p); }

    PathPoint* addPoint (int pointIndexToAddItAfter, const bool undoable);
    void deletePoint (int pointIndex, const bool undoable);

    void pointListChanged();

    int findSegmentAtXY (int x, int y) const;

    //==============================================================================
    bool isSubpathClosed (int pointIndex) const;
    void setSubpathClosed (int pointIndex, const bool closed, const bool undoable);

    bool isNonZeroWinding() const throw()                               { return nonZeroWinding; }
    void setNonZeroWinding (const bool nonZero, const bool undoable);

    //==============================================================================
    void getEditableProperties (Array <PropertyComponent*>& properties);

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode);

    //==============================================================================
    static const tchar* getTagName() throw()                            { return T("PATH"); }
    XmlElement* createXml() const;
    bool loadFromXml (const XmlElement& xml);

    void setToPath (const Path& p);

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea);
    void drawExtraEditorGraphics (Graphics& g, const Rectangle& relativeTo);

    void resized();
    void parentSizeChanged();

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);

    void createSiblingComponents();
    void changed();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class PathPoint;
    friend class PathPointComponent;
    OwnedArray <PathPoint> points;
    bool nonZeroWinding;
    mutable Path path;
    mutable Rectangle lastPathBounds;
    int mouseDownOnSegment;
    bool mouseDownSelectSegmentStatus;

    const String pathToString() const;
    void restorePathFromString (const String& s);
    void updateStoredPath (const ComponentLayout* layout, const Rectangle& parentArea) const;
    int getBorderSize() const;

    void rescalePoint (RelativePositionedRectangle& pos, int dx, int dy,
                       double scaleX, double scaleY,
                       double scaleStartX, double scaleStartY,
                       const Rectangle& parentArea) const;
};


//==============================================================================
/**
*/
class PathPointComponent    : public ElementSiblingComponent
{
public:
    //==============================================================================
    PathPointComponent (PaintElementPath* const path_,
                        const int index, const int pointNumber);

    ~PathPointComponent();

    //==============================================================================
    void updatePosition();
    void showPopupMenu();

    //==============================================================================
    void paint (Graphics& g);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);

    void changeListenerCallback (void*);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PaintElementPath* const path;
    PaintRoutine* const routine;
    const int index;
    const int pointNumber;
    int dragX, dragY;
    bool selected, dragging, mouseDownSelectStatus;
};


#endif   // __JUCER_PAINTELEMENTPATH_JUCEHEADER__
