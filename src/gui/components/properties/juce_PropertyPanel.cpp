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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_PropertyPanel.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../text/juce_LocalisedStrings.h"


//==============================================================================
class PropertyHolderComponent  : public Component
{
public:
    PropertyHolderComponent()
    {
    }

    ~PropertyHolderComponent()
    {
        deleteAllChildren();
    }

    void paint (Graphics&)
    {
    }

    void updateLayout (const int width);

    void refreshAll() const;
};

//==============================================================================
class PropertySectionComponent  : public Component
{
public:
    PropertySectionComponent (const String& sectionTitle,
                              const Array <PropertyComponent*>& newProperties,
                              const bool open)
        : Component (sectionTitle),
          titleHeight (sectionTitle.isNotEmpty() ? 22 : 0),
          isOpen_ (open)
    {
        for (int i = newProperties.size(); --i >= 0;)
        {
            addAndMakeVisible (newProperties.getUnchecked(i));
            newProperties.getUnchecked(i)->refresh();
        }
    }

    ~PropertySectionComponent()
    {
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        if (titleHeight > 0)
            getLookAndFeel().drawPropertyPanelSectionHeader (g, getName(), isOpen(), getWidth(), titleHeight);
    }

    void resized()
    {
        int y = titleHeight;

        for (int i = getNumChildComponents(); --i >= 0;)
        {
            PropertyComponent* const pec = dynamic_cast <PropertyComponent*> (getChildComponent (i));

            if (pec != 0)
            {
                const int prefH = pec->getPreferredHeight();
                pec->setBounds (1, y, getWidth() - 2, prefH);
                y += prefH;
            }
        }
    }

    int getPreferredHeight() const
    {
        int y = titleHeight;

        if (isOpen())
        {
            for (int i = 0; i < getNumChildComponents(); ++i)
            {
                PropertyComponent* pec = dynamic_cast <PropertyComponent*> (getChildComponent (i));

                if (pec != 0)
                    y += pec->getPreferredHeight();
            }
        }

        return y;
    }

    void setOpen (const bool open)
    {
        if (isOpen_ != open)
        {
            isOpen_ = open;

            for (int i = 0; i < getNumChildComponents(); ++i)
            {
                PropertyComponent* pec = dynamic_cast <PropertyComponent*> (getChildComponent (i));

                if (pec != 0)
                    pec->setVisible (open);
            }

            // (unable to use the syntax findParentComponentOfClass <DragAndDropContainer> () because of a VC6 compiler bug)
            PropertyPanel* const pp = findParentComponentOfClass ((PropertyPanel*) 0);

            if (pp != 0)
                pp->resized();
        }
    }

    bool isOpen() const
    {
        return isOpen_;
    }

    void refreshAll() const
    {
        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            PropertyComponent* pec = dynamic_cast <PropertyComponent*> (getChildComponent (i));

            if (pec != 0)
                pec->refresh();
        }
    }

    void mouseDown (const MouseEvent&)
    {
    }

    void mouseUp (const MouseEvent& e)
    {
        if (e.getMouseDownX() < titleHeight
             && e.x < titleHeight
             && e.y < titleHeight
             && e.getNumberOfClicks() != 2)
        {
            setOpen (! isOpen());
        }
    }

    void mouseDoubleClick (const MouseEvent& e)
    {
        if (e.y < titleHeight)
            setOpen (! isOpen());
    }

private:
    int titleHeight;
    bool isOpen_;
};

void PropertyHolderComponent::updateLayout (const int width)
{
    int y = 0;

    for (int i = getNumChildComponents(); --i >= 0;)
    {
        PropertySectionComponent* const section
            = dynamic_cast <PropertySectionComponent*> (getChildComponent (i));

        if (section != 0)
        {
            const int prefH = section->getPreferredHeight();
            section->setBounds (0, y, width, prefH);
            y += prefH;
        }
    }

    setSize (width, y);
    repaint();
}

void PropertyHolderComponent::refreshAll() const
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        PropertySectionComponent* const section
            = dynamic_cast <PropertySectionComponent*> (getChildComponent (i));

        if (section != 0)
            section->refreshAll();
    }
}

//==============================================================================
PropertyPanel::PropertyPanel()
{
    messageWhenEmpty = TRANS("(nothing selected)");

    addAndMakeVisible (viewport = new Viewport());
    viewport->setViewedComponent (propertyHolderComponent = new PropertyHolderComponent());
    viewport->setFocusContainer (true);
}

PropertyPanel::~PropertyPanel()
{
    clear();
    deleteAllChildren();
}

//==============================================================================
void PropertyPanel::paint (Graphics& g)
{
    if (propertyHolderComponent->getNumChildComponents() == 0)
    {
        g.setColour (Colours::black.withAlpha (0.5f));
        g.setFont (14.0f);
        g.drawText (messageWhenEmpty, 0, 0, getWidth(), 30,
                    Justification::centred, true);
    }
}

void PropertyPanel::resized()
{
    viewport->setBounds (0, 0, getWidth(), getHeight());
    updatePropHolderLayout();
}

