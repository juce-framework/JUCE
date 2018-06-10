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

#include "../jucer_GeneratedCode.h"
#include "../UI/jucer_RelativePositionedRectangle.h"
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
    void setPaintElementBounds (const Rectangle<int>& newBounds, const bool undoable);
    void setPaintElementBoundsAndProperties (PaintElement* elementToPosition, const Rectangle<int>& newBounds,
                                             PaintElement* referenceElement, const bool undoable);

    void updateBounds (const Rectangle<int>& activeArea);

    const String& getTypeName() const noexcept                   { return typeName; }
    PaintRoutine* getOwner() const noexcept                      { return owner; }

    //==============================================================================
    virtual void draw (Graphics& g,
                       const ComponentLayout* layout,
                       const Rectangle<int>& parentArea) = 0;

    virtual void drawExtraEditorGraphics (Graphics& g, const Rectangle<int>& relativeTo);

    virtual void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected);

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
    void paint (Graphics&) override;
    void resized() override;
    void mouseDown (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;
    void mouseUp (const MouseEvent&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void parentHierarchyChanged() override;

    virtual void applyCustomPaintSnippets (StringArray&) {}

    int borderThickness;

protected:
    PaintRoutine* const owner;
    RelativePositionedRectangle position;

    void resizeStart() override;
    void resizeEnd() override;
    void checkBounds (Rectangle<int>& bounds,
                      const Rectangle<int>& previousBounds,
                      const Rectangle<int>& limits,
                      bool isStretchingTop,
                      bool isStretchingLeft,
                      bool isStretchingBottom,
                      bool isStretchingRight) override;

    void applyBoundsToComponent (Component&, Rectangle<int>) override;

    Rectangle<int> getCurrentAbsoluteBounds() const;
    void getCurrentAbsoluteBoundsDouble (double& x, double& y, double& w, double& h) const;

    virtual void selectionChanged (const bool isSelected);

    virtual void createSiblingComponents();

    void siblingComponentsChanged();

    OwnedArray <ElementSiblingComponent> siblingComponents;

    void updateSiblingComps();

private:
    std::unique_ptr<ResizableBorderComponent> border;
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
        if (propToRefresh != nullptr && owner != nullptr)
            propToRefresh->refresh();
    }

    mutable Component::SafePointer<ElementType> owner;
    ChangeBroadcaster& broadcaster;
    PropertyComponent* propToRefresh;

    JUCE_DECLARE_NON_COPYABLE (ElementListener)
};
