/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../ProjectSaving/jucer_ProjectExporter.h"
#include "../../Utility/UI/PropertyComponents/jucer_FilePathPropertyComponent.h"
#include "../../Utility/Helpers/jucer_ValueTreePropertyWithDefaultWrapper.h"

#include "jucer_NewProjectWizard.h"

//==============================================================================
class ItemHeader final : public Component
{
public:
    ItemHeader (StringRef name, StringRef description, const char* iconSvgData)
        : nameLabel ({}, name),
          descriptionLabel ({}, description),
          icon (makeIcon (iconSvgData))
    {
        addAndMakeVisible (nameLabel);
        nameLabel.setFont (FontOptions { 18.0f });
        nameLabel.setMinimumHorizontalScale (1.0f);

        addAndMakeVisible (descriptionLabel);
        descriptionLabel.setMinimumHorizontalScale (1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        auto topSlice = bounds.removeFromTop (50);
        iconBounds = topSlice.removeFromRight (75);
        nameLabel.setBounds (topSlice);

        bounds.removeFromTop (10);
        descriptionLabel.setBounds (bounds.removeFromTop (50));
        bounds.removeFromTop (20);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));

        if (icon != nullptr)
            icon->drawWithin (g, iconBounds.toFloat(), RectanglePlacement::centred, 1.0f);
    }

private:
    static std::unique_ptr<Drawable> makeIcon (const char* iconSvgData)
    {
        if (auto svg = XmlDocument::parse (iconSvgData))
            return Drawable::createFromSVG (*svg);

        return {};
    }

    Label nameLabel, descriptionLabel;

    Rectangle<int> iconBounds;
    std::unique_ptr<Drawable> icon;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemHeader)
};

//==============================================================================
class TemplateComponent final : public Component
{
public:
    TemplateComponent (const NewProjectTemplates::ProjectTemplate& temp,
                       std::function<void (std::unique_ptr<Project>)>&& createdCallback)
        : projectTemplate (temp),
          projectCreatedCallback (std::move (createdCallback)),
          header (projectTemplate.displayName, projectTemplate.description, projectTemplate.icon)
    {
        createProjectButton.onClick = [this]
        {
            chooser = std::make_unique<FileChooser> ("Save Project", NewProjectWizard::getLastWizardFolder());
            auto browserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

            chooser->launchAsync (browserFlags, [this] (const FileChooser& fc)
            {
                auto dir = fc.getResult();

                if (dir == File{})
                    return;

                SafePointer<TemplateComponent> safeThis { this };
                messageBox = NewProjectWizard::createNewProject (projectTemplate,
                                                                 dir.getChildFile (projectNameValue.get().toString()),
                                                                 projectNameValue.get(),
                                                                 modulesValue.get(),
                                                                 exportersValue.get(),
                                                                 fileOptionsValue.get(),
                                                                 modulePathValue.getCurrentValue(),
                                                                 modulePathValue.getWrappedValueTreePropertyWithDefault().isUsingDefault(),
                                                                 [safeThis, dir] (ScopedMessageBox mb, std::unique_ptr<Project> project)
                {
                    if (safeThis == nullptr)
                        return;

                    safeThis->messageBox = std::move (mb);
                    safeThis->projectCreatedCallback (std::move (project));
                    getAppSettings().lastWizardFolder = dir;
                });
            });
        };

        addAndMakeVisible (createProjectButton);
        addAndMakeVisible (header);

        modulePathValue.init ({ settingsTree, Ids::defaultJuceModulePath, nullptr },
                              getAppSettings().getStoredPath (Ids::defaultJuceModulePath, TargetOS::getThisOS()),
                              TargetOS::getThisOS());

        panel.addProperties (buildPropertyList(), 2);
        addAndMakeVisible (panel);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (10);

        header.setBounds (bounds.removeFromTop (150));
        createProjectButton.setBounds (bounds.removeFromBottom (30).removeFromRight (150));
        bounds.removeFromBottom (5);

        panel.setBounds (bounds);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

private:
    NewProjectTemplates::ProjectTemplate projectTemplate;

    std::unique_ptr<FileChooser> chooser;
    std::function<void (std::unique_ptr<Project>)> projectCreatedCallback;

    ItemHeader header;
    TextButton createProjectButton { "Create Project..." };

    ValueTree settingsTree { "NewProjectSettings" };

    ValueTreePropertyWithDefault projectNameValue { settingsTree, Ids::name,          nullptr, "NewProject" },
                                 modulesValue     { settingsTree, Ids::dependencies_, nullptr, projectTemplate.requiredModules, "," },
                                 exportersValue   { settingsTree, Ids::exporters,     nullptr, StringArray (ProjectExporter::getCurrentPlatformExporterTypeInfo().identifier.toString()), "," },
                                 fileOptionsValue { settingsTree, Ids::file,          nullptr, NewProjectTemplates::getVarForFileOption (projectTemplate.defaultFileOption) };

    ValueTreePropertyWithDefaultWrapper modulePathValue;

    PropertyPanel panel;

    //==============================================================================
    PropertyComponent* createProjectNamePropertyComponent()
    {
        return new TextPropertyComponent (projectNameValue, "Project Name", 1024, false);
    }

    PropertyComponent* createModulesPropertyComponent()
    {
        Array<var> moduleVars;
        var requiredModules;

        for (auto& m : getJUCEModules())
        {
            moduleVars.add (m);

            if (projectTemplate.requiredModules.contains (m))
                requiredModules.append (m);
        }

        modulesValue = requiredModules;

        return new MultiChoicePropertyComponent (modulesValue, "Modules",
                                                 getJUCEModules(), moduleVars);
    }

    PropertyComponent* createModulePathPropertyComponent()
    {
        return new FilePathPropertyComponent (modulePathValue.getWrappedValueTreePropertyWithDefault(), "Path to Modules", true);
    }

    PropertyComponent* createExportersPropertyValue()
    {
        Array<var> exporterVars;
        StringArray exporterNames;

        for (auto& exporterTypeInfo : ProjectExporter::getExporterTypeInfos())
        {
            exporterVars.add (exporterTypeInfo.identifier.toString());
            exporterNames.add (exporterTypeInfo.displayName);
        }

        return new MultiChoicePropertyComponent (exportersValue, "Exporters", exporterNames, exporterVars);
    }

    PropertyComponent* createFileCreationOptionsPropertyComponent()
    {
        Array<var> optionVars;
        StringArray optionStrings;

        for (auto& opt : projectTemplate.fileOptionsAndFiles)
        {
            optionVars.add    (NewProjectTemplates::getVarForFileOption (opt.first));
            optionStrings.add (NewProjectTemplates::getStringForFileOption (opt.first));
        }

        return new ChoicePropertyComponent (fileOptionsValue, "File Creation Options", optionStrings, optionVars);
    }

    Array<PropertyComponent*> buildPropertyList()
    {
        PropertyListBuilder builder;

        builder.add (createProjectNamePropertyComponent());
        builder.add (createModulesPropertyComponent());
        builder.add (createModulePathPropertyComponent());
        builder.add (createExportersPropertyValue());

        if (! projectTemplate.fileOptionsAndFiles.empty())
            builder.add (createFileCreationOptionsPropertyComponent());

        return builder.components;
    }

    ScopedMessageBox messageBox;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateComponent)
};

