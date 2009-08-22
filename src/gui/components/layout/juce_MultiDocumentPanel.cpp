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

#include "juce_MultiDocumentPanel.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
MultiDocumentPanelWindow::MultiDocumentPanelWindow (const Colour& backgroundColour)
    : DocumentWindow (String::empty, backgroundColour,
                      DocumentWindow::maximiseButton | DocumentWindow::closeButton, false)
{
}

MultiDocumentPanelWindow::~MultiDocumentPanelWindow()
{
}

//==============================================================================
void MultiDocumentPanelWindow::maximiseButtonPressed()
{
    MultiDocumentPanel* const owner = getOwner();

    jassert (owner != 0); // these windows are only designed to be used inside a MultiDocumentPanel!
    if (owner != 0)
        owner->setLayoutMode (MultiDocumentPanel::MaximisedWindowsWithTabs);
}

void MultiDocumentPanelWindow::closeButtonPressed()
{
    MultiDocumentPanel* const owner = getOwner();

    jassert (owner != 0); // these windows are only designed to be used inside a MultiDocumentPanel!
    if (owner != 0)
        owner->closeDocument (getContentComponent(), true);
}

void MultiDocumentPanelWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();
    updateOrder();
}

void MultiDocumentPanelWindow::broughtToFront()
{
    DocumentWindow::broughtToFront();
    updateOrder();
}

void MultiDocumentPanelWindow::updateOrder()
{
    MultiDocumentPanel* const owner = getOwner();

    if (owner != 0)
        owner->updateOrder();
}

MultiDocumentPanel* MultiDocumentPanelWindow::getOwner() const throw()
{
    // (unable to use the syntax findParentComponentOfClass <MultiDocumentPanel> () because of a VC6 compiler bug)
    return findParentComponentOfClass ((MultiDocumentPanel*) 0);
}


//==============================================================================
class MDITabbedComponentInternal   : public TabbedComponent
{
public:
    MDITabbedComponentInternal()
        : TabbedComponent (TabbedButtonBar::TabsAtTop)
    {
    }

    ~MDITabbedComponentInternal()
    {
    }

    void currentTabChanged (const int, const String&)
    {
        // (unable to use the syntax findParentComponentOfClass <MultiDocumentPanel> () because of a VC6 compiler bug)
        MultiDocumentPanel* const owner = findParentComponentOfClass ((MultiDocumentPanel*) 0);

        if (owner != 0)
            owner->updateOrder();
    }
};


//==============================================================================
MultiDocumentPanel::MultiDocumentPanel()
    : mode (MaximisedWindowsWithTabs),
      tabComponent (0),
      backgroundColour (Colours::lightblue),
      maximumNumDocuments (0),
      numDocsBeforeTabsUsed (0)
{
    setOpaque (true);
}

MultiDocumentPanel::~MultiDocumentPanel()
{
    closeAllDocuments (false);
}

//==============================================================================
static bool shouldDeleteComp (Component* const c)
{
    return c->getComponentPropertyBool (T("mdiDocumentDelete_"), false);
}

bool MultiDocumentPanel::closeAllDocuments (const bool checkItsOkToCloseFirst)
{
    while (components.size() > 0)
        if (! closeDocument (components.getLast(), checkItsOkToCloseFirst))
            return false;

    return true;
}

MultiDocumentPanelWindow* MultiDocumentPanel::createNewDocumentWindow()
{
    return new MultiDocumentPanelWindow (backgroundColour);
}

void MultiDocumentPanel::addWindow (Component* component)
{
    MultiDocumentPanelWindow* const dw = createNewDocumentWindow();

    dw->setResizable (true, false);
    dw->setContentComponent (component, false, true);
    dw->setName (component->getName());
    dw->setBackgroundColour (component->getComponentPropertyColour (T("mdiDocumentBkg_"), false, backgroundColour));

    int x = 4;
    Component* const topComp = getChildComponent (getNumChildComponents() - 1);

    if (topComp != 0 && topComp->getX() == x && topComp->getY() == x)
        x += 16;

    dw->setTopLeftPosition (x, x);

    if (component->getComponentProperty (T("mdiDocumentPos_"), false, String::empty).isNotEmpty())
        dw->restoreWindowStateFromString (component->getComponentProperty (T("mdiDocumentPos_"), false, String::empty));

    addAndMakeVisible (dw);
    dw->toFront (true);
}

