/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ApplicationProperties.h"
#include "../gui/components/windows/juce_AlertWindow.h"
#include "../text/juce_LocalisedStrings.h"


//==============================================================================
juce_ImplementSingleton (ApplicationProperties)


//==============================================================================
ApplicationProperties::ApplicationProperties()
    : commonSettingsAreReadOnly (0)
{
}

ApplicationProperties::~ApplicationProperties()
{
    closeFiles();
    clearSingletonInstance();
}

//==============================================================================
void ApplicationProperties::setStorageParameters (const PropertiesFile::Options& newOptions)
{
    options = newOptions;
}

bool ApplicationProperties::testWriteAccess (const bool testUserSettings,
                                             const bool testCommonSettings,
                                             const bool showWarningDialogOnFailure)
{
    const bool userOk    = (! testUserSettings)   || getUserSettings()->save();
    const bool commonOk  = (! testCommonSettings) || getCommonSettings (false)->save();

    if (! (userOk && commonOk))
    {
        if (showWarningDialogOnFailure)
        {
            String filenames;

            if (userProps != nullptr && ! userOk)
                filenames << '\n' << userProps->getFile().getFullPathName();

            if (commonProps != nullptr && ! commonOk)
                filenames << '\n' << commonProps->getFile().getFullPathName();

            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              options.applicationName + TRANS(" - Unable to save settings"),
                                              TRANS("An error occurred when trying to save the application's settings file...\n\nIn order to save and restore its settings, ")
                                                + options.applicationName + TRANS(" needs to be able to write to the following files:\n")
                                                + filenames
                                                + TRANS("\n\nMake sure that these files aren't read-only, and that the disk isn't full."));
        }

        return false;
    }

    return true;
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
            userProps = new PropertiesFile (o);
        }

        if (commonProps == nullptr)
        {
            o.commonToAllUsers = true;
            commonProps = new PropertiesFile (o);
        }

        userProps->setFallbackPropertySet (commonProps);
    }
}

PropertiesFile* ApplicationProperties::getUserSettings()
{
    if (userProps == nullptr)
        openFiles();

    return userProps;
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
            return userProps;
    }

    return commonProps;
}

bool ApplicationProperties::saveIfNeeded()
{
    return (userProps == nullptr || userProps->saveIfNeeded())
         && (commonProps == nullptr || commonProps->saveIfNeeded());
}

void ApplicationProperties::closeFiles()
{
    userProps = nullptr;
    commonProps = nullptr;
}


END_JUCE_NAMESPACE
