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

#ifndef JUCE_PLUGINLISTCOMPONENT_H_INCLUDED
#define JUCE_PLUGINLISTCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A component displaying a list of plugins, with options to scan for them,
    add, remove and sort them.
*/
class JUCE_API  PluginListComponent   : public Component,
                                        public FileDragAndDropTarget,
                                        private ListBoxModel,
                                        private ChangeListener,
                                        private ButtonListener  // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    //==============================================================================
    /**
        Creates the list component.

        For info about the deadMansPedalFile, see the PluginDirectoryScanner constructor.
        The properties file, if supplied, is used to store the user's last search paths.
    */
    PluginListComponent (AudioPluginFormatManager& formatManager,
                         KnownPluginList& listToRepresent,
                         const File& deadMansPedalFile,
                         PropertiesFile* propertiesToUse);

    /** Destructor. */
    ~PluginListComponent();

    /** Changes the text in the panel's button. */
    void setOptionsButtonText (const String& newText);

    /** Sets how many threads to simultaneously scan for plugins.
        If this is 0, then all scanning happens on the message thread (this is the default)
    */
    void setNumberOfThreadsForScanning (int numThreads);

    /** Returns the last search path stored in a given properties file for the specified format. */
    static FileSearchPath getLastSearchPath (PropertiesFile& properties, AudioPluginFormat& format);

    /** Stores a search path in a properties file for the given format. */
    static void setLastSearchPath (PropertiesFile& properties, AudioPluginFormat& format,
                                   const FileSearchPath& newPath);

    /** Triggers an asynchronous scan for the given format. */
    void scanFor (AudioPluginFormat& format);

    /** Returns true if there's currently a scan in progress. */
    bool isScanning() const noexcept;

    //==============================================================================
    /** @internal */
    void resized() override;
    /** @internal */
    bool isInterestedInFileDrag (const StringArray&) override;
    /** @internal */
    void filesDropped (const StringArray&, int, int) override;
    /** @internal */
    int getNumRows() override;
    /** @internal */
    void paintListBoxItem (int row, Graphics&, int width, int height, bool rowIsSelected) override;
    /** @internal */
    void deleteKeyPressed (int lastRowSelected) override;

private:
    //==============================================================================
    AudioPluginFormatManager& formatManager;
    KnownPluginList& list;
    File deadMansPedalFile;
    ListBox listBox;
    TextButton optionsButton;
    PropertiesFile* propertiesToUse;
    int numThreads;

    class Scanner;
    friend class Scanner;
    friend struct ContainerDeletePolicy<Scanner>;
    ScopedPointer<Scanner> currentScanner;

    void scanFinished (const StringArray&);
    static void optionsMenuStaticCallback (int, PluginListComponent*);
    void optionsMenuCallback (int);
    void updateList();
    void removeSelected();
    void showSelectedFolder();
    bool canShowSelectedFolder() const;
    void removeMissingPlugins();

    void buttonClicked (Button*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListComponent)
};


#endif   // JUCE_PLUGINLISTCOMPONENT_H_INCLUDED
