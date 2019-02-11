/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component displaying a list of plugins, with options to scan for them,
    add, remove and sort them.

    @tags{Audio}
*/
class JUCE_API  PluginListComponent   : public Component,
                                        public FileDragAndDropTarget,
                                        private ChangeListener
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
                         PropertiesFile* propertiesToUse,
                         bool allowPluginsWhichRequireAsynchronousInstantiation = false);

    /** Destructor. */
    ~PluginListComponent() override;

    /** Changes the text in the panel's options button. */
    void setOptionsButtonText (const String& newText);

    /** Changes the text in the progress dialog box that is shown when scanning. */
    void setScanDialogText (const String& textForProgressWindowTitle,
                            const String& textForProgressWindowDescription);

    /** Sets how many threads to simultaneously scan for plugins.
     If this is 0, then all scanning happens on the message thread (this is the default when
     allowPluginsWhichRequireAsynchronousInstantiation is false). If
     allowPluginsWhichRequireAsynchronousInstantiation is true then numThreads must not
     be zero (it is one by default). */
    void setNumberOfThreadsForScanning (int numThreads);

    /** Returns the last search path stored in a given properties file for the specified format. */
    static FileSearchPath getLastSearchPath (PropertiesFile&, AudioPluginFormat&);

    /** Stores a search path in a properties file for the given format. */
    static void setLastSearchPath (PropertiesFile&, AudioPluginFormat&, const FileSearchPath&);

    /** Triggers an asynchronous scan for the given format. */
    void scanFor (AudioPluginFormat&);

    /** Triggers an asynchronous scan for the given format and scans only the given files or identifiers.
        @see AudioPluginFormat::searchPathsForPlugins
    */
    void scanFor (AudioPluginFormat&, const StringArray& filesOrIdentifiersToScan);

    /** Returns true if there's currently a scan in progress. */
    bool isScanning() const noexcept;

    /** Removes the plugins currently selected in the table. */
    void removeSelectedPlugins();

    /** Sets a custom table model to be used.
        This will take ownership of the model and delete it when no longer needed.
     */
    void setTableModel (TableListBoxModel* model);

    /** Returns the table used to display the plugin list. */
    TableListBox& getTableListBox() noexcept            { return table; }

private:
    //==============================================================================
    AudioPluginFormatManager& formatManager;
    KnownPluginList& list;
    File deadMansPedalFile;
    TableListBox table;
    TextButton optionsButton;
    PropertiesFile* propertiesToUse;
    String dialogTitle, dialogText;
    bool allowAsync;
    int numThreads;

    class TableModel;
    std::unique_ptr<TableListBoxModel> tableModel;

    class Scanner;
    std::unique_ptr<Scanner> currentScanner;

    void scanFinished (const StringArray&);
    static void optionsMenuStaticCallback (int, PluginListComponent*);
    void optionsMenuCallback (int);
    void updateList();
    void showSelectedFolder();
    bool canShowSelectedFolder() const;
    void removeMissingPlugins();
    void removePluginItem (int index);
    void showOptionsMenu();

    void resized() override;
    bool isInterestedInFileDrag (const StringArray&) override;
    void filesDropped (const StringArray&, int, int) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListComponent)
};

} // namespace juce
