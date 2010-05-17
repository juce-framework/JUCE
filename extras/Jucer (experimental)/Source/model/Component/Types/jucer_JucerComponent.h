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
#include "../../../ui/jucer_OpenDocumentManager.h"
#include "../../../ui/Project Editor/jucer_ProjectContentComponent.h"


//==============================================================================
class JucerComponent : public Component
{
public:
    JucerComponent()
    {
    }

    void paint (Graphics& g)
    {
        if (comp == 0)
            drawComponentPlaceholder (g, getWidth(), getHeight(), "(Not a valid Jucer component)");
    }

    void resized()
    {
        if (comp != 0)
            comp->setBounds (getLocalBounds());
    }

    void setJucerComp (ComponentTypeInstance& item, const String& newProjectId)
    {
        if (projectId != newProjectId)
        {
            projectId = newProjectId;
            comp = 0;

            if (newProjectId.isNotEmpty())
            {
                const File file (getDocumentFile (item, projectId));

                if (file.exists())
                {
                    OpenDocumentManager::Document* doc = OpenDocumentManager::getInstance()->getDocumentForFile (item.getDocument().getProject(), file);

                    if (doc != 0)
                    {
                        addAndMakeVisible (comp = doc->createViewer());
                        resized();
                    }
                }
            }
        }
    }

    static const File getDocumentFile (ComponentTypeInstance& item, const String projectItemId)
    {
        if (projectItemId.isNotEmpty())
        {
            Project* project = item.getDocument().getProject();

            if (project != 0)
            {
                Project::Item item (project->getMainGroup().findItemWithID (projectItemId));

                if (item.isValid())
                    return item.getFile();
            }
        }

        return File::nonexistent;
    }

private:
    String projectId;
    ScopedPointer<Component> comp;
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
        comp->setJucerComp (item, item [Ids::source].toString());
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        StringArray names;
        Array<var> ids;

        names.add ("<none>");
        ids.add (var::null);
        names.add (String::empty);
        ids.add (var::null);

        {
            Array <Project::Item> comps;
            findAllComponentDocumentsInProject (item, comps);

            for (int i = 0; i < comps.size(); ++i)
            {
                if (comps.getReference(i).getFile() != item.getDocument().getCppFile())
                {
                    names.add (comps.getReference(i).getName().toString());
                    ids.add (comps.getReference(i).getID());
                }
            }
        }

        props.add (new ChoicePropertyComponent (item.getValue (Ids::source), "Source", names, ids));
        item.addFocusOrderProperty (props);
    }

    void itemDoubleClicked (const MouseEvent& e, ComponentTypeInstance& item)
    {
        ProjectContentComponent* pcc = e.originalComponent->findParentComponentOfClass ((ProjectContentComponent*) 0);

        if (pcc != 0)
            pcc->showEditorForFile (JucerComponent::getDocumentFile (item, item [Ids::source].toString()));
    }

    const String getClassName (ComponentTypeInstance& item) const
    {
        return "xxx";
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode << item.createConstructorStatement (String::empty);
    }

    //==============================================================================
    static void findAllComponentDocumentsInProject (ComponentTypeInstance& item, Array <Project::Item>& comps)
    {
        Project* project = item.getDocument().getProject();

        if (project != 0)
            findAllComponentDocumentsInProjectItem (project->getMainGroup(), comps);
    }

    static void findAllComponentDocumentsInProjectItem (const Project::Item& item, Array <Project::Item>& comps)
    {
        if (item.isGroup())
        {
            const int num = item.getNumChildren();
            for (int i = 0; i < num; ++i)
                findAllComponentDocumentsInProjectItem (item.getChild (i), comps);
        }
        else if (item.isFile())
        {
            if (ComponentDocument::isComponentFile (item.getFile()))
                comps.add (item);
        }
    }
};

#endif
