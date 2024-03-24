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

#include "../../Application/jucer_Headers.h"
#include "jucer_PaintElementGroup.h"

PaintElementGroup::PaintElementGroup (PaintRoutine* pr)
    : PaintElement (pr, "Group")
{
}

PaintElementGroup::~PaintElementGroup() {}

void PaintElementGroup::ungroup (const bool undoable)
{
    getOwner()->getSelectedElements().deselectAll();
    getOwner()->getSelectedPoints().deselectAll();

    const int index = getOwner()->indexOfElement (this);

    for (int i = 0; i < subElements.size(); ++i)
    {
        std::unique_ptr<XmlElement> xml (subElements.getUnchecked (i)->createXml());

        PaintElement* newOne = getOwner()->addElementFromXml (*xml, index, undoable);
        getOwner()->getSelectedElements().addToSelection (newOne);
    }

    getOwner()->removeElement (this, undoable);
}

void PaintElementGroup::groupSelected (PaintRoutine* routine)
{
    if (routine->getSelectedElements().getNumSelected() > 1)
    {
        auto* newGroup = new PaintElementGroup (routine);
        int frontIndex = -1;

        for (int i = 0; i < routine->getNumElements(); ++i)
        {
            if (routine->getSelectedElements().isSelected (routine->getElement (i)))
            {
                std::unique_ptr<XmlElement> xml (routine->getElement (i)->createXml());

                if (auto* newOne = ObjectTypes::createElementForXml (xml.get(), routine))
                    newGroup->subElements.add (newOne);

                if (i > frontIndex)
                    frontIndex = i;
            }
        }

        routine->deleteSelected();

        auto* g = routine->addNewElement (newGroup, frontIndex, true);
        routine->getSelectedElements().selectOnly (g);
    }
}

int PaintElementGroup::getNumElements() const noexcept                              { return subElements.size(); }

PaintElement* PaintElementGroup::getElement (const int index) const noexcept        { return subElements [index]; }

int PaintElementGroup::indexOfElement (const PaintElement* element) const noexcept  { return subElements.indexOf (element); }

bool PaintElementGroup::containsElement (const PaintElement* element) const
{
    if (subElements.contains (element))
        return true;

    for (int i = subElements.size(); --i >= 0;)
        if (PaintElementGroup* pg = dynamic_cast<PaintElementGroup*> (subElements.getUnchecked (i)))
            if (pg->containsElement (element))
                return true;

    return false;
}

//==============================================================================
void PaintElementGroup::setInitialBounds (int /*parentWidth*/, int /*parentHeight*/)
{
}

Rectangle<int> PaintElementGroup::getCurrentBounds (const Rectangle<int>& parentArea) const
{
    Rectangle<int> r;

    if (subElements.size() > 0)
    {
        r = subElements.getUnchecked (0)->getCurrentBounds (parentArea);

        for (int i = 1; i < subElements.size(); ++i)
            r = r.getUnion (subElements.getUnchecked (i)->getCurrentBounds (parentArea));
    }

    return r;
}

void PaintElementGroup::setCurrentBounds (const Rectangle<int>& b, const Rectangle<int>& parentArea, const bool undoable)
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
            PaintElement* const e = subElements.getUnchecked (i);

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
void PaintElementGroup::draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea)
{
    for (int i = 0; i < subElements.size(); ++i)
        subElements.getUnchecked (i)->draw (g, layout, parentArea);
}

void PaintElementGroup::getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected)
{
    if (! multipleSelected)
        props.add (new UngroupProperty (this));
}

void PaintElementGroup::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
{
    for (int i = 0; i < subElements.size(); ++i)
        subElements.getUnchecked (i)->fillInGeneratedCode (code, paintMethodCode);
}

const char* PaintElementGroup::getTagName() noexcept        { return "GROUP"; }

XmlElement* PaintElementGroup::createXml() const
{
    XmlElement* e = new XmlElement (getTagName());

    for (int i = 0; i < subElements.size(); ++i)
    {
        XmlElement* const sub = subElements.getUnchecked (i)->createXml();
        e->addChildElement (sub);
    }

    return e;
}

bool PaintElementGroup::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName (getTagName()))
    {
        for (auto* e : xml.getChildIterator())
            if (PaintElement* const pe = ObjectTypes::createElementForXml (e, owner))
                subElements.add (pe);

        return true;
    }

    jassertfalse;
    return false;
}

void PaintElementGroup::applyCustomPaintSnippets (StringArray& snippets)
{
    for (auto* e : subElements)
        e->applyCustomPaintSnippets (snippets);
}

PaintElementGroup::UngroupProperty::UngroupProperty (PaintElementGroup* const e)
    : ButtonPropertyComponent ("ungroup", false),
      element (e)
{
}

void PaintElementGroup::UngroupProperty::buttonClicked()
{
    element->ungroup (true);
}

String PaintElementGroup::UngroupProperty::getButtonText() const
{
    return "Ungroup";
}