bool MultiDocumentPanel::addDocument (Component* const component,
                                      const Colour& docColour,
                                      const bool deleteWhenRemoved)
{
    // If you try passing a full DocumentWindow or ResizableWindow in here, you'll end up
    // with a frame-within-a-frame! Just pass in the bare content component.
    jassert (dynamic_cast <ResizableWindow*> (component) == 0);

    if (component == 0 || (maximumNumDocuments > 0 && components.size() >= maximumNumDocuments))
        return false;

    components.add (component);
    component->setComponentProperty (T("mdiDocumentDelete_"), deleteWhenRemoved);
    component->setComponentProperty (T("mdiDocumentBkg_"), docColour);
    component->addComponentListener (this);

    if (mode == FloatingWindows)
    {
        if (isFullscreenWhenOneDocument())
        {
            if (components.size() == 1)
            {
                addAndMakeVisible (component);
            }
            else
            {
                if (components.size() == 2)
                    addWindow (components.getFirst());

                addWindow (component);
            }
        }
        else
        {
           addWindow (component);
        }
    }
    else
    {
        if (tabComponent == 0 && components.size() > numDocsBeforeTabsUsed)
        {
            addAndMakeVisible (tabComponent = new MDITabbedComponentInternal());

            Array <Component*> temp (components);

            for (int i = 0; i < temp.size(); ++i)
                tabComponent->addTab (temp[i]->getName(), docColour, temp[i], false);

            resized();
        }
        else
        {
            if (tabComponent != 0)
                tabComponent->addTab (component->getName(), docColour, component, false);
            else
                addAndMakeVisible (component);
        }

        setActiveDocument (component);
    }

    resized();
    activeDocumentChanged();
    return true;
}

bool MultiDocumentPanel::closeDocument (Component* component,
                                        const bool checkItsOkToCloseFirst)
{
    if (components.contains (component))
    {
        if (checkItsOkToCloseFirst && ! tryToCloseDocument (component))
            return false;

        component->removeComponentListener (this);

        const bool shouldDelete = shouldDeleteComp (component);
        component->removeComponentProperty (T("mdiDocumentDelete_"));
        component->removeComponentProperty (T("mdiDocumentBkg_"));

        if (mode == FloatingWindows)
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                MultiDocumentPanelWindow* const dw = dynamic_cast <MultiDocumentPanelWindow*> (getChildComponent (i));

                if (dw != 0 && dw->getContentComponent() == component)
                {
                    dw->setContentComponent (0, false);
                    delete dw;
                    break;
                }
            }

            if (shouldDelete)
                delete component;

            components.removeValue (component);

            if (isFullscreenWhenOneDocument() && components.size() == 1)
            {
                for (int i = getNumChildComponents(); --i >= 0;)
                {
                    MultiDocumentPanelWindow* const dw = dynamic_cast <MultiDocumentPanelWindow*> (getChildComponent (i));

                    if (dw != 0)
                    {
                        dw->setContentComponent (0, false);
                        delete dw;
                    }
                }

                addAndMakeVisible (components.getFirst());
            }
        }
        else
        {
            jassert (components.indexOf (component) >= 0);

            if (tabComponent != 0)
            {
                for (int i = tabComponent->getNumTabs(); --i >= 0;)
                    if (tabComponent->getTabContentComponent (i) == component)
                        tabComponent->removeTab (i);
            }
            else
            {
                removeChildComponent (component);
            }

            if (shouldDelete)
                delete component;

            if (tabComponent != 0 && tabComponent->getNumTabs() <= numDocsBeforeTabsUsed)
                deleteAndZero (tabComponent);

            components.removeValue (component);

            if (components.size() > 0 && tabComponent == 0)
                addAndMakeVisible (components.getFirst());
        }

        resized();
        activeDocumentChanged();
    }
    else
    {
        jassertfalse
    }

    return true;
}

int MultiDocumentPanel::getNumDocuments() const throw()
{
    return components.size();
}

Component* MultiDocumentPanel::getDocument (const int index) const throw()
{
    return components [index];
}

Component* MultiDocumentPanel::getActiveDocument() const throw()
{
    if (mode == FloatingWindows)
    {
        for (int i = getNumChildComponents(); --i >= 0;)
        {
            MultiDocumentPanelWindow* const dw = dynamic_cast <MultiDocumentPanelWindow*> (getChildComponent (i));

            if (dw != 0 && dw->isActiveWindow())
                return dw->getContentComponent();
        }
    }

    return components.getLast();
}

