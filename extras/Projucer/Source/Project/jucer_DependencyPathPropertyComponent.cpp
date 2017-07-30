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

#include "../jucer_Headers.h"
#include "jucer_DependencyPathPropertyComponent.h"

//==============================================================================
DependencyPathValueSource::DependencyPathValueSource (const Value& projectSettingsPath,
                                                      Identifier globalSettingsKey,
                                                      DependencyPathOS osThisSettingAppliesTo)
  : projectSettingsValue (projectSettingsPath),
    globalKey (globalSettingsKey),
    os (osThisSettingAppliesTo),
    globalSettingsValue (getAppSettings().getStoredPath (globalKey)),
    fallbackValue (getAppSettings().getFallbackPathForOS (globalKey, os))
{
    globalSettingsValue.addListener (this);
    fallbackValue.addListener (this);
}

bool DependencyPathValueSource::isValidPath (const File& relativeTo) const
{
    // if we are on another OS than the one which this path setting is for,
    // we have no way of knowing whether the path is valid - so just assume it is:
    if (! appliesToThisOS())
        return true;

    return getAppSettings().isGlobalPathValid (relativeTo, globalKey, getValue().toString());
}

bool DependencyPathValueSource::isValidPath() const
{
    return isValidPath (File::getCurrentWorkingDirectory());
}

//==============================================================================
DependencyPathPropertyComponent::DependencyPathPropertyComponent (const File& pathRelativeToUse,
                                                                  const Value& value,
                                                                  const String& propertyName)
try : TextPropertyComponent (propertyName, 1024, false),
      pathRelativeTo (pathRelativeToUse),
      pathValue (value),
      pathValueSource (dynamic_cast<DependencyPathValueSource&> (pathValue.getValueSource()))
{
    bool initialValueIsEmpty = ! pathValueSource.isUsingProjectSettings();

    getValue().referTo (pathValue);

    // the following step is necessary because the above referTo() has internally called setValue(),
    // which has set the project value to whatever is displayed in the label (this may be the
    // global/fallback value). In this case we have to reset the project value to blank:
    if (initialValueIsEmpty)
        getValue().setValue (String());

    getValue().addListener (this);

    if (Label* label = dynamic_cast<Label*> (getChildComponent (0)))
        label->addListener (this);
    else
        jassertfalse;

    lookAndFeelChanged();
}
catch (const std::bad_cast&)
{
    // a DependencyPathPropertyComponent must be initialised with a Value
    // that is referring to a DependencyPathValueSource!
    jassertfalse;
    throw;
}

void DependencyPathPropertyComponent::valueChanged (Value& value)
{
    // this callback handles the update of this setting in case
    // the user changed the global preferences.
    if (value.refersToSameSourceAs (pathValue) && pathValueSource.isUsingGlobalSettings())
        textWasEdited();
}

void DependencyPathPropertyComponent::textWasEdited()
{
    setColour (textColourId, getTextColourToDisplay());
    TextPropertyComponent::textWasEdited();
}

Colour DependencyPathPropertyComponent::getTextColourToDisplay() const
{
    if (! pathValueSource.isUsingProjectSettings())
        return pathValueSource.isValidPath (pathRelativeTo) ? findColour (widgetTextColourId).withMultipliedAlpha (0.5f)
                                                            : Colours::red.withMultipliedAlpha (0.5f);

    return pathValueSource.isValidPath (pathRelativeTo) ? findColour (widgetTextColourId)
                                                        : Colours::red;
}

void DependencyPathPropertyComponent::labelTextChanged (Label*)
{
}

void DependencyPathPropertyComponent::editorShown (Label* /*label*/, TextEditor& editor)
{
    if (! pathValueSource.isUsingProjectSettings())
        editor.setText (String(), dontSendNotification);
}

void DependencyPathPropertyComponent::editorHidden (Label*, TextEditor&)
{
}

void DependencyPathPropertyComponent::lookAndFeelChanged()
{
    textWasEdited();
}

//==============================================================================
DependencyFilePathPropertyComponent::DependencyFilePathPropertyComponent (Value& value,
                                                                          const String& propertyDescription,
                                                                          bool isDir,
                                                                          const String& wc,
                                                                          const File& rootToUseForRelativePaths)
