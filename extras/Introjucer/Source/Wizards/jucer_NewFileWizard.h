/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef __JUCER_NEWFILEWIZARD_JUCEHEADER__
#define __JUCER_NEWFILEWIZARD_JUCEHEADER__

#include "../jucer_Headers.h"
#include "../Project/jucer_Project.h"


//==============================================================================
class NewFileWizard
{
public:
    //==============================================================================
    NewFileWizard();
    ~NewFileWizard();

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
    void addWizardsToMenu (PopupMenu&) const;
    bool runWizardFromMenu (int chosenMenuItemID, const Project::Item& projectGroupToAddTo) const;

    void registerWizard (Type*);

private:
    OwnedArray<Type> wizards;
};


#endif   // __JUCER_NEWFILEWIZARD_JUCEHEADER__
