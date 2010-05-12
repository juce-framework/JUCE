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

#ifdef ADD_TO_LIST
  ADD_TO_LIST (ViewportHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class ViewportHandler  : public ComponentTypeHelper<Viewport>
{
public:
    ViewportHandler() : ComponentTypeHelper<Viewport> ("Viewport", "Viewport", "VIEWPORT", "viewport")  {}
    ~ViewportHandler()  {}

    class DemoContentComponent  : public Component
    {
    public:
        DemoContentComponent() { setSize (1000, 1000); }
        ~DemoContentComponent() {}

        void paint (Graphics& g)
        {
            g.fillCheckerBoard (0, 0, getWidth(), getHeight(), 40, 40,
                                Colours::grey.withAlpha (0.7f),
                                Colours::white.withAlpha (0.7f));
        }
    };

    Component* createComponent()
    {
        Viewport* v = new Viewport();
        v->setViewedComponent (new DemoContentComponent());
        return v;
    }

    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 300, 200); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set ("scrollBarV", true);
        item.set ("scrollBarH", true);
        item.set ("scrollbarWidth", 18);
    }

    void update (ComponentTypeInstance& item, Viewport* comp)
    {
        comp->setScrollBarsShown (item ["scrollBarV"], item ["scrollBarH"]);
        comp->setScrollBarThickness (item ["scrollbarWidth"]);
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        props.add (new BooleanPropertyComponent (item.getValue ("scrollBarV"), "Scrollbar V", "Vertical scrollbar shown"));
        props.add (new BooleanPropertyComponent (item.getValue ("scrollBarH"), "Scrollbar H", "Horizontal scrollbar shown"));
        props.add (new SliderPropertyComponent (item.getValue ("scrollbarWidth"), "Scrollbar Thickness", 3, 40, 1));
        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode << item.createConstructorStatement (CodeFormatting::stringLiteral (item.getComponentName()));
    }
};

#endif
