/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
        return pathValueSource.isValidPath (pathRelativeTo) ? Colours::grey
                                                            : Colours::lightpink;

    return pathValueSource.isValidPath (pathRelativeTo) ? Colours::black
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
