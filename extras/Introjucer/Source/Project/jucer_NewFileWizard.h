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

#ifndef __JUCER_NEWFILEWIZARD_JUCEHEADER__
#define __JUCER_NEWFILEWIZARD_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class NewFileWizard  : public DeletedAtShutdown
{
public:
    //==============================================================================
    NewFileWizard();
    ~NewFileWizard();

    juce_DeclareSingleton_SingleThreaded_Minimal (NewFileWizard);

    //==============================================================================
    class Type
    {
    public:
        Type() {}
        virtual ~Type()  {}

        //==============================================================================
        virtual String getName() = 0;
        virtual void createNewFile (Project::Item projectGroupToAddTo) = 0;

    protected:
        //==============================================================================
        File askUserToChooseNewFile (const String& suggestedFilename, const String& wildcard,
                                     const Project::Item& projectGroupToAddTo);

        static void showFailedToWriteMessage (const File& file);
    };

    //==============================================================================
    void addWizardsToMenu (PopupMenu& m) const;
    bool runWizardFromMenu (int chosenMenuItemID, const Project::Item& projectGroupToAddTo) const;

    void registerWizard (Type* newWizard);

private:
    OwnedArray <Type> wizards;
};


#endif   // __JUCER_NEWFILEWIZARD_JUCEHEADER__
