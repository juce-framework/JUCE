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

#ifndef JUCE_APPLICATIONCOMMANDMANAGER_H_INCLUDED
#define JUCE_APPLICATIONCOMMANDMANAGER_H_INCLUDED


//==============================================================================
/**
    One of these objects holds a list of all the commands your app can perform,
    and despatches these commands when needed.

    Application commands are a good way to trigger actions in your app, e.g. "Quit",
    "Copy", "Paste", etc. Menus, buttons and keypresses can all be given commands
    to invoke automatically, which means you don't have to handle the result of a menu
    or button click manually. Commands are despatched to ApplicationCommandTarget objects
    which can choose which events they want to handle.

    This architecture also allows for nested ApplicationCommandTargets, so that for example
    you could have two different objects, one inside the other, both of which can respond to
    a "delete" command. Depending on which one has focus, the command will be sent to the
    appropriate place, regardless of whether it was triggered by a menu, keypress or some other
    method.

    To set up your app to use commands, you'll need to do the following:

    - Create a global ApplicationCommandManager to hold the list of all possible
      commands. (This will also manage a set of key-mappings for them).

    - Make some of your UI components (or other objects) inherit from ApplicationCommandTarget.
      This allows the object to provide a list of commands that it can perform, and
      to handle them.

    - Register each type of command using ApplicationCommandManager::registerAllCommandsForTarget(),
      or ApplicationCommandManager::registerCommand().

    - If you want key-presses to trigger your commands, use the ApplicationCommandManager::getKeyMappings()
      method to access the key-mapper object, which you will need to register as a key-listener
      in whatever top-level component you're using. See the KeyPressMappingSet class for more help
      about setting this up.

    - Use methods such as PopupMenu::addCommandItem() or Button::setCommandToTrigger() to
      cause these commands to be invoked automatically.

    - Commands can be invoked directly by your code using ApplicationCommandManager::invokeDirectly().

    When a command is invoked, the ApplicationCommandManager will try to choose the best
    ApplicationCommandTarget to receive the specified command. To do this it will use the
    current keyboard focus to see which component might be interested, and will search the
    component hierarchy for those that also implement the ApplicationCommandTarget interface.
    If an ApplicationCommandTarget isn't interested in the command that is being invoked, then
    the next one in line will be tried (see the ApplicationCommandTarget::getNextCommandTarget()
    method), and so on until ApplicationCommandTarget::getNextCommandTarget() returns nullptr.
    At this point if the command still hasn't been performed, it will be passed to the current
    JUCEApplication object (which is itself an ApplicationCommandTarget).

    To exert some custom control over which ApplicationCommandTarget is chosen to invoke a command,
    you can override the ApplicationCommandManager::getFirstCommandTarget() method and choose
    the object yourself.

    @see ApplicationCommandTarget, ApplicationCommandInfo
*/
class JUCE_API  ApplicationCommandManager   : private AsyncUpdater,
                                              private FocusChangeListener
{
public:
    //==============================================================================
    /** Creates an ApplicationCommandManager.

        Once created, you'll need to register all your app's commands with it, using
        ApplicationCommandManager::registerAllCommandsForTarget() or
        ApplicationCommandManager::registerCommand().
    */
    ApplicationCommandManager();

    /** Destructor.

        Make sure that you don't delete this if pointers to it are still being used by
        objects such as PopupMenus or Buttons.
    */
    virtual ~ApplicationCommandManager();

    //==============================================================================
    /** Clears the current list of all commands.
        Note that this will also clear the contents of the KeyPressMappingSet.
    */
    void clearCommands();

    /** Adds a command to the list of registered commands.
        @see registerAllCommandsForTarget
    */
    void registerCommand (const ApplicationCommandInfo& newCommand);

    /** Adds all the commands that this target publishes to the manager's list.

        This will use ApplicationCommandTarget::getAllCommands() and ApplicationCommandTarget::getCommandInfo()
        to get details about all the commands that this target can do, and will call
        registerCommand() to add each one to the manger's list.

        @see registerCommand
    */
    void registerAllCommandsForTarget (ApplicationCommandTarget* target);

    /** Removes the command with a specified ID.
        Note that this will also remove any key mappings that are mapped to the command.
    */
    void removeCommand (CommandID commandID);

    /** This should be called to tell the manager that one of its registered commands may have changed
        its active status.

        Because the command manager only finds out whether a command is active or inactive by querying
        the current ApplicationCommandTarget, this is used to tell it that things may have changed. It
        allows things like buttons to update their enablement, etc.

        This method will cause an asynchronous call to ApplicationCommandManagerListener::applicationCommandListChanged()
        for any registered listeners.
    */
    void commandStatusChanged();

    //==============================================================================
    /** Returns the number of commands that have been registered.
        @see registerCommand
    */
    int getNumCommands() const noexcept                                             { return commands.size(); }

    /** Returns the details about one of the registered commands.
        The index is between 0 and (getNumCommands() - 1).
    */
    const ApplicationCommandInfo* getCommandForIndex (int index) const noexcept     { return commands [index]; }

    /** Returns the details about a given command ID.

        This will search the list of registered commands for one with the given command
        ID number, and return its associated info. If no matching command is found, this
        will return 0.
    */
    const ApplicationCommandInfo* getCommandForID (CommandID commandID) const noexcept;

    /** Returns the name field for a command.

        An empty string is returned if no command with this ID has been registered.
        @see getDescriptionOfCommand
    */
    String getNameOfCommand (CommandID commandID) const noexcept;

    /** Returns the description field for a command.

        An empty string is returned if no command with this ID has been registered. If the
        command has no description, this will return its short name field instead.

        @see getNameOfCommand
    */
    String getDescriptionOfCommand (CommandID commandID) const noexcept;

    /** Returns the list of categories.

        This will go through all registered commands, and return a list of all the distinct
        categoryName values from their ApplicationCommandInfo structure.

        @see getCommandsInCategory()
    */
    StringArray getCommandCategories() const;

    /** Returns a list of all the command UIDs in a particular category.
        @see getCommandCategories()
    */
    Array<CommandID> getCommandsInCategory (const String& categoryName) const;

    //==============================================================================
    /** Returns the manager's internal set of key mappings.

        This object can be used to edit the keypresses. To actually link this object up
        to invoke commands when a key is pressed, see the comments for the KeyPressMappingSet
        class.

        @see KeyPressMappingSet
    */
    KeyPressMappingSet* getKeyMappings() const noexcept                         { return keyMappings; }


    //==============================================================================
    /** Invokes the given command directly, sending it to the default target.
        This is just an easy way to call invoke() without having to fill out the InvocationInfo
        structure.
    */
    bool invokeDirectly (CommandID commandID, bool asynchronously);

    /** Sends a command to the default target.

        This will choose a target using getFirstCommandTarget(), and send the specified command
        to it using the ApplicationCommandTarget::invoke() method. This means that if the
        first target can't handle the command, it will be passed on to targets further down the
        chain (see ApplicationCommandTarget::invoke() for more info).

        @param invocationInfo       this must be correctly filled-in, describing the context for
                                    the invocation.
        @param asynchronously       if false, the command will be performed before this method returns.
                                    If true, a message will be posted so that the command will be performed
                                    later on the message thread, and this method will return immediately.

        @see ApplicationCommandTarget::invoke
    */
    bool invoke (const ApplicationCommandTarget::InvocationInfo& invocationInfo,
                 bool asynchronously);


    //==============================================================================
    /** Chooses the ApplicationCommandTarget to which a command should be sent.

        Whenever the manager needs to know which target a command should be sent to, it calls
        this method to determine the first one to try.

        By default, this method will return the target that was set by calling setFirstCommandTarget().
        If no target is set, it will return the result of findDefaultComponentTarget().

        If you need to make sure all commands go via your own custom target, then you can
        either use setFirstCommandTarget() to specify a single target, or override this method
        if you need more complex logic to choose one.

        It may return nullptr if no targets are available.

        @see getTargetForCommand, invoke, invokeDirectly
    */
    virtual ApplicationCommandTarget* getFirstCommandTarget (CommandID commandID);

    /** Sets a target to be returned by getFirstCommandTarget().

        If this is set to nullptr, then getFirstCommandTarget() will by default return the
        result of findDefaultComponentTarget().

        If you use this to set a target, make sure you call setFirstCommandTarget(nullptr)
        before deleting the target object.
    */
    void setFirstCommandTarget (ApplicationCommandTarget* newTarget) noexcept;

    /** Tries to find the best target to use to perform a given command.

        This will call getFirstCommandTarget() to find the preferred target, and will
        check whether that target can handle the given command. If it can't, then it'll use
        ApplicationCommandTarget::getNextCommandTarget() to find the next one to try, and
        so on until no more are available.

        If no targets are found that can perform the command, this method will return nullptr.

        If a target is found, then it will get the target to fill-in the upToDateInfo
        structure with the latest info about that command, so that the caller can see
        whether the command is disabled, ticked, etc.
    */
    ApplicationCommandTarget* getTargetForCommand (CommandID commandID,
                                                   ApplicationCommandInfo& upToDateInfo);

    //==============================================================================
    /** Registers a listener that will be called when various events occur. */
    void addListener (ApplicationCommandManagerListener* listener);

    /** Deregisters a previously-added listener. */
    void removeListener (ApplicationCommandManagerListener* listener);

    //==============================================================================
    /** Looks for a suitable command target based on which Components have the keyboard focus.

        This is used by the default implementation of ApplicationCommandTarget::getFirstCommandTarget(),
        but is exposed here in case it's useful.

        It tries to pick the best ApplicationCommandTarget by looking at focused components, top level
        windows, etc., and using the findTargetForComponent() method.
    */
    static ApplicationCommandTarget* findDefaultComponentTarget();

    /** Examines this component and all its parents in turn, looking for the first one
        which is a ApplicationCommandTarget.

        Returns the first ApplicationCommandTarget that it finds, or nullptr if none of them
        implement that class.
    */
    static ApplicationCommandTarget* findTargetForComponent (Component*);


private:
    //==============================================================================
    OwnedArray<ApplicationCommandInfo> commands;
    ListenerList<ApplicationCommandManagerListener> listeners;
    ScopedPointer<KeyPressMappingSet> keyMappings;
    ApplicationCommandTarget* firstTarget;

    void sendListenerInvokeCallback (const ApplicationCommandTarget::InvocationInfo&);
    void handleAsyncUpdate() override;
    void globalFocusChanged (Component*) override;

   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // This is just here to cause a compile error in old code that hasn't been changed to use the new
    // version of this method.
    virtual short getFirstCommandTarget() { return 0; }
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ApplicationCommandManager)
};


//==============================================================================
/**
    A listener that receives callbacks from an ApplicationCommandManager when
    commands are invoked or the command list is changed.

    @see ApplicationCommandManager::addListener, ApplicationCommandManager::removeListener

*/
class JUCE_API  ApplicationCommandManagerListener
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~ApplicationCommandManagerListener()  {}

    /** Called when an app command is about to be invoked. */
    virtual void applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo&) = 0;

    /** Called when commands are registered or deregistered from the
        command manager, or when commands are made active or inactive.

        Note that if you're using this to watch for changes to whether a command is disabled,
        you'll need to make sure that ApplicationCommandManager::commandStatusChanged() is called
        whenever the status of your command might have changed.
    */
    virtual void applicationCommandListChanged() = 0;
};



#endif   // JUCE_APPLICATIONCOMMANDMANAGER_H_INCLUDED
