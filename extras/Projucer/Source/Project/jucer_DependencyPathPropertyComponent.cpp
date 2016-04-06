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
DependencyPathValueSource::DependencyPathValueSource (const Value& projectSettingsPath,
                                                      Identifier globalSettingsKey,
                                                      DependencyPathOS osThisSettingAppliesTo)
  : projectSettingsValue (projectSettingsPath),
    globalKey (globalSettingsKey),
    os (osThisSettingAppliesTo),
    globalSettingsValue (getAppSettings().getGlobalPath (globalKey, os)),
    fallbackValue (getAppSettings().getFallbackPath (globalKey, os))
{
    globalSettingsValue.addListener (this);
}

bool DependencyPathValueSource::isValidPath() const
{
    // if we are on another OS than the one which this path setting is for,
    // we have no way of knowing whether the path is valid - so just assume it is:
    if (! appliesToThisOS())
        return true;

    return getAppSettings().isGlobalPathValid (globalKey, getValue().toString());
}

//==============================================================================
DependencyPathPropertyComponent::DependencyPathPropertyComponent (const Value& value,
                                                                  const String& propertyName)
try : TextPropertyComponent (propertyName, 1024, false),
      pathValue (value),
      pathValueSource (dynamic_cast<DependencyPathValueSource&> (pathValue.getValueSource()))
{
    bool initialValueIsEmpty = ! pathValueSource.isUsingProjectSettings();

    getValue().referTo (pathValue);

    // the following step is necessary because the above referTo() has internally called setValue(),
    // which has set the project value to whatever is displayed in the label (this may be the
    // global/fallback value). In this case we have to reset the project value to blank:
    if (initialValueIsEmpty)
        getValue().setValue (String::empty);

    getValue().addListener (this);
    setColour (textColourId, getTextColourToDisplay());

    if (Label* label = dynamic_cast<Label*> (getChildComponent (0)))
        label->addListener (this);
    else
        jassertfalse;
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
        return pathValueSource.isValidPath() ? Colours::grey
                                              : Colours::lightpink;

    return pathValueSource.isValidPath() ? Colours::black
                                          : Colours::red;
}

void DependencyPathPropertyComponent::labelTextChanged (Label*)
{
}

void DependencyPathPropertyComponent::editorShown (Label* /*label*/, TextEditor& editor)
{
    if (! pathValueSource.isUsingProjectSettings())
        editor.setText (String::empty, dontSendNotification);
}

void DependencyPathPropertyComponent::editorHidden (Label*, TextEditor&)
{
}