//==============================================================================
class ExampleComponent final : public Component
{
public:
    ExampleComponent (const File& f, std::function<void (const File&)> selectedCallback)
        : exampleFile (f),
          metadata (parseJUCEHeaderMetadata (exampleFile)),
          exampleSelectedCallback (std::move (selectedCallback)),
          header (metadata[Ids::name].toString(), metadata[Ids::description].toString(), BinaryData::background_logo_svg),
          codeViewer (doc, &cppTokeniser)
    {
        setTitle (exampleFile.getFileName());
        setFocusContainerType (FocusContainerType::focusContainer);

        addAndMakeVisible (header);

        openExampleButton.onClick = [this] { exampleSelectedCallback (exampleFile); };
        addAndMakeVisible (openExampleButton);

        setupCodeViewer();
        addAndMakeVisible (codeViewer);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (10);

        header.setBounds (bounds.removeFromTop (125));
        openExampleButton.setBounds (bounds.removeFromBottom (30).removeFromRight (150));
        codeViewer.setBounds (bounds);
    }

private:
    void setupCodeViewer()
    {
        auto fileString = exampleFile.loadFileAsString();

        doc.replaceAllContent (fileString);

        codeViewer.setScrollbarThickness (6);
        codeViewer.setReadOnly (true);
        codeViewer.setTitle ("Code");
        getAppSettings().appearance.applyToCodeEditor (codeViewer);

        codeViewer.scrollToLine (findBestLineToScrollToForClass (StringArray::fromLines (fileString),
                                                                 metadata[Ids::name].toString(),
                                                                 metadata[Ids::type] == "AudioProcessor"));
    }

    //==============================================================================
    File exampleFile;
    var metadata;

    std::function<void (const File&)> exampleSelectedCallback;

    ItemHeader header;

    CPlusPlusCodeTokeniser cppTokeniser;
    CodeDocument doc;
    CodeEditorComponent codeViewer;

    TextButton openExampleButton { "Open Example..." };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExampleComponent)
};
