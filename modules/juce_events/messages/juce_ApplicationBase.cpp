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

JUCEApplicationBase::CreateInstanceFunction JUCEApplicationBase::createInstance = 0;
JUCEApplicationBase* JUCEApplicationBase::appInstance = nullptr;

JUCEApplicationBase::JUCEApplicationBase()
{
    jassert (isStandaloneApp() && appInstance == nullptr);
    appInstance = this;
}

JUCEApplicationBase::~JUCEApplicationBase()
{
    jassert (appInstance == this);
    appInstance = nullptr;
}

// This is called on the Mac and iOS where the OS doesn't allow the stack to unwind on shutdown..
void JUCEApplicationBase::appWillTerminateByForce()
{
    JUCE_AUTORELEASEPOOL
    {
        {
            const ScopedPointer<JUCEApplicationBase> app (appInstance);

            if (app != nullptr)
                app->shutdownApp();
        }

        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    }
}
