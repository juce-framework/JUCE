/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

//==============================================================================
static String getWidthLimitedStringFromVarArray (const var& varArray) noexcept
{
    if (! varArray.isArray())
        return {};

    int numLines = 1;
    const int lineWidth = 100;
    const String indent ("                    ");

    String str;
    if (auto* arr = varArray.getArray())
    {
        for (auto& v : *arr)
        {
            if ((str.length() + v.toString().length()) > (lineWidth * numLines))
            {
                str += newLine;
                str += indent;

                ++numLines;
            }

            str += v.toString() + (arr->indexOf (v) != arr->size() - 1 ? ", " : "");
        }
    }

    return str;
}

//==============================================================================
class PIPCreatorWindowComponent    : public Component,
                                     private ValueTree::Listener
{
public:
    PIPCreatorWindowComponent()
    {
        lf.reset (new PIPCreatorLookAndFeel());
        setLookAndFeel (lf.get());

        addAndMakeVisible (propertyViewport);
        propertyViewport.setViewedComponent (&propertyGroup, false);
        buildProps();

        addAndMakeVisible (createButton);
        createButton.onClick = [this]
        {
            FileChooser fc ("Save PIP File",
                            File::getSpecialLocation (File::SpecialLocationType::userDesktopDirectory)
                                 .getChildFile (nameValue.get().toString() + ".h"));

            fc.browseForFileToSave (true);

            createPIPFile (fc.getResult());
        };

        pipTree.addListener (this);
    }

    ~PIPCreatorWindowComponent()
    {
        setLookAndFeel (nullptr);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        createButton.setBounds (bounds.removeFromBottom (50).reduced (100, 10));

        propertyGroup.updateSize (0, 0, getWidth() - propertyViewport.getScrollBarThickness());
        propertyViewport.setBounds (bounds);
    }

private:
    //==============================================================================
    struct PIPCreatorLookAndFeel    : public ProjucerLookAndFeel
    {
        PIPCreatorLookAndFeel()    {}

        Rectangle<int> getPropertyComponentContentPosition (PropertyComponent& component)
        {
            auto textW = jmin (200, component.getWidth() / 3);
            return { textW, 0, component.getWidth() - textW, component.getHeight() - 1 };
        }
    };

    //==============================================================================
    void buildProps()
    {
        PropertyListBuilder builder;

        builder.add (new TextPropertyComponent (nameValue, "Name", 256, false),
                     "The name of your JUCE project.");
        builder.add (new TextPropertyComponent (versionValue, "Version", 16, false),
                     "This will be used for the \"Project Version\" field in the Projucer.");
        builder.add (new TextPropertyComponent (vendorValue, "Vendor", 2048, false),
                     "This will be used for the \"Company Name\" field in the Projucer.");
        builder.add (new TextPropertyComponent (websiteValue, "Website", 2048, false),
                     "This will be used for the \"Company Website\" field in the Projucer");
        builder.add (new TextPropertyComponent (descriptionValue, "Description", 2048, true),
                     "A short description of your JUCE project.");

        {
            Array<var> moduleVars;
            for (auto& m : getJUCEModules())
                moduleVars.add (m);

            builder.add (new MultiChoicePropertyComponent (dependenciesValue, "Dependencies",
                                                           getJUCEModules(), moduleVars),
                         "The JUCE modules that should be added to your project.");
        }

        {
            Array<var> exporterVars;
            for (auto& e : ProjectExporter::getExporterValueTreeNames())
                exporterVars.add (e.toLowerCase());

            builder.add (new MultiChoicePropertyComponent (exportersValue, "Exporters",
                                                           ProjectExporter::getExporterNames(), exporterVars),
                         "The exporters that should be added to your project.");
        }

        builder.add (new TextPropertyComponent (moduleFlagsValue, "Module Flags", 2048, true),
                     "Use this to set one, or many, of the JUCE module flags");
        builder.add (new TextPropertyComponent (definesValue, "Defines", 2048, true),
                     "This sets some global preprocessor definitions for your project. Used to populate the \"Preprocessor Definitions\" field in the Projucer.");
        builder.add (new ChoicePropertyComponent (typeValue, "Type",
                                                  { "Component", "Plugin",         "Console Application" },
                                                  { "Component", "AudioProcessor", "Console" }),
                     "The project type.");

        builder.add (new TextPropertyComponent (mainClassValue, "Main Class", 2048, false),
                     "The name of the main class that should be instantiated. "
                     "There can only be one main class and it must have a default constructor. "
                     "Depending on the type, this may need to inherit from a specific JUCE class");

        builder.add (new ChoicePropertyComponent (useLocalCopyValue, "Use Local Copy"),
                     "Enable this to specify that the PIP file should be copied to the generated project directory instead of just referred to.");

        propertyGroup.setProperties (builder);
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier& id) override
    {
        if (id == Ids::type)
        {
            auto type = typeValue.get().toString();

            if (type == "Component")
            {
                nameValue.setDefault ("MyComponentPIP");
                dependenciesValue.setDefault (getModulesRequiredForComponent());
                mainClassValue.setDefault ("MyComponent");
            }
            else if (type == "AudioProcessor")
            {
                nameValue.setDefault ("MyPluginPIP");
                dependenciesValue.setDefault (getModulesRequiredForAudioProcessor());
                mainClassValue.setDefault ("MyPlugin");
            }
            else if (type == "Console")
            {
                nameValue.setDefault ("MyConsolePIP");
                dependenciesValue.setDefault (getModulesRequiredForConsole());
                mainClassValue.setDefault ({});
            }

            MessageManager::callAsync ([this]
            {
                buildProps();
                resized();
            });
        }
    }

    void valueTreeChildAdded (ValueTree&, ValueTree&) override           {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override    {}
    void valueTreeChildOrderChanged (ValueTree&, int, int) override      {}
    void valueTreeParentChanged (ValueTree&) override                    {}

    //==============================================================================
    String getFormattedMetadataString() const noexcept
    {
        StringArray metadata;

        {
            StringArray section;

            if (nameValue.get().toString().isNotEmpty())          section.add ("  name:             " + nameValue.get().toString());
            if (versionValue.get().toString().isNotEmpty())       section.add ("  version:          " + versionValue.get().toString());
            if (vendorValue.get().toString().isNotEmpty())        section.add ("  vendor:           " + vendorValue.get().toString());
            if (websiteValue.get().toString().isNotEmpty())       section.add ("  website:          " + websiteValue.get().toString());
            if (descriptionValue.get().toString().isNotEmpty())   section.add ("  description:      " + descriptionValue.get().toString());

            if (! section.isEmpty())
                metadata.add (section.joinIntoString (getPreferredLinefeed()));
        }

        {
            StringArray section;

            auto dependenciesString = getWidthLimitedStringFromVarArray (dependenciesValue.get());
            if (dependenciesString.isNotEmpty())                  section.add ("  dependencies:     " + dependenciesString);

            auto exportersString = getWidthLimitedStringFromVarArray (exportersValue.get());
            if (exportersString.isNotEmpty())                     section.add ("  exporters:        " + exportersString);

            if (! section.isEmpty())
                metadata.add (section.joinIntoString (getPreferredLinefeed()));
        }

        {
            StringArray section;

            if (moduleFlagsValue.get().toString().isNotEmpty())   section.add ("  moduleFlags:      " + moduleFlagsValue.get().toString());
            if (definesValue.get().toString().isNotEmpty())       section.add ("  defines:          " + definesValue.get().toString());

            if (! section.isEmpty())
                metadata.add (section.joinIntoString (getPreferredLinefeed()));
        }

        {
            StringArray section;

            if (typeValue.get().toString().isNotEmpty())          section.add ("  type:             " + typeValue.get().toString());
            if (mainClassValue.get().toString().isNotEmpty())     section.add ("  mainClass:        " + mainClassValue.get().toString());

            if (! section.isEmpty())
                metadata.add (section.joinIntoString (getPreferredLinefeed()));
        }

        {
            StringArray section;

            if (useLocalCopyValue.get())                          section.add ("  useLocalCopy:     " + useLocalCopyValue.get().toString());

            if (! section.isEmpty())
                metadata.add (section.joinIntoString (getPreferredLinefeed()));
        }

        return metadata.joinIntoString (String (getPreferredLinefeed()) + getPreferredLinefeed());
    }

    void createPIPFile (File fileToSave)
    {
        String fileTemplate (BinaryData::jucer_PIPTemplate_h);
        fileTemplate = fileTemplate.replace ("%%pip_metadata%%", getFormattedMetadataString());

        auto type = typeValue.get().toString();

        if (type == "Component")
        {
            String componentCode (BinaryData::jucer_ContentCompSimpleTemplate_h);
            componentCode = componentCode.substring (componentCode.indexOf ("class %%content_component_class%%"))
                                         .replace ("%%content_component_class%%", mainClassValue.get().toString());

            fileTemplate = fileTemplate.replace ("%%pip_code%%", componentCode);
        }
        else if (type == "AudioProcessor")
        {
            String audioProcessorCode (BinaryData::jucer_PIPAudioProcessorTemplate_h);
            audioProcessorCode = audioProcessorCode.replace ("%%class_name%%", mainClassValue.get().toString())
                                                   .replace ("%%name%%", nameValue.get().toString());

            fileTemplate = fileTemplate.replace ("%%pip_code%%", audioProcessorCode);
        }
        else if (type == "Console")
        {
            String consoleCode (BinaryData::jucer_MainConsoleAppTemplate_cpp);
            consoleCode = consoleCode.substring (consoleCode.indexOf ("int main (int argc, char* argv[])"));

            fileTemplate = fileTemplate.replace ("%%pip_code%%", consoleCode);
        }

        if (fileToSave.create())
            fileToSave.replaceWithText (fileTemplate);
    }

    //==============================================================================
    ValueTree pipTree  { "PIPSettings" };
    ValueWithDefault nameValue          { pipTree, Ids::name,          nullptr, "MyComponentPIP" },
                     versionValue       { pipTree, Ids::version,       nullptr },
                     vendorValue        { pipTree, Ids::vendor,        nullptr },
                     websiteValue       { pipTree, Ids::website,       nullptr },
                     descriptionValue   { pipTree, Ids::description,   nullptr },
                     dependenciesValue  { pipTree, Ids::dependencies_, nullptr, getModulesRequiredForComponent(), "," },
                     exportersValue     { pipTree, Ids::exporters,     nullptr,
                                          StringArray (ProjectExporter::getValueTreeNameForExporter (ProjectExporter::getCurrentPlatformExporterName()).toLowerCase()), "," },
                     moduleFlagsValue   { pipTree, Ids::moduleFlags,   nullptr, "JUCE_STRICT_REFCOUNTEDPOINTER=1" },
                     definesValue       { pipTree, Ids::defines,       nullptr },
                     typeValue          { pipTree, Ids::type,          nullptr, "Component" },
                     mainClassValue     { pipTree, Ids::mainClass,     nullptr, "MyComponent" },
                     useLocalCopyValue  { pipTree, Ids::useLocalCopy,  nullptr, false };

    std::unique_ptr<LookAndFeel> lf;

    Viewport propertyViewport;
    PropertyGroupComponent propertyGroup  { "PIP Creator", { getIcons().juceLogo, Colours::transparentBlack } };

    TextButton createButton  { "Create PIP" };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PIPCreatorWindowComponent)
};
