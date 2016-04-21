/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

struct MessageHandler
{
    virtual ~MessageHandler() {}
    virtual bool sendMessage (const ValueTree&) = 0;

    static MemoryBlock convertMessage (const ValueTree& tree)
    {
        MemoryOutputStream out;
        tree.writeToStream (out);
        return out.getMemoryBlock();
    }

    static ValueTree convertMessage (const MemoryBlock& rawData)
    {
        return ValueTree::readFromData (rawData.getData(), rawData.getSize());
    }
};

//==============================================================================
static inline Rectangle<int> varToRect (const var& v)
{
    if (const Array<var>* obj = v.getArray())
    {
        if (obj->size() == 4)
        {
            int intArray[4];

            for (int i = 0; i < 4; ++i)
            {
                const var& p = obj->getReference (i);

                if (p.isInt() || p.isDouble() || p.isInt64())
                    intArray[i] = static_cast<int> (p);
                else
                    return Rectangle<int>();
            }

            return Rectangle<int> (intArray[0], intArray[1], intArray[2], intArray[3]);
        }
    }

    return Rectangle<int>();
}

static inline var rectToVar (const Rectangle<int>& rect)
{
    Array<var> retval;

    retval.add (var (rect.getX()));
    retval.add (var (rect.getY()));
    retval.add (var (rect.getWidth()));
    retval.add (var (rect.getHeight()));

    return var (retval);
}

//==============================================================================
namespace MessageTypes
{
    inline bool send (MessageHandler& target, const ValueTree& v)
    {
        //DBG ("Send: " << v.getType().toString());
        bool result = target.sendMessage (v);

        if (! result)
        {
            DBG ("*** Message failed: " << v.getType().toString());
        }

        return result;
    }

    inline bool sendPing (MessageHandler& target)
    {
        return send (target, ValueTree (PING));
    }

    //==============================================================================
    // client -> server

    inline void sendOpenPreview (MessageHandler& target, const ClassDatabase::Class& comp, Rectangle<int> mainWindowRect)
    {
        ValueTree v (OPEN_PREVIEW);
        v.setProperty (Ids::name, comp.getName(), nullptr);
        v.setProperty (Ids::bounds, rectToVar (mainWindowRect), nullptr);
        send (target, v);
    }

    inline void sendReinstantiate (MessageHandler& target)
    {
        send (target, ValueTree (RELOAD));
    }

    inline void sendFileChanges (MessageHandler& target, const Array<CodeChange>& changes, const File& file)
    {
        ValueTree changesMessage (MessageTypes::LIVE_FILE_CHANGES);
        changesMessage.setProperty (Ids::file, file.getFullPathName(), nullptr);

        for (const CodeChange& change : changes)
        {
            ValueTree v (CHANGE);
            v.setProperty (Ids::start, change.range.getStart(), nullptr);
            v.setProperty (Ids::end, change.range.getEnd(), nullptr);
            v.setProperty (Ids::text, change.text, nullptr);
            changesMessage.addChild (v, -1, nullptr);
        }

        send (target, changesMessage);
    }

    inline Array<CodeChange> getChangeArray (const ValueTree& changes)
    {
        Array<CodeChange> result;

        for (int i = 0; i < changes.getNumChildren(); ++i)
        {
            const ValueTree& v = changes.getChild (i);
            result.add (CodeChange (Range<int> (v[Ids::start], v[Ids::end]), v[Ids::text]));
        }

        return result;
    }

    inline void sendFileContentFullUpdate (MessageHandler& target, const File& file, const String& text)
    {
        ValueTree v (LIVE_FILE_UPDATE);
        v.setProperty (Ids::file, file.getFullPathName(), nullptr);
        v.setProperty (Ids::text, text, nullptr);
        send (target, v);
    }

    inline void sendHandleFileReset (MessageHandler& target, const File& file)
    {
        ValueTree v (LIVE_FILE_RESET);
        v.setProperty (Ids::file, file.getFullPathName(), nullptr);
        send (target, v);
    }

    inline void sendNewBuild (MessageHandler& target, const ProjectBuildInfo& build)
    {
        send (target, build.tree);
    }

    inline void sendCleanAll (MessageHandler& target)
    {
        send (target, ValueTree (CLEAN_ALL));
    }

    inline void sendNewDiagnosticList (MessageHandler& target, const ValueTree& list)
    {
        send (target, list);
    }

    inline void sendEmptyDiagnosticList (MessageHandler& target)
    {
        send (target, ValueTree (MessageTypes::DIAGNOSTIC_LIST));
    }

    inline void sendProcessActivationState (MessageHandler& target, bool isNowActive)
    {
        ValueTree v (FOREGROUND);
        v.setProperty (Ids::parentActive, isNowActive, nullptr);
        send (target, v);
    }

    inline void sendLaunchApp (MessageHandler& target)      { send (target, ValueTree (LAUNCH_APP)); }
    inline void sendQuit (MessageHandler& target)           { send (target, ValueTree (QUIT_SERVER)); }
    inline void sendShouldCloseIDE (MessageHandler& target) { send (target, ValueTree (QUIT_IDE)); }

