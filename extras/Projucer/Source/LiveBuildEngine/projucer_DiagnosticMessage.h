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

struct DiagnosticMessage
{
    DiagnosticMessage() = default;

    DiagnosticMessage (const DiagnosticMessage& other)
       : associatedDiagnostic (createCopyIfNotNull (other.associatedDiagnostic.get())),
         message (other.message),
         mainFile (other.mainFile),
         range (other.range),
         type (other.type)
    {
    }

    DiagnosticMessage& operator= (const DiagnosticMessage& other)
    {
        associatedDiagnostic = createCopyIfNotNull (other.associatedDiagnostic.get());
        message = other.message;
        mainFile = other.mainFile;
        range = other.range;
        type = other.type;

        return *this;
    }

    enum Type
    {
        error = 0,
        warning = 1,
        note = 2
    };

    ScopedPointer<DiagnosticMessage> associatedDiagnostic;
    String message;
    String mainFile;
    SourceCodeRange range;
    Type type;

    bool isError() const noexcept       { return type == error; }
    bool isWarning() const noexcept     { return type == warning; }
    bool isNote() const noexcept        { return type == note; }

    String toString() const
    {
        // todo: copy recursively from root
        String res;

        switch (type)
        {
            case error:   res << "error: "; break;
            case warning: res << "warning: "; break;
            case note:    res << "note: "; break;
        };

        res << range.file << ": ";
        res << message << "\n";

        return res;
    }

    ValueTree toValueTree() const
    {
        ValueTree v (MessageTypes::DIAGNOSTIC);
        v.setProperty (Ids::text, message, nullptr);
        v.setProperty (Ids::file, mainFile, nullptr);
        v.setProperty (Ids::range, range.toString(), nullptr);
        v.setProperty (Ids::type, (int) type, nullptr);

        if (associatedDiagnostic != nullptr)
            v.addChild (associatedDiagnostic->toValueTree(), 0, nullptr);

        return v;
    }

    static DiagnosticMessage fromValueTree (const ValueTree& v)
    {
        DiagnosticMessage d;
        d.message = v[Ids::text];
        d.mainFile = v[Ids::file];
        d.range = SourceCodeRange (v [Ids::range]);
        d.type = (Type) static_cast<int> (v[Ids::type]);

        auto associated = v.getChild (0);
        if (associated.isValid())
            d.associatedDiagnostic = new DiagnosticMessage (fromValueTree (associated));

        return d;
    }

    bool operator== (const DiagnosticMessage& other) const noexcept
    {
        return range == other.range
                && message == other.message
                && mainFile == other.mainFile;
    }

    bool operator!= (const DiagnosticMessage& other) const noexcept    { return ! operator== (other); }
};

//==============================================================================
struct DiagnosticReceiver
{
    virtual ~DiagnosticReceiver() {}
    virtual void handleDiagnostic (const DiagnosticMessage&) = 0;
    virtual void handleRecoverableErrorPCH (const DiagnosticMessage& m, String pchFileName, String sourceFileName) = 0;
};

//==============================================================================
struct DiagnosticList
{
    // after some research, it seems that notes never come on their own
    // i.e. they always have a warning / error preceding them
    // so we can keep notes and their associated diagnostics
    // together by keeping track of the last message
    DiagnosticMessage lastMessage;

    ValueTree list { MessageTypes::DIAGNOSTIC_LIST };

    void clear()
    {
        list = ValueTree { MessageTypes::DIAGNOSTIC_LIST };
        lastMessage = DiagnosticMessage();
    }

    void add (DiagnosticMessage m)
    {
        if (m.isNote())
        {
            if (lastMessage.message.isEmpty())
                return; // seems to happen sometimes, but with seemingly duplicated messages (?)

            m.associatedDiagnostic = new DiagnosticMessage (lastMessage);
        }
        else
        {
            lastMessage = m;
        }

        list.addChild (m.toValueTree(), -1, nullptr);
    }

    void add (const DiagnosticList& l)
    {
        jassert (l.list != list);

        for (int i = 0; i < l.list.getNumChildren(); ++i)
            list.addChild (l.list.getChild(i).createCopy(), -1, nullptr);
    }

    void remove (DiagnosticMessage m)
    {
        auto n = m.toValueTree();

        for (int i = 0; i < list.getNumChildren(); ++i)
        {
            if (list.getChild (i).isEquivalentTo (n))
            {
                list.removeChild (i, nullptr);
                return;
            }
        }

        jassertfalse;
    }

    bool hasRecoveryWarning (DiagnosticMessage m) const
    {
        auto n = m.toValueTree();

        for (int i = 0; i < list.getNumChildren(); ++i)
            if (list.getChild (i).isEquivalentTo (n))
                return true;

        return false;
    }

    const ValueTree& toValueTree() const noexcept
    {
        return list;
    }

    void loadFromChildOfValueTree (ValueTree& parent)
    {
        list = parent.getChildWithName (MessageTypes::DIAGNOSTIC_LIST).createCopy();
    }
};
