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

#ifndef JUCER_DEPENDENCYPATHPROPERTYCOMPONENT_H_INCLUDED
#define JUCER_DEPENDENCYPATHPROPERTYCOMPONENT_H_INCLUDED


//==============================================================================
/** This ValueSource type implements the fallback logic required for dependency
    path settings: use the project exporter value; if this is empty, fall back to
    the global preference value; if the exporter is supposed to run on another
    OS and we don't know what the global preferences on that other machine are,
    fall back to a generic OS-specific fallback value.
*/
class DependencyPathValueSource : public Value::ValueSource,
                                  private Value::Listener
{
public:
    DependencyPathValueSource (const Value& projectSettingsPath,
                               Identifier globalSettingsKey,
                               DependencyPathOS osThisSettingAppliesTo);

    /** This gets the currently used value, which may be either
        the project setting, the global setting, or the fallback value. */
    var getValue() const override
    {
        if (isUsingProjectSettings())
            return projectSettingsValue;

        if (isUsingGlobalSettings())
            return globalSettingsValue;

        return fallbackValue;
    }

    void setValue (const var& newValue) override
    {
        projectSettingsValue = newValue;

        if (isUsingProjectSettings())
            sendChangeMessage (false);
    }

    bool isUsingProjectSettings() const
    {
        return projectSettingsValueIsValid();
    }

    bool isUsingGlobalSettings() const
    {
        return ! projectSettingsValueIsValid() && globalSettingsValueIsValid();
    }

    bool isUsingFallbackValue() const
    {
        return ! projectSettingsValueIsValid() && !globalSettingsValueIsValid();
    }

    bool appliesToThisOS() const
    {
        return os == TargetOS::getThisOS();
    }

    bool isValidPath (const File& relativeTo) const;

    bool isValidPath() const;

private:
    void valueChanged (Value& value) override
    {
        if ((value.refersToSameSourceAs (globalSettingsValue) && isUsingGlobalSettings()))
        {
            sendChangeMessage (true);
            setValue (String()); // make sure that the project-specific value is still blank
        }
    }

    /** This defines when to use the project setting, and when to
        consider it invalid and to fall back to the global setting or
        the fallback value. */
    bool projectSettingsValueIsValid() const
    {
        return ! projectSettingsValue.toString().isEmpty();
    }

    /** This defines when to use the global setting - given the project setting
        is invalid, and when to fall back to the fallback value instead. */
    bool globalSettingsValueIsValid() const
    {
        // only use the global settings if they are set on the same OS
        // that this setting is for!
        DependencyPathOS thisOS = TargetOS::getThisOS();

        return thisOS == TargetOS::unknown ? false : os == thisOS;
    }

    /** the dependency path setting as set in this Projucer project. */
    Value projectSettingsValue;

    /** the global key used in the application settings for the global setting value.
        needed for checking whether the path is valid. */
    Identifier globalKey;

    /** on what operating system should this dependency path be used?
     note that this is *not* the os that is targeted by the project,
     but rather the os on which the project will be compiled
     (= on which the path settings need to be set correctly). */
    DependencyPathOS os;

    /** the dependency path global setting on this machine.
        used when there value set for this project is invalid. */
    Value globalSettingsValue;

    /** the dependency path fallback setting. used instead of the global setting
        whenever the latter doesn't apply, e.g. the setting is for another
        OS than the ome this machine is running. */
    String fallbackValue;
};


//==============================================================================
class DependencyPathPropertyComponent : public TextPropertyComponent,
                                        private Value::Listener,
                                        private Label::Listener
{
public:
    DependencyPathPropertyComponent (const File& pathRelativeToUse,
                                     const Value& value,
                                     const String& propertyName);


private:
    /** This function defines what colour the label text should assume
        depending on the current state of the value the component tracks. */
    Colour getTextColourToDisplay() const;

    /** This function handles path changes because of user input. */
    void textWasEdited() override;

    /** This function handles path changes because the global path changed. */
    void valueChanged (Value& value) override;

    /** If the dependency path is relative, relative to which directory should
        we check if an object is available. */
    File pathRelativeTo;

    /** the value that represents this dependency path setting. */
    Value pathValue;

    /** a reference to the value source that this value refers to. */
    DependencyPathValueSource& pathValueSource;

    // Label::Listener overrides:
    void labelTextChanged (Label* labelThatHasChanged) override;
    void editorShown (Label*, TextEditor&) override;
    void editorHidden (Label*, TextEditor&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DependencyPathPropertyComponent)
};


#endif  // JUCER_DEPENDENCYPATHPROPERTYCOMPONENT_H_INCLUDED