    //==============================================================================
    // server -> client

    inline void sendNewClassList (MessageHandler& target, const ClassDatabase::ClassList& classes)
    {
        send (target, classes.toValueTree());
    }

    inline void sendCrash (MessageHandler& target, const String& message)
    {
        ValueTree v (CRASH);
        v.setProperty (Ids::message, message, nullptr);
        send (target, v);
    }

    inline void sendSystemHeadersMissing (MessageHandler& target)
    {
        send (target, ValueTree (MISSING_SYSTEM_HEADERS));
    }

    inline void sendBuildFailed (MessageHandler& target)
    {
        send (target, ValueTree (BUILD_FAILED));
    }

    inline void sendNewActivityList (MessageHandler& target, const StringArray& list)
    {
        ValueTree v (ACTIVITY_LIST);
        v.setProperty (Ids::list, concatenateListOfStrings (list), nullptr);
        send (target, v);
    }

    inline void sendChangeCode (MessageHandler& target, const String& location, const String& newText)
    {
        if (location.isNotEmpty())
        {
            ValueTree v (CHANGE_CODE);
            v.setProperty (Ids::position, location, nullptr);
            v.setProperty (Ids::text, newText, nullptr);
            send (target, v);
        }
    }

    inline void sendHighlightCode (MessageHandler& target, const String& location)
    {
        if (location.isNotEmpty())
        {
            ValueTree v (HIGHLIGHT_CODE);
            v.setProperty (Ids::position, location, nullptr);
            send (target, v);
        }
    }

    inline void sendAppLaunched (MessageHandler& target)    { send (target, ValueTree (LAUNCHED)); }
    inline void sendAppQuit (MessageHandler& target)        { send (target, ValueTree (APPQUIT)); }

    inline void sendKeyPress (MessageHandler& target, const String& className, const String& keyDesc)
    {
        ValueTree v (KEY);
        v.setProperty (Ids::class_, className, nullptr);
        v.setProperty (Ids::key, keyDesc, nullptr);
        send (target, v);
    }

    //==============================================================================
    template <class MessageHandlerType>
    static void dispatchToClient (MessageHandlerType& target, const ValueTree& v)
    {
        if (v.hasType (DIAGNOSTIC_LIST))             target.handleNewDiagnosticList (v);
        else if (v.hasType (ACTIVITY_LIST))          target.handleActivityListChanged (separateJoinedStrings (v [Ids::list]));
        else if (v.hasType (Ids::CLASSLIST))         target.handleClassListChanged (v);
        else if (v.hasType (BUILD_FAILED))           target.handleBuildFailed();
        else if (v.hasType (CHANGE_CODE))            target.handleChangeCode (v [Ids::position].toString(), v [Ids::text]);
        else if (v.hasType (HIGHLIGHT_CODE))         target.handleHighlightCode (v [Ids::position].toString());
        else if (v.hasType (LAUNCHED))               target.handleAppLaunched();
        else if (v.hasType (APPQUIT))                target.handleAppQuit();
        else if (v.hasType (PING))                   target.handlePing();
        else if (v.hasType (CRASH))                  target.handleCrash (v [Ids::message]);
        else if (v.hasType (KEY))                    target.handleKeyPress (v[Ids::class_], KeyPress::createFromDescription (v[Ids::key]));
        else if (v.hasType (QUIT_IDE))               target.handleCloseIDE();
        else if (v.hasType (MISSING_SYSTEM_HEADERS)) target.handleMissingSystemHeaders();
        else                                         jassertfalse;
    }

    template <class MessageHandlerType>
    static void dispatchToServer (MessageHandlerType& target, const ValueTree& v)
    {
        if (v.hasType (CLEAN_ALL))               target.handleCleanAll();
        else if (v.hasType (BUILDINFO))          target.handleNewBuildSettings (ProjectBuildInfo (v));
        else if (v.hasType (OPEN_PREVIEW))       target.handleOpenPreview (v[Ids::name], varToRect (v[Ids::bounds]));
        else if (v.hasType (RELOAD))             target.handleReinstantiatePreviews();
        else if (v.hasType (LAUNCH_APP))         target.handleLaunchApp();
        else if (v.hasType (LIVE_FILE_CHANGES))  target.handleLiveFileChanges (v[Ids::file].toString(), getChangeArray (v));
        else if (v.hasType (LIVE_FILE_UPDATE))   target.handleLiveFileFullUpdate (v[Ids::file].toString(), v[Ids::text]);
        else if (v.hasType (LIVE_FILE_RESET))    target.handleResetLiveFileContent (v[Ids::file].toString());
        else if (v.hasType (FOREGROUND))         target.handleProcessActivationState (v[Ids::parentActive]);
        else if (v.hasType (PING))               target.handlePing();
        else                                     jassertfalse;
    }
}
