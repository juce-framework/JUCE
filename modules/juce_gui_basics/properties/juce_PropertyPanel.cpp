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

class PropertyPanel::SectionComponent  : public Component
{
public:
    SectionComponent (const String& sectionTitle,
                      const Array <PropertyComponent*>& newProperties,
                      const bool sectionIsOpen_)
        : Component (sectionTitle),
          titleHeight (sectionTitle.isNotEmpty() ? 22 : 0),
          sectionIsOpen (sectionIsOpen_)
    {
        propertyComps.addArray (newProperties);

        for (int i = propertyComps.size(); --i >= 0;)
        {
            addAndMakeVisible (propertyComps.getUnchecked(i));
            propertyComps.getUnchecked(i)->refresh();
        }
    }

    ~SectionComponent()
    {
        propertyComps.clear();
    }

    void paint (Graphics& g)
    {
        if (titleHeight > 0)
            getLookAndFeel().drawPropertyPanelSectionHeader (g, getName(), isOpen(), getWidth(), titleHeight);
    }

    void resized()
    {
        int y = titleHeight;

        for (int i = 0; i < propertyComps.size(); ++i)
        {
            PropertyComponent* const pec = propertyComps.getUnchecked (i);
            pec->setBounds (1, y, getWidth() - 2, pec->getPreferredHeight());
            y = pec->getBottom();
        }
    }

    int getPreferredHeight() const
    {
        int y = titleHeight;

        if (isOpen())
        {
            for (int i = propertyComps.size(); --i >= 0;)
                y += propertyComps.getUnchecked(i)->getPreferredHeight();
        }

        return y;
    }

    void setOpen (const bool open)
    {
        if (sectionIsOpen != open)
        {
            sectionIsOpen = open;

            for (int i = propertyComps.size(); --i >= 0;)
                propertyComps.getUnchecked(i)->setVisible (open);

            if (PropertyPanel* const pp = findParentComponentOfClass<PropertyPanel>())
                pp->resized();
        }
    }

    bool isOpen() const
    {
        return sectionIsOpen;
    }

    void refreshAll() const
    {
        for (int i = propertyComps.size(); --i >= 0;)
            propertyComps.getUnchecked (i)->refresh();
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
    OwnedArray <PropertyComponent> propertyComps;
    int titleHeight;
    bool sectionIsOpen;

    JUCE_DECLARE_NON_COPYABLE (SectionComponent)
};

//==============================================================================
class PropertyPanel::PropertyHolderComponent  : public Component
{
public:
    PropertyHolderComponent() {}

    void paint (Graphics&) {}

    void updateLayout (int width)
    {
        int y = 0;

        for (int i = 0; i < sections.size(); ++i)
        {
            SectionComponent* const section = sections.getUnchecked(i);

            section->setBounds (0, y, width, section->getPreferredHeight());
            y = section->getBottom();
        }

        setSize (width, y);
        repaint();
    }

    void refreshAll() const
    {
        for (int i = 0; i < sections.size(); ++i)
            sections.getUnchecked(i)->refreshAll();
    }

    void clear()
    {
        sections.clear();
    }

    void addSection (SectionComponent* newSection)
    {
        sections.add (newSection);
        addAndMakeVisible (newSection, 0);
    }

    int getNumSections() const noexcept                     { return sections.size(); }
    SectionComponent* getSection (const int index) const    { return sections [index]; }

private:
    OwnedArray<SectionComponent> sections;

