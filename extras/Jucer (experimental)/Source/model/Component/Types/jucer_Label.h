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
  ADD_TO_LIST (LabelHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class LabelHandler  : public ComponentTypeHelper<Label>
{
public:
    LabelHandler() : ComponentTypeHelper<Label> ("Label", "Label", "LABEL", "label")
    {
        addEditableColour (Label::backgroundColourId, "Background", "backgroundColour");
        addEditableColour (Label::textColourId, "Text Colour", "textColour");
        addEditableColour (Label::outlineColourId, "Outline Colour", "outlineColour");
    }

    ~LabelHandler()  {}

    Component* createComponent()                { return new Label(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set (Ids::text, "New Label");
        item.set (Ids::font, Font (14.0f).toString());
        item.set (Ids::editMode, 1);
        item.set (Ids::justification, (int) Justification::centredLeft);
    }

    void update (ComponentTypeInstance& item, Label* comp)
    {
        comp->setText (item [Ids::text].toString(), false);
        comp->setFont (Font::fromString (item [Ids::font]));
        int editMode = (int) item [Ids::editMode];
        comp->setEditable (editMode == 2, editMode == 3, false);
        comp->setJustificationType ((int) item [Ids::justification]);
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue (Ids::text), "Text", 16384, true));
        props.getLast()->setTooltip ("The label's text.");

        item.addJustificationProperty (props, "Layout", item.getValue ("justification"), false);

        const char* const editModes[] = { "Read-only", "Edit on Single-Click", "Edit on Double-Click", 0 };
        props.add (new ChoicePropertyComponent (item.getValue (Ids::editMode), "Edit Mode", StringArray (editModes)));

        item.addFontProperties (props, Ids::font);

        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        int editMode = (int) item [Ids::editMode];

        code.constructorCode
            << item.createConstructorStatement (String::empty)
            << item.getMemberName() << "->setText (" << CodeHelpers::stringLiteral (item [Ids::text]) << ");" << newLine
            << item.getMemberName() << "->setFont (" << CodeHelpers::fontToCode (Font::fromString (item [Ids::font])) << ");" << newLine
            << item.getMemberName() << "->setEditable (" << CodeHelpers::boolLiteral (editMode == 2)
                                    << ", " << CodeHelpers::boolLiteral (editMode == 3) << ", false);" << newLine;

        Justification justification ((int) item ["textJustification"]);
        if (justification.getFlags() != 0)
            code.constructorCode << item.getMemberName() << "->setJustificationType ("
                                 << CodeHelpers::justificationToCode (justification) << ");" << newLine;
    }
};

#endif