//==============================================================================
void PropertyPanel::clear()
{
    if (propertyHolderComponent->getNumChildComponents() > 0)
    {
        propertyHolderComponent->deleteAllChildren();
        repaint();
    }
}

void PropertyPanel::addProperties (const Array <PropertyComponent*>& newProperties)
{
    if (propertyHolderComponent->getNumChildComponents() == 0)
        repaint();

    propertyHolderComponent->addAndMakeVisible (new PropertySectionComponent (String::empty,
                                                                              newProperties,
                                                                              true), 0);
    updatePropHolderLayout();
}

void PropertyPanel::addSection (const String& sectionTitle,
                                const Array <PropertyComponent*>& newProperties,
                                const bool shouldBeOpen)
{
    jassert (sectionTitle.isNotEmpty());

    if (propertyHolderComponent->getNumChildComponents() == 0)
        repaint();

    propertyHolderComponent->addAndMakeVisible (new PropertySectionComponent (sectionTitle,
                                                                              newProperties,
                                                                              shouldBeOpen), 0);

    updatePropHolderLayout();
}

void PropertyPanel::updatePropHolderLayout() const
{
    const int maxWidth = viewport->getMaximumVisibleWidth();
    ((PropertyHolderComponent*) propertyHolderComponent)->updateLayout (maxWidth);

    const int newMaxWidth = viewport->getMaximumVisibleWidth();
    if (maxWidth != newMaxWidth)
    {
        // need to do this twice because of scrollbars changing the size, etc.
        ((PropertyHolderComponent*) propertyHolderComponent)->updateLayout (newMaxWidth);
    }
}

void PropertyPanel::refreshAll() const
{
    ((PropertyHolderComponent*) propertyHolderComponent)->refreshAll();
}

//==============================================================================
const StringArray PropertyPanel::getSectionNames() const
{
    StringArray s;

    for (int i = 0; i < propertyHolderComponent->getNumChildComponents(); ++i)
    {
        PropertySectionComponent* const section = dynamic_cast <PropertySectionComponent*> (propertyHolderComponent->getChildComponent (i));

        if (section != 0 && section->getName().isNotEmpty())
            s.add (section->getName());
    }

    return s;
}

bool PropertyPanel::isSectionOpen (const int sectionIndex) const
{
    int index = 0;

    for (int i = 0; i < propertyHolderComponent->getNumChildComponents(); ++i)
    {
        PropertySectionComponent* const section = dynamic_cast <PropertySectionComponent*> (propertyHolderComponent->getChildComponent (i));

        if (section != 0 && section->getName().isNotEmpty())
        {
            if (index == sectionIndex)
                return section->isOpen();

            ++index;
        }
    }

    return false;
}

void PropertyPanel::setSectionOpen (const int sectionIndex, const bool shouldBeOpen)
{
    int index = 0;

    for (int i = 0; i < propertyHolderComponent->getNumChildComponents(); ++i)
    {
        PropertySectionComponent* const section = dynamic_cast <PropertySectionComponent*> (propertyHolderComponent->getChildComponent (i));

        if (section != 0 && section->getName().isNotEmpty())
        {
            if (index == sectionIndex)
            {
                section->setOpen (shouldBeOpen);
                break;
            }

            ++index;
        }
    }
}

void PropertyPanel::setSectionEnabled (const int sectionIndex, const bool shouldBeEnabled)
{
    int index = 0;

    for (int i = 0; i < propertyHolderComponent->getNumChildComponents(); ++i)
    {
        PropertySectionComponent* const section = dynamic_cast <PropertySectionComponent*> (propertyHolderComponent->getChildComponent (i));

        if (section != 0 && section->getName().isNotEmpty())
        {
            if (index == sectionIndex)
            {
                section->setEnabled (shouldBeEnabled);
                break;
            }

            ++index;
        }
    }
}

//==============================================================================
XmlElement* PropertyPanel::getOpennessState() const
{
    XmlElement* const xml = new XmlElement (T("PROPERTYPANELSTATE"));

    const StringArray sections (getSectionNames());

    for (int i = 0; i < sections.size(); ++i)
    {
        if (sections[i].isNotEmpty())
        {
            XmlElement* const e = new XmlElement (T("SECTION"));
            e->setAttribute (T("name"), sections[i]);
            e->setAttribute (T("open"), isSectionOpen (i) ? 1 : 0);
            xml->addChildElement (e);
        }
    }

    return xml;
}

void PropertyPanel::restoreOpennessState (const XmlElement& xml)
{
    if (xml.hasTagName (T("PROPERTYPANELSTATE")))
    {
        const StringArray sections (getSectionNames());

        forEachXmlChildElementWithTagName (xml, e, T("SECTION"))
        {
            setSectionOpen (sections.indexOf (e->getStringAttribute (T("name"))),
                            e->getBoolAttribute (T("open")));
        }
    }
}

//==============================================================================
void PropertyPanel::setMessageWhenEmpty (const String& newMessage)
{
    if (messageWhenEmpty != newMessage)
    {
        messageWhenEmpty = newMessage;
        repaint();
    }
}

const String& PropertyPanel::getMessageWhenEmpty() const
{
    return messageWhenEmpty;
}


END_JUCE_NAMESPACE
