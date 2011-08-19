/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCER_PAINTELEMENTGROUP_JUCEHEADER__
#define __JUCER_PAINTELEMENTGROUP_JUCEHEADER__

#include "jucer_PaintElement.h"
#include "../jucer_ObjectTypes.h"


//==============================================================================
/**
*/
class PaintElementGroup   : public PaintElement
{
public:
    //==============================================================================
    PaintElementGroup (PaintRoutine* owner)
        : PaintElement (owner, "Group")
    {
    }

    //==============================================================================
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

                    PaintElement* newOne = ObjectTypes::createElementForXml (xml, routine);
                    if (newOne != 0)
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

    int getNumElements() const throw()                              { return subElements.size(); }

    PaintElement* getElement (const int index) const throw()        { return subElements [index]; }

    int indexOfElement (const PaintElement* element) const throw()  { return subElements.indexOf (element); }

    bool containsElement (const PaintElement* element) const
    {
        if (subElements.contains (element))
            return true;

        for (int i = subElements.size(); --i >= 0;)
        {
            PaintElementGroup* pg = dynamic_cast <PaintElementGroup*> (subElements.getUnchecked(i));

            if (pg != 0 && pg->containsElement (element))
                return true;
        }

        return false;
    }

    //==============================================================================
    void setInitialBounds (int parentWidth, int parentHeight)
    {
    }

    const Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const
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

    void setCurrentBounds (const Rectangle<int>& b, const Rectangle<int>& parentArea, const bool undoable)
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
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea)
    {
        for (int i = 0; i < subElements.size(); ++i)
            subElements.getUnchecked(i)->draw (g, layout, parentArea);
    }

    void getEditableProperties (Array <PropertyComponent*>& properties)
    {
        properties.add (new UngroupProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
    {
        for (int i = 0; i < subElements.size(); ++i)
            subElements.getUnchecked(i)->fillInGeneratedCode (code, paintMethodCode);
    }

    static const char* getTagName() throw()        { return "GROUP"; }

    XmlElement* createXml() const
    {
        XmlElement* e = new XmlElement (getTagName());

        for (int i = 0; i < subElements.size(); ++i)
        {
            XmlElement* const sub = subElements.getUnchecked(i)->createXml();
            e->addChildElement (sub);
        }

        return e;
    }

    bool loadFromXml (const XmlElement& xml)
    {
        if (xml.hasTagName (getTagName()))
        {
            forEachXmlChildElement (xml, e)
            {
                PaintElement* const pe = ObjectTypes::createElementForXml (e, owner);

                if (pe != 0)
                    subElements.add (pe);
            }

            return true;
        }
        else
        {
            jassertfalse
            return false;
        }
    }

private:
    OwnedArray <PaintElement> subElements;

    class UngroupProperty  : public ButtonPropertyComponent
    {
    public:
        UngroupProperty (PaintElementGroup* const element_)
            : ButtonPropertyComponent ("ungroup", false),
              element (element_)
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

    private:
        PaintElementGroup* const element;
    };
};


#endif   // __JUCER_PAINTELEMENTGROUP_JUCEHEADER__
