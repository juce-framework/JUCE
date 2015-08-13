/*
  ==============================================================================

    jucer_GlobalDefaultedTextPropertyComponent.cpp
    Created: 27 Jul 2015 10:42:17am
    Author:  Joshua Gerrard

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_DependencyPathPropertyComponent.h"
#include "../Application/jucer_GlobalPreferences.h"

//==============================================================================
const String DependencyPath::vst2KeyName = "vst2Path";
const String DependencyPath::vst3KeyName = "vst3Path";
const String DependencyPath::rtasKeyName = "rtasPath";
const String DependencyPath::aaxKeyName = "aaxPath";
const String DependencyPath::androidSdkKeyName = "androidSdkPath";
const String DependencyPath::androidNdkKeyName = "androidNdkPath";

//==============================================================================
DependencyPathPropertyComponent::DependencyPathPropertyComponent (const Value& value,
                                                                  const String& propertyName,
                                                                  const String& globalKeyName,
                                                                  DependencyPathOS os)
    : TextPropertyComponent (propertyName, 1024, false),
      globalKey (globalKeyName),
      pathValueSource (new DependencyPathValueSource (value,
                                                     PathSettingsTab::getPathByKey (globalKeyName, os),
                                                     PathSettingsTab::getFallbackPathByKey (globalKeyName, os),
                                                     os)),
      pathValue (pathValueSource)
{
    bool initialValueIsEmpty = value.toString().isEmpty();

    getValue().referTo (pathValue);

    if (initialValueIsEmpty)
        getValue().setValue (String::empty);

    getValue().addListener (this);
    setColour (textColourId, getTextColourToDisplay());

    if (Label* label = dynamic_cast<Label*> (getChildComponent (0)))
        label->addListener (this);
    else
        jassertfalse;
}

void DependencyPathPropertyComponent::valueChanged (Value& value)
{
    // this callback handles the update of this setting in case
    // the user changed the global preferences.
    if (value.refersToSameSourceAs (pathValue) && pathValueSource->isUsingGlobalSettings())
        textWasEdited();
}

void DependencyPathPropertyComponent::textWasEdited()
{
    setColour (textColourId, getTextColourToDisplay());
    TextPropertyComponent::textWasEdited();
}

Colour DependencyPathPropertyComponent::getTextColourToDisplay() const
{
    if (! pathValueSource->isUsingProjectSettings())
        return isValidPath() ? Colours::grey
                             : Colours::lightpink;

    return isValidPath() ? Colours::black
                         : Colours::red;
}

bool DependencyPathPropertyComponent::isValidPath() const
{
    // if we are on another OS than the one which this path setting is for,
    // we have no way of knowing whether the path is valid - so just assume it is:
    if (! pathValueSource->appliesToThisOS())
        return true;

    return PathSettingsTab::checkPathByKey (globalKey, getValue().toString());
}

void DependencyPathPropertyComponent::labelTextChanged (Label*)
{
}

void DependencyPathPropertyComponent::editorShown (Label* /*label*/, TextEditor& editor)
{
    if (! pathValueSource->isUsingProjectSettings())
        editor.setText (String::empty, dontSendNotification);
}

void DependencyPathPropertyComponent::editorHidden (Label*, TextEditor&)
{
}
