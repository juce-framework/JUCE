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
  ADD_TO_LIST (GenericComponentHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class GenericComponentHandler  : public ComponentTypeHelper<Component>
{
public:
    GenericComponentHandler() : ComponentTypeHelper<Component> ("Generic Component", "Component", "COMPONENT", "component")  {}
    ~GenericComponentHandler()  {}

    //==============================================================================
    class PlaceholderComp : public Component
    {
    public:
        PlaceholderComp()
        {
        }

        void setDetails (const String& memberName, const String& className)
        {
            const String name (memberName + " (" + className + ")");

            if (name != getName())
            {
                setName (name);
                repaint();
            }
        }

        void paint (Graphics& g)
        {
            drawComponentPlaceholder (g, getWidth(), getHeight(), getName());
        }
    };

    //==============================================================================
    Component* createComponent()                { return new PlaceholderComp(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set ("class", "Component");
    }

    void update (ComponentTypeInstance& item, Component* comp)
    {
        static_cast<PlaceholderComp*> (comp)->setDetails (item [ComponentDocument::memberNameProperty],
                                                          item ["class"]);
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue ("class"), "Class", 256, false));
        props.getLast()->setTooltip ("The class that this component is an instance of.");
    }

    const String getClassName (ComponentTypeInstance& item) const
    {
        return item ["class"];
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode << item.createConstructorStatement (String::empty);
    }
};

#endif
