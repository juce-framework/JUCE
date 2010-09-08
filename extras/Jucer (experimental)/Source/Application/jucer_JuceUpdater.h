/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_JUCEUPDATER_H_81537815__
#define __JUCER_JUCEUPDATER_H_81537815__


//==============================================================================
class JuceUpdater  : public Component,
                     public ButtonListener,
                     public FilenameComponentListener,
                     public ListBoxModel
{
public:
    JuceUpdater();
    ~JuceUpdater();

    static void show (Component* mainWindow);

    //==============================================================================
    void resized();
    void paint (Graphics& g);
    void buttonClicked (Button*);
    void filenameComponentChanged (FilenameComponent* fileComponentThatHasChanged);

    int getNumRows();
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);

private:
    Label label, currentVersionLabel;
    FilenameComponent filenameComp;
    TextButton checkNowButton;
    ListBox availableVersionsList;

    XmlElement* downloadVersionList();
    const String getCurrentVersion();
    bool isAlreadyUpToDate();

    struct VersionInfo
    {
        URL url;
        String desc;
        String version;
        String date;
    };

    OwnedArray<VersionInfo> availableVersions;

    void updateVersions (const XmlElement& xml);
    void applyVersion (VersionInfo* version);
};


#endif  // __JUCER_JUCEUPDATER_H_81537815__
