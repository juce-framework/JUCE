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

#include "jucer_PaintElement.h"
#include "../jucer_ObjectTypes.h"


//==============================================================================
class PaintElementGroup   : public PaintElement
{
public:
    PaintElementGroup (PaintRoutine* pr)
        : PaintElement (pr, "Group")
    {
    }

    void ungroup (const bool undoable)
    {
        getOwner()->getSelectedElements().deselectAll();
        getOwner()->getSelectedPoints().deselectAll();

        const int index = getOwner()->indexOfElement (this);

        for (int i = 0; i < subElements.size(); ++i)
        {
            ScopedPointer<XmlElement> xml (subElements.getUnchecked(i)->createXml());

            PaintElement* newOne = getOwner()->addElementFromXml (*xml, index, undoable);
            getOwner()->getSelectedElements().addToSelection (newOne);
        }

        getOwner()->removeElement (this, undoable);
    }

    static void groupSelected (PaintRoutine* const routine)
    {
        if (routine->getSelectedElements().getNumSelected() > 1)
        {
            PaintElementGroup* newGroup = new PaintElementGroup (routine);

            int frontIndex = -1;

            for (int i = 0; i < routine->getNumElements(); ++i)
            {
                if (routine->getSelectedElements().isSelected (routine->getElement (i)))
                {
                    ScopedPointer<XmlElement> xml (routine->getElement(i)->createXml());

                    if (PaintElement* newOne = ObjectTypes::createElementForXml (xml, routine))
                        newGroup->subElements.add (newOne);

                    if (i > frontIndex)
                        frontIndex = i;
                }
            }

            routine->deleteSelected();

            PaintElement* const g = routine->addNewElement (newGroup, frontIndex, true);
            routine->getSelectedElements().selectOnly (g);
        }
    }

    int getNumElements() const noexcept                              { return subElements.size(); }

    PaintElement* getElement (const int index) const noexcept        { return subElements [index]; }

    int indexOfElement (const PaintElement* element) const noexcept  { return subElements.indexOf (element); }

    bool containsElement (const PaintElement* element) const
    {
        if (subElements.contains (element))
            return true;

        for (int i = subElements.size(); --i >= 0;)
            if (PaintElementGroup* pg = dynamic_cast<PaintElementGroup*> (subElements.getUnchecked(i)))
                if (pg->containsElement (element))
                    return true;

        return false;
    }

    //==============================================================================
    void setInitialBounds (int /*parentWidth*/, int /*parentHeight*/) override
    {
    }

    Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const override
    {
        Rectangle<int> r;

        if (subElements.size() > 0)
        {
            r = subElements.getUnchecked(0)->getCurrentBounds (parentArea);

            for (int i = 1; i < subElements.size(); ++i)
                r = r.getUnion (subElements.getUnchecked(i)->getCurrentBounds (parentArea));
        }

        return r;
    }

    void setCurrentBounds (const Rectangle<int>& b, const Rectangle<int>& parentArea, const bool undoable) override
    {
        Rectangle<int> newBounds (b);
        newBounds.setSize (jmax (1, newBounds.getWidth()),
                           jmax (1, newBounds.getHeight()));

        const Rectangle<int> current (getCurrentBounds (parentArea));

        if (newBounds != current)
        {
            const int dx = newBounds.getX() - current.getX();
            const int dy = newBounds.getY() - current.getY();

            const double scaleStartX = current.getX();
            const double scaleStartY = current.getY();
            const double scaleX = newBounds.getWidth() / (double) current.getWidth();
            const double scaleY = newBounds.getHeight() / (double) current.getHeight();

            for (int i = 0; i < subElements.size(); ++i)
            {
                PaintElement* const e = subElements.getUnchecked(i);

                Rectangle<int> pos (e->getCurrentBounds (parentArea));

                const int newX = roundToInt ((pos.getX() - scaleStartX) * scaleX + scaleStartX + dx);
                const int newY = roundToInt ((pos.getY() - scaleStartY) * scaleY + scaleStartY + dy);

                pos.setBounds (newX, newY,
                               roundToInt ((pos.getRight() - scaleStartX) * scaleX + scaleStartX + dx) - newX,
                               roundToInt ((pos.getBottom() - scaleStartY) * scaleY + scaleStartY + dy) - newY);

                e->setCurrentBounds (pos, parentArea, undoable);
            }
        }
    }

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea) override
    {
        for (int i = 0; i < subElements.size(); ++i)
            subElements.getUnchecked(i)->draw (g, layout, parentArea);
    }

    void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        if (! multipleSelected)
            props.add (new UngroupProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) override
    {
        for (int i = 0; i < subElements.size(); ++i)
            subElements.getUnchecked(i)->fillInGeneratedCode (code, paintMethodCode);
    }

    static const char* getTagName() noexcept        { return "GROUP"; }

    XmlElement* createXml() const override
    {
        XmlElement* e = new XmlElement (getTagName());

        for (int i = 0; i < subElements.size(); ++i)
        {
            XmlElement* const sub = subElements.getUnchecked(i)->createXml();
            e->addChildElement (sub);
        }

        return e;
    }

    bool loadFromXml (const XmlElement& xml) override
    {
        if (xml.hasTagName (getTagName()))
        {
            forEachXmlChildElement (xml, e)
                if (PaintElement* const pe = ObjectTypes::createElementForXml (e, owner))
                    subElements.add (pe);

            return true;
        }

        jassertfalse;
        return false;
    }

    void applyCustomPaintSnippets (StringArray& snippets) override
    {
        for (auto* e : subElements)
            e->applyCustomPaintSnippets (snippets);
    }

private:
    OwnedArray<PaintElement> subElements;

    struct UngroupProperty  : public ButtonPropertyComponent
    {
        UngroupProperty (PaintElementGroup* const e)
            : ButtonPropertyComponent ("ungroup", false),
              element (e)
        {
        }

        void buttonClicked()
        {
            element->ungroup (true);
        }

        String getButtonText() const
        {
            return "Ungroup";
        }

        PaintElementGroup* element;
    };
};