try : TextPropertyComponent (propertyDescription, 1024, false),
      pathRelativeTo (rootToUseForRelativePaths),
      pathValue (value),
      pathValueSource (dynamic_cast<DependencyPathValueSource&> (pathValue.getValueSource())),
      browseButton ("..."),
      isDirectory (isDir),
      wildcards (wc)
{
    auto initialValueIsEmpty = ! pathValueSource.isUsingProjectSettings();

    getValue().referTo (pathValue);

    if (initialValueIsEmpty)
        getValue().setValue (String());

    getValue().addListener (this);

    if (auto* label = dynamic_cast<Label*> (getChildComponent (0)))
        label->addListener (this);
    else
        jassertfalse;

    setInterestedInFileDrag (false);

    addAndMakeVisible (browseButton);
    browseButton.addListener (this);

    lookAndFeelChanged();
}
catch (const std::bad_cast&)
{
    // a DependencyPathPropertyComponent must be initialised with a Value
    // that is referring to a DependencyPathValueSource!
    jassertfalse;
    throw;
}

void DependencyFilePathPropertyComponent::resized()
{
    auto bounds = getLookAndFeel().getPropertyComponentContentPosition (*this);

    browseButton.setBounds (bounds.removeFromRight (30));
    getChildComponent (0)->setBounds (bounds);
}

void DependencyFilePathPropertyComponent::paintOverChildren (Graphics& g)
{
    if (highlightForDragAndDrop)
    {
        g.setColour (findColour (defaultHighlightColourId).withAlpha (0.5f));
        g.fillRect (getChildComponent (0)->getBounds());
    }
}

void DependencyFilePathPropertyComponent::filesDropped (const StringArray& files, int, int)
{
    const File firstFile (files[0]);

    if (isDirectory)
        setTo (firstFile.isDirectory() ? firstFile
                                       : firstFile.getParentDirectory());
    else
        setTo (firstFile);

    highlightForDragAndDrop = false;
}

void DependencyFilePathPropertyComponent::setTo (const File& f)
{
    pathValue = (pathRelativeTo == File()) ? f.getFullPathName()
                                           : f.getRelativePathFrom (pathRelativeTo);

    textWasEdited();
}

void DependencyFilePathPropertyComponent::enablementChanged()
{
    getValue().referTo (isEnabled() ? pathValue
                                    : pathValueSource.appliesToThisOS() ? pathValueSource.getGlobalSettingsValue()
                                                                        : pathValueSource.getFallbackSettingsValue());
    textWasEdited();
    repaint();
}

void DependencyFilePathPropertyComponent::textWasEdited()
{
    setColour (textColourId, getTextColourToDisplay());
    TextPropertyComponent::textWasEdited();
}

void DependencyFilePathPropertyComponent::valueChanged (Value& value)
{
    if ((value.refersToSameSourceAs (pathValue) && pathValueSource.isUsingGlobalSettings())
             || value.refersToSameSourceAs (pathValueSource.getGlobalSettingsValue()))
        textWasEdited();
}

void DependencyFilePathPropertyComponent::editorShown (Label*, TextEditor& editor)
{
    if (! pathValueSource.isUsingProjectSettings())
        editor.setText (String(), dontSendNotification);
}

void DependencyFilePathPropertyComponent::buttonClicked (Button*)
{
    auto currentFile = pathRelativeTo.getChildFile (pathValue.toString());

    if (isDirectory)
    {
        FileChooser chooser ("Select directory", currentFile);

        if (chooser.browseForDirectory())
            setTo (chooser.getResult());
    }
    else
    {
        FileChooser chooser ("Select file", currentFile, wildcards);

        if (chooser.browseForFileToOpen())
            setTo (chooser.getResult());
    }
}

Colour DependencyFilePathPropertyComponent::getTextColourToDisplay() const
{
    auto alpha = 1.0f;
    auto key = pathValueSource.getKey();
    const auto& globalSettingsValue = pathValueSource.getGlobalSettingsValue();

    if (! pathValueSource.isUsingProjectSettings() && isEnabled())
        alpha = 0.5f;

    if ((key == Ids::defaultUserModulePath && getValue().toString().contains (";")) || ! pathValueSource.appliesToThisOS())
        return findColour (widgetTextColourId).withMultipliedAlpha (alpha);

    auto usingGlobalPath = (getValue().refersToSameSourceAs (globalSettingsValue));

    auto isValidPath = getAppSettings().isGlobalPathValid (pathRelativeTo, key,
                                                           (usingGlobalPath ? globalSettingsValue : pathValue).toString());

    return isValidPath ? findColour (widgetTextColourId).withMultipliedAlpha (alpha)
                       : Colours::red.withMultipliedAlpha (alpha);
}
