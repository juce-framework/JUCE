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

namespace juce
{

struct PropertyPanel::SectionComponent final : public Component
{
    SectionComponent (const String& sectionTitle,
                      const Array<PropertyComponent*>& newProperties,
                      bool sectionIsOpen,
                      int extraPadding)
        : Component (sectionTitle),
          isOpen (sectionIsOpen),
          padding (extraPadding)
    {
        lookAndFeelChanged();

        propertyComps.addArray (newProperties);

        for (auto* propertyComponent : propertyComps)
        {
            addAndMakeVisible (propertyComponent);
            propertyComponent->refresh();
        }
    }

    ~SectionComponent() override
    {
        propertyComps.clear();
    }

    void paint (Graphics& g) override
    {
        if (titleHeight > 0)
            getLookAndFeel().drawPropertyPanelSectionHeader (g, getName(), isOpen, getWidth(), titleHeight);
    }

    void resized() override
    {
        auto y = titleHeight;

        for (auto* propertyComponent : propertyComps)
        {
            propertyComponent->setBounds (1, y, getWidth() - 2, propertyComponent->getPreferredHeight());
            y = propertyComponent->getBottom() + padding;
        }
    }

    void lookAndFeelChanged() override
    {
        titleHeight = getLookAndFeel().getPropertyPanelSectionHeaderHeight (getName());
        resized();
        repaint();
    }

    int getPreferredHeight() const
    {
        auto y = titleHeight;

        auto numComponents = propertyComps.size();

        if (numComponents > 0 && isOpen)
        {
            for (auto* propertyComponent : propertyComps)
                y += propertyComponent->getPreferredHeight();

            y += (numComponents - 1) * padding;
        }

        return y;
    }

    void setOpen (bool open)
    {
        if (isOpen != open)
        {
            isOpen = open;

            for (auto* propertyComponent : propertyComps)
                propertyComponent->setVisible (open);

            if (auto* propertyPanel = findParentComponentOfClass<PropertyPanel>())
                propertyPanel->resized();
        }
    }

    void refreshAll() const
    {
        for (auto* propertyComponent : propertyComps)
            propertyComponent->refresh();
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (e.getMouseDownX() < titleHeight
              && e.x < titleHeight
              && e.getNumberOfClicks() != 2)
            mouseDoubleClick (e);
    }

    void mouseDoubleClick (const MouseEvent& e) override
    {
        if (e.y < titleHeight)
            setOpen (! isOpen);
    }

    OwnedArray<PropertyComponent> propertyComps;
    int titleHeight;
    bool isOpen;
    int padding;

    JUCE_DECLARE_NON_COPYABLE (SectionComponent)
};

//==============================================================================
struct PropertyPanel::PropertyHolderComponent final : public Component
{
    PropertyHolderComponent() {}

    void paint (Graphics&) override {}

    void updateLayout (int width)
    {
        auto y = 0;

        for (auto* section : sections)
        {
            section->setBounds (0, y, width, section->getPreferredHeight());
            y = section->getBottom();
        }

        setSize (width, y);
        repaint();
    }

    void refreshAll() const
    {
        for (auto* section : sections)
            section->refreshAll();
    }

    void insertSection (int indexToInsertAt, SectionComponent* newSection)
    {
        sections.insert (indexToInsertAt, newSection);
        addAndMakeVisible (newSection, 0);
    }

    SectionComponent* getSectionWithNonEmptyName (int targetIndex) const noexcept
    {
        auto index = 0;
        for (auto* section : sections)
        {
            if (section->getName().isNotEmpty())
                if (index++ == targetIndex)
                    return section;
        }

        return nullptr;
    }

    OwnedArray<SectionComponent> sections;

