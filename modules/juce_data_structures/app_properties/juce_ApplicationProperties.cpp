/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ApplicationProperties::~ApplicationProperties()
{
    closeFiles();
}

//==============================================================================
void ApplicationProperties::setStorageParameters (const PropertiesFile::Options& newOptions)
{
    options = newOptions;
}

//==============================================================================
void ApplicationProperties::openFiles()
{
    // You need to call setStorageParameters() before trying to get hold of the properties!
    jassert (options.applicationName.isNotEmpty());

    if (options.applicationName.isNotEmpty())
    {
        PropertiesFile::Options o (options);

        if (userProps == nullptr)
        {
            o.commonToAllUsers = false;
            userProps.reset (new PropertiesFile (o));
        }

        if (commonProps == nullptr)
        {
            o.commonToAllUsers = true;
            commonProps.reset (new PropertiesFile (o));
        }

        userProps->setFallbackPropertySet (commonProps.get());
    }
}

PropertiesFile* ApplicationProperties::getUserSettings()
{
    if (userProps == nullptr)
        openFiles();

    return userProps.get();
}

PropertiesFile* ApplicationProperties::getCommonSettings (const bool returnUserPropsIfReadOnly)
{
    if (commonProps == nullptr)
        openFiles();

    if (returnUserPropsIfReadOnly)
    {
        if (commonSettingsAreReadOnly == 0)
            commonSettingsAreReadOnly = commonProps->save() ? -1 : 1;

        if (commonSettingsAreReadOnly > 0)
            return userProps.get();
    }

    return commonProps.get();
}

bool ApplicationProperties::saveIfNeeded()
{
    return (userProps == nullptr || userProps->saveIfNeeded())
         && (commonProps == nullptr || commonProps->saveIfNeeded());
}

void ApplicationProperties::closeFiles()
{
    userProps.reset();
    commonProps.reset();
}

} // namespace juce
