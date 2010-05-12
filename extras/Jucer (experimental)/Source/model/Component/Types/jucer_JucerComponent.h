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
  ADD_TO_LIST (JucerComponentHandler);
#else

#include "../jucer_ComponentDocument.h"

//==============================================================================
class JucerComponent : public Component
{
public:
    JucerComponent()
    {
        addAndMakeVisible (comp = new ComponentDocument::TestComponent (0, File::nonexistent));
    }

    void resized()
    {
        comp->setBounds (getLocalBounds());
    }

    void setJucerComp (ComponentDocument& document, const File& cppFile)
    {
        addAndMakeVisible (comp = new ComponentDocument::TestComponent (document.getProject(), cppFile));
        resized();
    }

private:
    ScopedPointer<ComponentDocument::TestComponent> comp;
};

//==============================================================================
class JucerComponentHandler  : public ComponentTypeHelper<JucerComponent>
{
public:
    JucerComponentHandler() : ComponentTypeHelper<JucerComponent> ("Jucer Component", "Component", "JUCERCOMPONENT", "jucerComp")  {}
    ~JucerComponentHandler()  {}

    //==============================================================================
    Component* createComponent()                { return new JucerComponent(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 150, 150); }

    void initialiseNew (ComponentTypeInstance& item)
    {
    }

    void update (ComponentTypeInstance& item, JucerComponent* comp)
    {
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
    }

    const String getClassName (ComponentTypeInstance& item) const
    {
        return "xxx";
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode << item.createConstructorStatement (String::empty);
    }
};

#endif