void MultiDocumentPanel::setActiveDocument (Component* component)
{
    if (mode == FloatingWindows)
    {
        component = getContainerComp (component);

        if (component != 0)
            component->toFront (true);
    }
    else if (tabComponent != 0)
    {
        jassert (components.indexOf (component) >= 0);

        for (int i = tabComponent->getNumTabs(); --i >= 0;)
        {
            if (tabComponent->getTabContentComponent (i) == component)
            {
                tabComponent->setCurrentTabIndex (i);
                break;
            }
        }
    }
    else
    {
        component->grabKeyboardFocus();
    }
}

void MultiDocumentPanel::activeDocumentChanged()
{
}

void MultiDocumentPanel::setMaximumNumDocuments (const int newNumber)
{
    maximumNumDocuments = newNumber;
}

void MultiDocumentPanel::useFullscreenWhenOneDocument (const bool shouldUseTabs)
{
    numDocsBeforeTabsUsed = shouldUseTabs ? 1 : 0;
}

bool MultiDocumentPanel::isFullscreenWhenOneDocument() const throw()
{
    return numDocsBeforeTabsUsed != 0;
}

//==============================================================================
void MultiDocumentPanel::setLayoutMode (const LayoutMode newLayoutMode)
{
    if (mode != newLayoutMode)
    {
        mode = newLayoutMode;

        if (mode == FloatingWindows)
        {
            deleteAndZero (tabComponent);
        }
        else
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                MultiDocumentPanelWindow* const dw = dynamic_cast <MultiDocumentPanelWindow*> (getChildComponent (i));

                if (dw != 0)
                {
                    dw->getContentComponent()->setComponentProperty (T("mdiDocumentPos_"), dw->getWindowStateAsString());
                    dw->setContentComponent (0, false);
                    delete dw;
                }
            }
        }

        resized();

        const Array <Component*> tempComps (components);
        components.clear();

        for (int i = 0; i < tempComps.size(); ++i)
        {
            Component* const c = tempComps.getUnchecked(i);
            addDocument (c,
                         c->getComponentPropertyColour (T("mdiDocumentBkg_"), false, Colours::white),
                         shouldDeleteComp (c));
        }
    }
}

void MultiDocumentPanel::setBackgroundColour (const Colour& newBackgroundColour)
{
    if (backgroundColour != newBackgroundColour)
    {
        backgroundColour = newBackgroundColour;
        setOpaque (newBackgroundColour.isOpaque());
        repaint();
    }
}

//==============================================================================
void MultiDocumentPanel::paint (Graphics& g)
{
    g.fillAll (backgroundColour);
}

void MultiDocumentPanel::resized()
{
    if (mode == MaximisedWindowsWithTabs || components.size() == numDocsBeforeTabsUsed)
    {
        for (int i = getNumChildComponents(); --i >= 0;)
            getChildComponent (i)->setBounds (0, 0, getWidth(), getHeight());
    }

    setWantsKeyboardFocus (components.size() == 0);
}

Component* MultiDocumentPanel::getContainerComp (Component* c) const
{
    if (mode == FloatingWindows)
    {
        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            MultiDocumentPanelWindow* const dw = dynamic_cast <MultiDocumentPanelWindow*> (getChildComponent (i));

            if (dw != 0 && dw->getContentComponent() == c)
            {
                c = dw;
                break;
            }
        }
    }

    return c;
}

void MultiDocumentPanel::componentNameChanged (Component&)
{
    if (mode == FloatingWindows)
    {
        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            MultiDocumentPanelWindow* const dw = dynamic_cast <MultiDocumentPanelWindow*> (getChildComponent (i));

            if (dw != 0)
                dw->setName (dw->getContentComponent()->getName());
        }
    }
    else if (tabComponent != 0)
    {
        for (int i = tabComponent->getNumTabs(); --i >= 0;)
            tabComponent->setTabName (i, tabComponent->getTabContentComponent (i)->getName());
    }
}

void MultiDocumentPanel::updateOrder()
{
    const Array <Component*> oldList (components);

    if (mode == FloatingWindows)
    {
        components.clear();

        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            MultiDocumentPanelWindow* const dw = dynamic_cast <MultiDocumentPanelWindow*> (getChildComponent (i));

            if (dw != 0)
                components.add (dw->getContentComponent());
        }
    }
    else
    {
        if (tabComponent != 0)
        {
            Component* const current = tabComponent->getCurrentContentComponent();

            if (current != 0)
            {
                components.removeValue (current);
                components.add (current);
            }
        }
    }

    if (components != oldList)
        activeDocumentChanged();
}


END_JUCE_NAMESPACE