    JUCE_DECLARE_NON_COPYABLE (PropertyHolderComponent)
};


//==============================================================================
PropertyPanel::PropertyPanel()
{
    messageWhenEmpty = TRANS("(nothing selected)");

    addAndMakeVisible (&viewport);
    viewport.setViewedComponent (propertyHolderComponent = new PropertyHolderComponent());
    viewport.setFocusContainer (true);
}

PropertyPanel::~PropertyPanel()
{
    clear();
}

//==============================================================================
void PropertyPanel::paint (Graphics& g)
{
    if (propertyHolderComponent->getNumSections() == 0)
    {
        g.setColour (Colours::black.withAlpha (0.5f));
        g.setFont (14.0f);
        g.drawText (messageWhenEmpty, getLocalBounds().withHeight (30),
                    Justification::centred, true);
    }
}

void PropertyPanel::resized()
{
    viewport.setBounds (getLocalBounds());
    updatePropHolderLayout();
}

//==============================================================================
void PropertyPanel::clear()
{
    if (propertyHolderComponent->getNumSections() > 0)
    {
        propertyHolderComponent->clear();
        repaint();
    }
}

void PropertyPanel::addProperties (const Array <PropertyComponent*>& newProperties)
{
    if (propertyHolderComponent->getNumSections() == 0)
        repaint();

    propertyHolderComponent->addSection (new SectionComponent (String::empty, newProperties, true));
    updatePropHolderLayout();
}

void PropertyPanel::addSection (const String& sectionTitle,
                                const Array <PropertyComponent*>& newProperties,
                                const bool shouldBeOpen)
{
    jassert (sectionTitle.isNotEmpty());

    if (propertyHolderComponent->getNumSections() == 0)
        repaint();

    propertyHolderComponent->addSection (new SectionComponent (sectionTitle, newProperties, shouldBeOpen));
    updatePropHolderLayout();
}

void PropertyPanel::updatePropHolderLayout() const
{
    const int maxWidth = viewport.getMaximumVisibleWidth();
    propertyHolderComponent->updateLayout (maxWidth);

    const int newMaxWidth = viewport.getMaximumVisibleWidth();
    if (maxWidth != newMaxWidth)
    {
        // need to do this twice because of scrollbars changing the size, etc.
        propertyHolderComponent->updateLayout (newMaxWidth);
    }
}

void PropertyPanel::refreshAll() const
{
    propertyHolderComponent->refreshAll();
}

//==============================================================================
StringArray PropertyPanel::getSectionNames() const
{
    StringArray s;

    for (int i = 0; i < propertyHolderComponent->getNumSections(); ++i)
    {
        SectionComponent* const section = propertyHolderComponent->getSection (i);

        if (section->getName().isNotEmpty())
            s.add (section->getName());
    }

    return s;
}

bool PropertyPanel::isSectionOpen (const int sectionIndex) const
{
    int index = 0;

    for (int i = 0; i < propertyHolderComponent->getNumSections(); ++i)
    {
        SectionComponent* const section = propertyHolderComponent->getSection (i);

        if (section->getName().isNotEmpty())
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

    for (int i = 0; i < propertyHolderComponent->getNumSections(); ++i)
    {
        SectionComponent* const section = propertyHolderComponent->getSection (i);

        if (section->getName().isNotEmpty())
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

    for (int i = 0; i < propertyHolderComponent->getNumSections(); ++i)
    {
        SectionComponent* const section = propertyHolderComponent->getSection (i);

        if (section->getName().isNotEmpty())
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
    XmlElement* const xml = new XmlElement ("PROPERTYPANELSTATE");

    xml->setAttribute ("scrollPos", viewport.getViewPositionY());

    const StringArray sections (getSectionNames());

    for (int i = 0; i < sections.size(); ++i)
    {
        if (sections[i].isNotEmpty())
        {
            XmlElement* const e = xml->createNewChildElement ("SECTION");
            e->setAttribute ("name", sections[i]);
            e->setAttribute ("open", isSectionOpen (i) ? 1 : 0);
        }
    }

    return xml;
}

void PropertyPanel::restoreOpennessState (const XmlElement& xml)
{
    if (xml.hasTagName ("PROPERTYPANELSTATE"))
    {
        const StringArray sections (getSectionNames());

        forEachXmlChildElementWithTagName (xml, e, "SECTION")
        {
            setSectionOpen (sections.indexOf (e->getStringAttribute ("name")),
                            e->getBoolAttribute ("open"));
        }

        viewport.setViewPosition (viewport.getViewPositionX(),
                                  xml.getIntAttribute ("scrollPos", viewport.getViewPositionY()));
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
