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

#include "../../jucer_Headers.h"
#include "jucer_ComponentDocument.h"


//==============================================================================
ComponentDocument::ComponentDocument()
{
    components = new ComponentLayout();
    components->setDocument (this);

    backgroundGraphics = new PaintRoutine();
    backgroundGraphics->setDocument (this);
}

ComponentDocument::~ComponentDocument()
{
    delete components;
    delete backgroundGraphics;
}

//==============================================================================
const String ComponentDocument::getTypeName() const
{
    return T("Component");
}

JucerDocument* ComponentDocument::createCopy()
{
    ComponentDocument* newOne = new ComponentDocument();

    newOne->resources = resources;
    newOne->setFile (getFile());

    XmlElement* const xml = createXml();
    newOne->loadFromXml (*xml);
    delete xml;

    return newOne;
}

XmlElement* ComponentDocument::createXml() const
{
    XmlElement* const doc = JucerDocument::createXml();

    doc->addChildElement (backgroundGraphics->createXml());
    components->addToXml (*doc);

    return doc;
}

bool ComponentDocument::loadFromXml (const XmlElement& xml)
{
    if (JucerDocument::loadFromXml (xml))
    {
        components->clearComponents();

        forEachXmlChildElement (xml, e)
        {
            if (e->hasTagName (PaintRoutine::xmlTagName))
                backgroundGraphics->loadFromXml (*e);
            else
                components->addComponentFromXml (*e, false);
        }

        changed();
        getUndoManager().clearUndoHistory();
        return true;
    }

    return false;
}

//==============================================================================
class NormalTestComponent   : public Component
{
public:
    NormalTestComponent (ComponentDocument* const document_, const bool alwaysFillBackground_)
        : document (document_),
          alwaysFillBackground (alwaysFillBackground_)
    {
        ComponentLayout* const layout = document->getComponentLayout();

        for (int i = 0; i < layout->getNumComponents(); ++i)
            addAndMakeVisible (layout->getComponent (i));
    }

    ~NormalTestComponent()
    {
        for (int i = getNumChildComponents(); --i >= 0;)
            removeChildComponent (i);
    }

    void paint (Graphics& g)
    {
        document->getPaintRoutine (0)->fillWithBackground (g, alwaysFillBackground);
        document->getPaintRoutine (0)->drawElements (g, Rectangle (0, 0, getWidth(), getHeight()));
    }

    void resized()
    {
        if (! getBounds().isEmpty())
        {
            int numTimesToTry = 10;

            while (--numTimesToTry >= 0)
            {
                bool anyCompsMoved = false;

                for (int i = 0; i < getNumChildComponents(); ++i)
                {
                    Component* comp = getChildComponent (i);
                    ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*comp);

                    if (type != 0)
                    {
                        const Rectangle newBounds (type->getComponentPosition (comp)
                                                    .getRectangle (Rectangle (0, 0, getWidth(), getHeight()),
                                                                   document->getComponentLayout()));

                        anyCompsMoved = anyCompsMoved || (comp->getBounds() != newBounds);
                        comp->setBounds (newBounds);
                    }
                }

                // repeat this loop until they've all stopped shuffling (might require a few
                // loops for all the relative positioned comps to settle down)
                if (! anyCompsMoved)
                    break;
            }
        }
    }

private:
    ComponentDocument* const document;
    const bool alwaysFillBackground;
};

Component* ComponentDocument::createTestComponent (const bool alwaysFillBackground)
{
    return new NormalTestComponent (this, alwaysFillBackground);
}

//==============================================================================
void ComponentDocument::fillInGeneratedCode (GeneratedCode& code) const
{
    JucerDocument::fillInGeneratedCode (code);
}
