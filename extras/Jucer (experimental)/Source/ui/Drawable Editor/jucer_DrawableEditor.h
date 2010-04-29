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

#ifndef __JUCER_DRAWABLEEDITOR_JUCEHEADER__
#define __JUCER_DRAWABLEEDITOR_JUCEHEADER__

#include "../jucer_DocumentEditorComponent.h"
class DrawableObjectComponent;


//==============================================================================
/**
*/
class DrawableEditor  : public DocumentEditorComponent
{
public:
    //==============================================================================
    DrawableEditor (OpenDocumentManager::Document* document,
                    Project* project,
                    DrawableDocument* drawableDocument);
    ~DrawableEditor();

    void paint (Graphics& g);
    void resized();

    DrawableDocument& getDocument() const   { return *drawableDocument; }

    static int64 getHashForNode (const ValueTree& node);

    //==============================================================================
    class Canvas  : public Component,
                    public LassoSource <int64>
    {
    public:
        Canvas (DrawableEditor& editor_);
        ~Canvas();

        void createRootObject();
        const Point<int> getOrigin() const throw()   { return origin; }

        void paint (Graphics& g);
        void mouseDown (const MouseEvent& e);
        void mouseDrag (const MouseEvent& e);
        void mouseUp (const MouseEvent& e);
        void childBoundsChanged (Component* child);
        void updateSize();

        void findLassoItemsInArea (Array <int64>& itemsFound, const Rectangle<int>& area);
        SelectedItemSet <int64>& getLassoSelection();

    private:
        DrawableEditor& editor;
        ScopedPointer <DrawableObjectComponent> rootObject;
        ScopedPointer <LassoComponent <int64> > lasso;
        BorderSize border;
        Point<int> origin;
    };

    Canvas* getCanvas() const       { return static_cast <Canvas*> (viewport->getViewedComponent()); }

    SelectedItemSet <int64> selectedItems;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class Canvas;
    Project* project;
    DrawableDocument* drawableDocument;

    Viewport* viewport;
    Component* rightHandPanel;
};


#endif   // __JUCER_DRAWABLEEDITOR_JUCEHEADER__
