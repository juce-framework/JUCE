/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_PAINTELEMENTPATH_H_INCLUDED
#define JUCER_PAINTELEMENTPATH_H_INCLUDED

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
    void getEditableProperties (Array<PropertyComponent*>& props);

private:
    PathPoint withChangedPointType (const Path::Iterator::PathElementType newType,
                                    const Rectangle<int>& parentArea) const;
};


//==============================================================================
class PaintElementPath   : public ColouredElement
{
public:
    PaintElementPath (PaintRoutine* owner);
    ~PaintElementPath();

    //==============================================================================
    void setInitialBounds (int parentWidth, int parentHeight);
    Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const;
    void setCurrentBounds (const Rectangle<int>& b, const Rectangle<int>& parentArea, const bool undoable);

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
    void getEditableProperties (Array<PropertyComponent*>& props);

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode);

    //==============================================================================
    static const char* getTagName() noexcept                            { return "PATH"; }
    XmlElement* createXml() const;
    bool loadFromXml (const XmlElement& xml);

    void setToPath (const Path& p);

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea);
    void drawExtraEditorGraphics (Graphics& g, const Rectangle<int>& relativeTo);

    void resized();
    void parentSizeChanged();

    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);

    void createSiblingComponents();
    void changed();

private:
    friend class PathPoint;
    friend class PathPointComponent;
    OwnedArray <PathPoint> points;
    bool nonZeroWinding;
    mutable Path path;
    mutable Rectangle<int> lastPathBounds;
    int mouseDownOnSegment;
    bool mouseDownSelectSegmentStatus;

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


#endif   // JUCER_PAINTELEMENTPATH_H_INCLUDED
