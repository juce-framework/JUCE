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

#ifndef JUCER_PAINTELEMENT_H_INCLUDED
#define JUCER_PAINTELEMENT_H_INCLUDED

#include "../jucer_GeneratedCode.h"
#include "../ui/jucer_RelativePositionedRectangle.h"
class FillType;
class PaintRoutine;
class JucerDocument;
class ElementSiblingComponent;


//==============================================================================
/**
    Base class for objects that can be used in a PaintRoutine.

*/
class PaintElement  : public Component,
                      public ChangeListener,
                      public ComponentBoundsConstrainer
{
public:
    //==============================================================================
    PaintElement (PaintRoutine* owner, const String& typeName);
    virtual ~PaintElement();

    //==============================================================================
    virtual void setInitialBounds (int parentWidth, int parentHeight);

    virtual Rectangle<int> getCurrentBounds (const Rectangle<int>& activeArea) const;
    virtual void setCurrentBounds (const Rectangle<int>& newBounds, const Rectangle<int>& activeArea, const bool undoable);

    const RelativePositionedRectangle& getPosition() const;
    void setPosition (const RelativePositionedRectangle& newPosition, const bool undoable);

    void updateBounds (const Rectangle<int>& activeArea);

    const String& getTypeName() const noexcept                   { return typeName; }
    PaintRoutine* getOwner() const noexcept                      { return owner; }

    //==============================================================================
    virtual void draw (Graphics& g,
                       const ComponentLayout* layout,
                       const Rectangle<int>& parentArea) = 0;

    virtual void drawExtraEditorGraphics (Graphics& g, const Rectangle<int>& relativeTo);

    virtual void getEditableProperties (Array<PropertyComponent*>& props);

    virtual void showPopupMenu();

    //==============================================================================
    virtual XmlElement* createXml() const = 0;
    virtual bool loadFromXml (const XmlElement& xml) = 0;

    //==============================================================================
    virtual void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) = 0;

    JucerDocument* getDocument() const;

    virtual void changed();
    bool perform (UndoableAction* action, const String& actionName);

    //==============================================================================
    void paint (Graphics& g);
    void resized();
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void changeListenerCallback (ChangeBroadcaster*);
    void parentHierarchyChanged();

    int borderThickness;

protected:
    PaintRoutine* const owner;
    RelativePositionedRectangle position;

    void resizeStart();
    void resizeEnd();
    void checkBounds (Rectangle<int>& bounds,
                      const Rectangle<int>& previousBounds,
                      const Rectangle<int>& limits,
                      bool isStretchingTop,
                      bool isStretchingLeft,
                      bool isStretchingBottom,
                      bool isStretchingRight);

    void applyBoundsToComponent (Component* component, const Rectangle<int>& bounds);

    Rectangle<int> getCurrentAbsoluteBounds() const;
    void getCurrentAbsoluteBoundsDouble (double& x, double& y, double& w, double& h) const;

    virtual void selectionChanged (const bool isSelected);

    virtual void createSiblingComponents();

    void siblingComponentsChanged();

    OwnedArray <ElementSiblingComponent> siblingComponents;

    void updateSiblingComps();

private:
    ScopedPointer<ResizableBorderComponent> border;
    String typeName;
    bool selected, dragging, mouseDownSelectStatus;
    double originalAspectRatio;
    ChangeBroadcaster selfChangeListenerList;
};

//==============================================================================
template <typename ElementType>
class ElementListener   : public ChangeListener
{
public:
    ElementListener (ElementType* e)
        : owner (e), broadcaster (*owner->getDocument()),
          propToRefresh (nullptr)
    {
        broadcaster.addChangeListener (this);
    }

    ~ElementListener()
    {
        jassert (propToRefresh != nullptr);
        broadcaster.removeChangeListener (this);
    }

    void setPropertyToRefresh (PropertyComponent& pc)
    {
        propToRefresh = &pc;
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        jassert (propToRefresh != nullptr);
        if (propToRefresh != nullptr)
            propToRefresh->refresh();
    }

    mutable Component::SafePointer<ElementType> owner;
    ChangeBroadcaster& broadcaster;
    PropertyComponent* propToRefresh;

    JUCE_DECLARE_NON_COPYABLE (ElementListener)
};


#endif   // JUCER_PAINTELEMENT_H_INCLUDED