    JUCE_DECLARE_NON_COPYABLE (PropertyHolderComponent)
};


//==============================================================================
PropertyPanel::PropertyPanel()
{
    init();
}

PropertyPanel::PropertyPanel (const String& name)  : Component (name)
{
    init();
}

void PropertyPanel::init()
{
    messageWhenEmpty = TRANS ("(nothing selected)");

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (propertyHolderComponent = new PropertyHolderComponent());
    viewport.setFocusContainerType (FocusContainerType::keyboardFocusContainer);
}

PropertyPanel::~PropertyPanel()
{
    clear();
}

//==============================================================================
void PropertyPanel::paint (Graphics& g)
{
    if (isEmpty())
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
    if (! isEmpty())
    {
        propertyHolderComponent->sections.clear();
        updatePropHolderLayout();
    }
}

bool PropertyPanel::isEmpty() const
{
    return propertyHolderComponent->sections.size() == 0;
}

int PropertyPanel::getTotalContentHeight() const
{
    return propertyHolderComponent->getHeight();
}

void PropertyPanel::addProperties (const Array<PropertyComponent*>& newProperties,
                                   int extraPaddingBetweenComponents)
{
    if (isEmpty())
        repaint();

    propertyHolderComponent->insertSection (-1, new SectionComponent ({}, newProperties, true, extraPaddingBetweenComponents));
    updatePropHolderLayout();
}

void PropertyPanel::addSection (const String& sectionTitle,
                                const Array<PropertyComponent*>& newProperties,
                                bool shouldBeOpen,
                                int indexToInsertAt,
                                int extraPaddingBetweenComponents)
{
    jassert (sectionTitle.isNotEmpty());

    if (isEmpty())
        repaint();

    propertyHolderComponent->insertSection (indexToInsertAt, new SectionComponent (sectionTitle,
                                                                                   newProperties,
                                                                                   shouldBeOpen,
                                                                                   extraPaddingBetweenComponents));

    updatePropHolderLayout();
}

void PropertyPanel::updatePropHolderLayout() const
{
    auto maxWidth = viewport.getMaximumVisibleWidth();
    propertyHolderComponent->updateLayout (maxWidth);

    auto newMaxWidth = viewport.getMaximumVisibleWidth();
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

    for (auto* section : propertyHolderComponent->sections)
    {
        if (section->getName().isNotEmpty())
            s.add (section->getName());
    }

    return s;
}

bool PropertyPanel::isSectionOpen (int sectionIndex) const
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
        return s->isOpen;

    return false;
}

void PropertyPanel::setSectionOpen (int sectionIndex, bool shouldBeOpen)
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
        s->setOpen (shouldBeOpen);
}

void PropertyPanel::setSectionEnabled (int sectionIndex, bool shouldBeEnabled)
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
        s->setEnabled (shouldBeEnabled);
}

void PropertyPanel::removeSection (int sectionIndex)
{
    if (auto* s = propertyHolderComponent->getSectionWithNonEmptyName (sectionIndex))
    {
        propertyHolderComponent->sections.removeObject (s);
        updatePropHolderLayout();
    }
}

//==============================================================================
std::unique_ptr<XmlElement> PropertyPanel::getOpennessState() const
{
    auto xml = std::make_unique<XmlElement> ("PROPERTYPANELSTATE");

    xml->setAttribute ("scrollPos", viewport.getViewPositionY());

    auto sections = getSectionNames();
    for (auto s : sections)
    {
        if (s.isNotEmpty())
        {
            auto* e = xml->createNewChildElement ("SECTION");
            e->setAttribute ("name", s);
            e->setAttribute ("open", isSectionOpen (sections.indexOf (s)) ? 1 : 0);
        }
    }

    return xml;
}

void PropertyPanel::restoreOpennessState (const XmlElement& xml)
{
    if (xml.hasTagName ("PROPERTYPANELSTATE"))
    {
        auto sections = getSectionNames();

        for (auto* e : xml.getChildWithTagNameIterator ("SECTION"))
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

const String& PropertyPanel::getMessageWhenEmpty() const noexcept
{
    return messageWhenEmpty;
}

} // namespace juce
