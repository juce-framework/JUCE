/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../../jucer_Headers.h"
#include "jucer_ComponentViewer.h"


//==============================================================================
ComponentViewer::ComponentViewer (OpenDocumentManager::Document* document_, Project* project_, ComponentDocument* componentDocument_)
    : document (document_), project (project_), componentDocument (componentDocument_)
{
    OpenDocumentManager::getInstance()->addListener (this);

    documentRoot = componentDocument->getRoot();
    documentRoot.addListener (this);

    handleAsyncUpdate();
}

ComponentViewer::~ComponentViewer()
{
    OpenDocumentManager::getInstance()->removeListener (this);
    deleteAllChildren();
}

void ComponentViewer::documentAboutToClose (OpenDocumentManager::Document* closingDoc)
{
    if (document == closingDoc)
    {
        componentDocument = 0;
        document = 0;
        layoutManager = 0;
        documentRoot = ValueTree::invalid;

        triggerAsyncUpdate();
        handleUpdateNowIfNeeded();

        // xxx
    }
}

void ComponentViewer::paint (Graphics& g)
{
    if (componentDocument == 0)
        drawComponentPlaceholder (g, getWidth(), getHeight(), "(Not a valid Jucer component)");
    else
        g.fillAll (background);
}

void ComponentViewer::handleAsyncUpdate()
{
    deleteAllChildren();
    layoutManager = 0;
    background = Colours::transparentBlack;

    if (componentDocument != 0)
    {
        background = Colour::fromString (componentDocument->getBackgroundColour().toString());

        if (layoutManager == 0)
            layoutManager = new ComponentAutoLayoutManager (this);

        int i;
        for (i = getNumChildComponents(); --i >= 0;)
        {
            Component* c = getChildComponent (i);

            if (! componentDocument->containsComponent (c))
                delete c;
        }

        {
            Array <Component*> componentsInOrder;

            const int num = componentDocument->getNumComponents();
            for (i = 0; i < num; ++i)
            {
                const ValueTree v (componentDocument->getComponent (i));
                Component* c = componentDocument->findComponentForState (this, v);

                if (c == 0)
                    addAndMakeVisible (c = componentDocument->createComponent (i));
                else
                    componentDocument->updateComponent (c);

                componentsInOrder.add (c);

                layoutManager->setComponentBounds (c, v [ComponentDocument::memberNameProperty],
                                                   componentDocument->getCoordsFor (v));
            }

            // Make sure the z-order is correct..
            if (num > 0)
            {
                componentsInOrder.getLast()->toFront (false);

                for (i = num - 1; --i >= 0;)
                    componentsInOrder.getUnchecked(i)->toBehind (componentsInOrder.getUnchecked (i + 1));
            }
        }

        setSize (componentDocument->getCanvasWidth().getValue(),
                 componentDocument->getCanvasHeight().getValue());

        for (i = 0; i < componentDocument->getMarkerListX().size(); ++i)
        {
            const ValueTree marker (componentDocument->getMarkerListX().getMarker (i));
            layoutManager->setMarker (componentDocument->getMarkerListX().getName (marker),
                                      componentDocument->getMarkerListX().getCoordinate (marker));
        }

        for (i = 0; i < componentDocument->getMarkerListY().size(); ++i)
        {
            const ValueTree marker (componentDocument->getMarkerListY().getMarker (i));
            layoutManager->setMarker (componentDocument->getMarkerListY().getName (marker),
                                      componentDocument->getMarkerListY().getCoordinate (marker));
        }
    }

    repaint();
}
