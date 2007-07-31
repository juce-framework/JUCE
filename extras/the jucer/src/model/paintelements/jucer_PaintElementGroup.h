/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
        : PaintElement (owner, T("Group"))
    {
    }

    ~PaintElementGroup() {}

    //==============================================================================
    void ungroup (const bool undoable)
    {
        getOwner()->getSelectedElements().deselectAll();
        getOwner()->getSelectedPoints().deselectAll();

        const int index = getOwner()->indexOfElement (this);

        for (int i = 0; i < subElements.size(); ++i)
        {
            XmlElement* const xml = subElements.getUnchecked(i)->createXml();

            PaintElement* newOne = getOwner()->addElementFromXml (*xml, index, undoable);
            getOwner()->getSelectedElements().addToSelection (newOne);

            delete xml;
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
                    XmlElement* xml = routine->getElement(i)->createXml();

                    PaintElement* newOne = ObjectTypes::createElementForXml (xml, routine);
                    if (newOne != 0)
                        newGroup->subElements.add (newOne);

                    delete xml;

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

    const Rectangle getCurrentBounds (const Rectangle& parentArea) const
    {
        Rectangle r;

        if (subElements.size() > 0)
        {
            r = subElements.getUnchecked(0)->getCurrentBounds (parentArea);

            for (int i = 1; i < subElements.size(); ++i)
                r = r.getUnion (subElements.getUnchecked(i)->getCurrentBounds (parentArea));
        }

        return r;
    }

    void setCurrentBounds (const Rectangle& b, const Rectangle& parentArea, const bool undoable)
    {
        Rectangle newBounds (b);
        newBounds.setSize (jmax (1, newBounds.getWidth()),
                           jmax (1, newBounds.getHeight()));

        const Rectangle current (getCurrentBounds (parentArea));

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

                Rectangle pos (e->getCurrentBounds (parentArea));

                const int newX = roundDoubleToInt ((pos.getX() - scaleStartX) * scaleX + scaleStartX + dx);
                const int newY = roundDoubleToInt ((pos.getY() - scaleStartY) * scaleY + scaleStartY + dy);

                pos.setBounds (newX, newY,
                               roundDoubleToInt ((pos.getRight() - scaleStartX) * scaleX + scaleStartX + dx) - newX,
                               roundDoubleToInt ((pos.getBottom() - scaleStartY) * scaleY + scaleStartY + dy) - newY);

                e->setCurrentBounds (pos, parentArea, undoable);
            }
        }
    }

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea)
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

    static const tchar* getTagName() throw()        { return T("GROUP"); }

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

    juce_UseDebuggingNewOperator

private:
    OwnedArray <PaintElement> subElements;

    class UngroupProperty  : public ButtonPropertyComponent
    {
    public:
        UngroupProperty (PaintElementGroup* const element_)
            : ButtonPropertyComponent (T("ungroup"), false),
              element (element_)
        {
        }

        void buttonClicked()
        {
            element->ungroup (true);
        }

        const String getButtonText() const
        {
            return T("Ungroup");
        }

    private:
        PaintElementGroup* const element;
    };
};


#endif   // __JUCER_PAINTELEMENTGROUP_JUCEHEADER__
