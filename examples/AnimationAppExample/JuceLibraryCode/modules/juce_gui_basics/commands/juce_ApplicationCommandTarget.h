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

#ifndef JUCE_APPLICATIONCOMMANDTARGET_H_INCLUDED
#define JUCE_APPLICATIONCOMMANDTARGET_H_INCLUDED


//==============================================================================
/**
    A command target publishes a list of command IDs that it can perform.

    An ApplicationCommandManager despatches commands to targets, which must be
    able to provide information about what commands they can handle.

    To create a target, you'll need to inherit from this class, implementing all of
    its pure virtual methods.

    For info about how a target is chosen to receive a command, see
    ApplicationCommandManager::getFirstCommandTarget().

    @see ApplicationCommandManager, ApplicationCommandInfo
*/
class JUCE_API  ApplicationCommandTarget
{
public:
    //==============================================================================
    /** Creates a command target. */
    ApplicationCommandTarget();

    /** Destructor. */
    virtual ~ApplicationCommandTarget();

    //==============================================================================
    /**
        Contains contextual details about the invocation of a command.
    */
    struct JUCE_API  InvocationInfo
    {
        //==============================================================================
        InvocationInfo (const CommandID commandID);

        //==============================================================================
        /** The UID of the command that should be performed. */
        CommandID commandID;

        /** The command's flags.
            See ApplicationCommandInfo for a description of these flag values.
        */
        int commandFlags;

        //==============================================================================
        /** The types of context in which the command might be called. */
        enum InvocationMethod
        {
            direct = 0,     /**< The command is being invoked directly by a piece of code. */
            fromKeyPress,   /**< The command is being invoked by a key-press. */
            fromMenu,       /**< The command is being invoked by a menu selection. */
            fromButton      /**< The command is being invoked by a button click. */
        };

        /** The type of event that triggered this command. */
        InvocationMethod invocationMethod;

        //==============================================================================
        /** If triggered by a keypress or menu, this will be the component that had the
            keyboard focus at the time.

            If triggered by a button, it may be set to that component, or it may be null.
        */
        Component* originatingComponent;

        //==============================================================================
        /** The keypress that was used to invoke it.

            Note that this will be an invalid keypress if the command was invoked
            by some other means than a keyboard shortcut.
        */
        KeyPress keyPress;

        /** True if the callback is being invoked when the key is pressed,
            false if the key is being released.

            @see KeyPressMappingSet::addCommand()
        */
        bool isKeyDown;

        /** If the key is being released, this indicates how long it had been held
            down for.

            (Only relevant if isKeyDown is false.)
        */
        int millisecsSinceKeyPressed;
    };

    //==============================================================================
    /** This must return the next target to try after this one.

        When a command is being sent, and the first target can't handle
        that command, this method is used to determine the next target that should
        be tried.

        It may return nullptr if it doesn't know of another target.

        If your target is a Component, you would usually use the findFirstTargetParentComponent()
        method to return a parent component that might want to handle it.

        @see invoke
    */
    virtual ApplicationCommandTarget* getNextCommandTarget() = 0;

    /** This must return a complete list of commands that this target can handle.

        Your target should add all the command IDs that it handles to the array that is
        passed-in.
    */
    virtual void getAllCommands (Array<CommandID>& commands) = 0;

    /** This must provide details about one of the commands that this target can perform.

        This will be called with one of the command IDs that the target provided in its
        getAllCommands() methods.

        It should fill-in all appropriate fields of the ApplicationCommandInfo structure with
        suitable information about the command. (The commandID field will already have been filled-in
        by the caller).

        The easiest way to set the info is using the ApplicationCommandInfo::setInfo() method to
        set all the fields at once.

        If the command is currently inactive for some reason, this method must use
        ApplicationCommandInfo::setActive() to make that clear, (or it should set the isDisabled
        bit of the ApplicationCommandInfo::flags field).

        Any default key-presses for the command should be appended to the
        ApplicationCommandInfo::defaultKeypresses field.

        Note that if you change something that affects the status of the commands
        that would be returned by this method (e.g. something that makes some commands
        active or inactive), you should call ApplicationCommandManager::commandStatusChanged()
        to cause the manager to refresh its status.
    */
    virtual void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) = 0;

    /** This must actually perform the specified command.

        If this target is able to perform the command specified by the commandID field of the
        InvocationInfo structure, then it should do so, and must return true.

        If it can't handle this command, it should return false, which tells the caller to pass
        the command on to the next target in line.

        @see invoke, ApplicationCommandManager::invoke
    */
    virtual bool perform (const InvocationInfo& info) = 0;

    //==============================================================================
    /** Makes this target invoke a command.

        Your code can call this method to invoke a command on this target, but normally
        you'd call it indirectly via ApplicationCommandManager::invoke() or
        ApplicationCommandManager::invokeDirectly().

        If this target can perform the given command, it will call its perform() method to
        do so. If not, then getNextCommandTarget() will be used to determine the next target
        to try, and the command will be passed along to it.

        @param invocationInfo       this must be correctly filled-in, describing the context for
                                    the invocation.
        @param asynchronously       if false, the command will be performed before this method returns.
                                    If true, a message will be posted so that the command will be performed
                                    later on the message thread, and this method will return immediately.
        @see perform, ApplicationCommandManager::invoke
    */
    bool invoke (const InvocationInfo& invocationInfo,
                 const bool asynchronously);

    /** Invokes a given command directly on this target.

        This is just an easy way to call invoke() without having to fill out the InvocationInfo
        structure.
    */
    bool invokeDirectly (const CommandID commandID,
                         const bool asynchronously);

    //==============================================================================
    /** Searches this target and all subsequent ones for the first one that can handle
        the specified command.

        This will use getNextCommandTarget() to determine the chain of targets to try
        after this one.
    */
    ApplicationCommandTarget* getTargetForCommand (const CommandID commandID);

    /** Checks whether this command can currently be performed by this target.

        This will return true only if a call to getCommandInfo() doesn't set the
        isDisabled flag to indicate that the command is inactive.
    */
    bool isCommandActive (const CommandID commandID);

    /** If this object is a Component, this method will seach upwards in its current
        UI hierarchy for the next parent component that implements the
        ApplicationCommandTarget class.

        If your target is a Component, this is a very handy method to use in your
        getNextCommandTarget() implementation.
    */
    ApplicationCommandTarget* findFirstTargetParentComponent();

private:
    //==============================================================================
    WeakReference<ApplicationCommandTarget>::Master masterReference;
    friend class WeakReference<ApplicationCommandTarget>;

    class CommandMessage;
    friend class CommandMessage;

    bool tryToInvoke (const InvocationInfo&, bool async);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ApplicationCommandTarget)
};


#endif   // JUCE_APPLICATIONCOMMANDTARGET_H_INCLUDED
