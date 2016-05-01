/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../../jucer_Headers.h"
#include "jucer_ComponentDocument.h"


//==============================================================================
ComponentDocument::ComponentDocument (SourceCodeDocument* c)
    : JucerDocument (c)
{
    components = new ComponentLayout();
    components->setDocument (this);

    backgroundGraphics = new PaintRoutine();
    backgroundGraphics->setDocument (this);
}

ComponentDocument::~ComponentDocument()
{
}

//==============================================================================
String ComponentDocument::getTypeName() const
{
    return "Component";
}

JucerDocument* ComponentDocument::createCopy()
{
    ComponentDocument* newOne = new ComponentDocument (cpp);

    newOne->resources = resources;

    ScopedPointer<XmlElement> xml (createXml());
    newOne->loadFromXml (*xml);

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
    NormalTestComponent (ComponentDocument* const doc, const bool fillBackground)
        : document (doc),
          alwaysFillBackground (fillBackground)
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

    void paint (Graphics& g) override
    {
        document->getPaintRoutine (0)->fillWithBackground (g, alwaysFillBackground);
        document->getPaintRoutine (0)->drawElements (g, getLocalBounds());
    }

    void resized() override
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

                    if (ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*comp))
                    {
                        const Rectangle<int> newBounds (type->getComponentPosition (comp)
                                                        .getRectangle (getLocalBounds(),
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

void ComponentDocument::fillInGeneratedCode (GeneratedCode& code) const
{
    JucerDocument::fillInGeneratedCode (code);
}
