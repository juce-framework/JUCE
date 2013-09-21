/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

class ModulesItem   : public ConfigTreeItemBase
{
public:
    ModulesItem (Project& p)  : project (p) {}

    bool isModulesList() const override     { return true; }
    bool canBeSelected() const override     { return true; }
    bool mightContainSubItems() override    { return false; }
    String getUniqueName() const override   { return "modules"; }
    String getRenamingName() const override { return getDisplayName(); }
    String getDisplayName() const override  { return "Modules"; }
    void setName (const String&) override   {}
    bool isMissing() override               { return false; }
    Icon getIcon() const override           { return Icon (getIcons().graph, getContrastingColour (Colours::red, 0.5f)); }
    void showDocument() override            { showSettingsPage (new SettingsComp (project)); }

private:
    Project& project;

    class SettingsComp  : public Component
    {
    public:
        SettingsComp (Project& p)   : project (p)
        {
            addAndMakeVisible (&group);

            PropertyListBuilder props;
            props.add (new ModulesPanel (project));
            group.setProperties (props);
            group.setName ("Modules");

            parentSizeChanged();
        }

        void parentSizeChanged() override
        {
            updateSize (*this, group);
        }

    private:
        Project& project;
        var lastProjectType;
        PropertyGroupComponent group;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulesItem)
};
