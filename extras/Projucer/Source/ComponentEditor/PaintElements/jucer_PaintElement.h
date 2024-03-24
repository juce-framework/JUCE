/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
                      public ComponentBoundsConstrainer,
                      private ChangeListener
{
public:
    //==============================================================================
    PaintElement (PaintRoutine* owner, const String& typeName);
    ~PaintElement() override;

    //==============================================================================
    virtual void setInitialBounds (int parentWidth, int parentHeight);

    virtual Rectangle<int> getCurrentBounds (const Rectangle<int>& activeArea) const;
    virtual void setCurrentBounds (const Rectangle<int>& newBounds, const Rectangle<int>& activeArea, bool undoable);

    const RelativePositionedRectangle& getPosition() const;
    void setPosition (const RelativePositionedRectangle& newPosition, bool undoable);
    void setPaintElementBounds (const Rectangle<int>& newBounds, bool undoable);
    void setPaintElementBoundsAndProperties (PaintElement* elementToPosition, const Rectangle<int>& newBounds,
                                             PaintElement* referenceElement, bool undoable);

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

    virtual void selectionChanged (bool isSelected);

    virtual void createSiblingComponents();

    void siblingComponentsChanged();

    OwnedArray<ElementSiblingComponent> siblingComponents;

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
class ElementListener   : private ChangeListener
{
public:
    ElementListener (ElementType* e)
        : owner (e), broadcaster (*owner->getDocument()),
          propToRefresh (nullptr)
    {
        broadcaster.addChangeListener (this);
    }

    ~ElementListener() override
    {
        jassert (propToRefresh != nullptr);
        broadcaster.removeChangeListener (this);
    }

    void setPropertyToRefresh (PropertyComponent& pc)
    {
        propToRefresh = &pc;
    }

    mutable Component::SafePointer<ElementType> owner;
    ChangeBroadcaster& broadcaster;
    PropertyComponent* propToRefresh;

private:
    void changeListenerCallback (ChangeBroadcaster*) override
    {
        jassert (propToRefresh != nullptr);
        if (propToRefresh != nullptr && owner != nullptr)
            propToRefresh->refresh();
    }

    JUCE_DECLARE_NON_COPYABLE (ElementListener)
};
