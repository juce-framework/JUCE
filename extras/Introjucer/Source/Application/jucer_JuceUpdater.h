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

#ifndef __JUCER_JUCEUPDATER_JUCEHEADER__
#define __JUCER_JUCEUPDATER_JUCEHEADER__

#include "../Project/jucer_Module.h"


//==============================================================================
class JuceUpdater  : public Component,
                     private ButtonListener,
                     private FilenameComponentListener,
                     private ListBoxModel,
                     private ValueTree::Listener
{
public:
    JuceUpdater (ModuleList& moduleList, const String& message);
    ~JuceUpdater();

    static void show (ModuleList& moduleList, Component* mainWindow, const String& message);

    //==============================================================================
    void resized();
    void paint (Graphics& g);
    void buttonClicked (Button*);
    void filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged);

    int getNumRows();
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);

    void backgroundUpdateComplete (const ModuleList& newList);

private:
    ModuleList& moduleList;
    ModuleList latestList;

    Label messageLabel, label, currentVersionLabel;
    FilenameComponent filenameComp;
    TextButton checkNowButton;
    ListBox availableVersionsList;
    ValueTree versionsToDownload;
    TextButton installButton;
    ToggleButton selectAllButton;
    ScopedPointer<Thread> websiteContacterThread;

    void checkNow();
    void install();
    void updateInstallButtonStatus();
    void refresh();
    void selectAll();
    int getNumCheckedModules() const;
    bool isLatestVersion (const String& moduleID) const;

    void valueTreePropertyChanged (ValueTree&, const Identifier&);
    void valueTreeChildAdded (ValueTree&, ValueTree&);
    void valueTreeChildRemoved (ValueTree&, ValueTree&);
    void valueTreeChildOrderChanged (ValueTree&);
    void valueTreeParentChanged (ValueTree&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceUpdater)
};

#endif   // __JUCER_JUCEUPDATER_JUCEHEADER__
