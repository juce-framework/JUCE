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

#ifndef JUCE_LIVECONSTANTEDITOR_H_INCLUDED
#define JUCE_LIVECONSTANTEDITOR_H_INCLUDED

#if JUCE_ENABLE_LIVE_CONSTANT_EDITOR && ! DOXYGEN

//==============================================================================
/** You can safely ignore all the stuff in this namespace - it's a bunch of boilerplate
    code used to implement the JUCE_LIVE_CONSTANT functionality.
*/
namespace LiveConstantEditor
{
    int64 parseInt (String);
    double parseDouble (const String&);
    String intToString (int, bool preferHex);
    String intToString (int64, bool preferHex);

    template <typename Type>
    static void setFromString (Type& v,           const String& s)    { v = static_cast<Type> (s); }
    inline void setFromString (char& v,           const String& s)    { v = (char)           parseInt (s); }
    inline void setFromString (unsigned char& v,  const String& s)    { v = (unsigned char)  parseInt (s); }
    inline void setFromString (short& v,          const String& s)    { v = (short)          parseInt (s); }
    inline void setFromString (unsigned short& v, const String& s)    { v = (unsigned short) parseInt (s); }
    inline void setFromString (int& v,            const String& s)    { v = (int)            parseInt (s); }
    inline void setFromString (unsigned int& v,   const String& s)    { v = (unsigned int)   parseInt (s); }
    inline void setFromString (long& v,           const String& s)    { v = (long)           parseInt (s); }
    inline void setFromString (unsigned long& v,  const String& s)    { v = (unsigned long)  parseInt (s); }
    inline void setFromString (int64& v,          const String& s)    { v = (int64)          parseInt (s); }
    inline void setFromString (uint64& v,         const String& s)    { v = (uint64)         parseInt (s); }
    inline void setFromString (double& v,         const String& s)    { v = parseDouble (s); }
    inline void setFromString (float& v,          const String& s)    { v = (float) parseDouble (s); }
    inline void setFromString (String& v,         const String& s)    { v = s; }
    inline void setFromString (Colour& v,         const String& s)    { v = Colour ((int) parseInt (s)); }

    template <typename Type>
    inline String getAsString (const Type& v, bool)              { return String (v); }
    inline String getAsString (char v, bool preferHex)           { return intToString ((int) v, preferHex); }
    inline String getAsString (unsigned char v, bool preferHex)  { return intToString ((int) v, preferHex); }
    inline String getAsString (short v, bool preferHex)          { return intToString ((int) v, preferHex); }
    inline String getAsString (unsigned short v, bool preferHex) { return intToString ((int) v, preferHex); }
    inline String getAsString (int v, bool preferHex)            { return intToString ((int) v, preferHex); }
    inline String getAsString (unsigned int v, bool preferHex)   { return intToString ((int) v, preferHex); }
    inline String getAsString (int64 v, bool preferHex)          { return intToString ((int64) v, preferHex); }
    inline String getAsString (uint64 v, bool preferHex)         { return intToString ((int64) v, preferHex); }
    inline String getAsString (Colour v, bool)                   { return intToString ((int) v.getARGB(), true); }

    template <typename Type>
    inline String getAsCode (Type& v, bool preferHex)       { return getAsString (v, preferHex); }
    inline String getAsCode (Colour v, bool)                { return "Colour (0x" + String::toHexString ((int) v.getARGB()).paddedLeft ('0', 8) + ")"; }
    inline String getAsCode (const String& v, bool)         { return "\"" + v + "\""; }
    inline String getAsCode (const char* v, bool)           { return getAsCode (String (v), false); }

    template <typename Type>
    inline const char* castToCharPointer (const Type&)      { return ""; }
    inline const char* castToCharPointer (const String& s)  { return s.toRawUTF8(); }

    struct LivePropertyEditorBase;

    //==============================================================================
    struct JUCE_API  LiveValueBase
    {
        LiveValueBase (const char* file, int line);
        virtual ~LiveValueBase();

        virtual LivePropertyEditorBase* createPropertyComponent (CodeDocument&) = 0;
        virtual String getStringValue (bool preferHex) const = 0;
        virtual String getCodeValue (bool preferHex) const = 0;
        virtual void setStringValue (const String&) = 0;
        virtual String getOriginalStringValue (bool preferHex) const = 0;

        String name, sourceFile;
        int sourceLine;

        JUCE_DECLARE_NON_COPYABLE (LiveValueBase)
    };

    //==============================================================================
    struct JUCE_API  LivePropertyEditorBase  : public Component,
                                               private TextEditor::Listener,
                                               private ButtonListener
    {
        LivePropertyEditorBase (LiveValueBase&, CodeDocument&);

        void paint (Graphics&) override;
        void resized() override;
        void textEditorTextChanged (TextEditor&) override;
        void buttonClicked (Button*) override;

        void applyNewValue (const String&);
        void selectOriginalValue();
        void findOriginalValueInCode();

        LiveValueBase& value;
        Label name;
        TextEditor valueEditor;
        TextButton resetButton;
        CodeDocument& document;
        CPlusPlusCodeTokeniser tokeniser;
        CodeEditorComponent sourceEditor;
        CodeDocument::Position valueStart, valueEnd;
        ScopedPointer<Component> customComp;
        bool wasHex;

        JUCE_DECLARE_NON_COPYABLE (LivePropertyEditorBase)
    };

    //==============================================================================
    Component* createColourEditor (LivePropertyEditorBase&);
    Component* createIntegerSlider (LivePropertyEditorBase&);
    Component* createFloatSlider (LivePropertyEditorBase&);

    template <typename Type> struct CustomEditor    { static Component* create (LivePropertyEditorBase&)   { return nullptr; } };
    template<> struct CustomEditor<char>            { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<unsigned char>   { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<int>             { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<unsigned int>    { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<short>           { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<unsigned short>  { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<int64>           { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<uint64>          { static Component* create (LivePropertyEditorBase& e) { return createIntegerSlider (e); } };
    template<> struct CustomEditor<float>           { static Component* create (LivePropertyEditorBase& e) { return createFloatSlider (e); } };
    template<> struct CustomEditor<double>          { static Component* create (LivePropertyEditorBase& e) { return createFloatSlider (e); } };
    template<> struct CustomEditor<Colour>          { static Component* create (LivePropertyEditorBase& e) { return createColourEditor (e); } };

    template <typename Type>
    struct LivePropertyEditor  : public LivePropertyEditorBase
    {
        template <typename ValueType>
        LivePropertyEditor (ValueType& v, CodeDocument& d)  : LivePropertyEditorBase (v, d)
        {
            addAndMakeVisible (customComp = CustomEditor<Type>::create (*this));
        }
    };

    //==============================================================================
    template <typename Type>
    struct LiveValue  : public LiveValueBase
    {
        LiveValue (const char* file, int line, const Type& initialValue)
            : LiveValueBase (file, line), value (initialValue), originalValue (initialValue)
        {}

        operator Type() const noexcept   { return value; }
        operator const char*() const     { return castToCharPointer (value); }

        LivePropertyEditorBase* createPropertyComponent (CodeDocument& doc) override
        {
            return new LivePropertyEditor<Type> (*this, doc);
        }

        String getStringValue (bool preferHex) const override           { return getAsString (value, preferHex); }
        String getCodeValue (bool preferHex) const override             { return getAsCode (value, preferHex); }
        String getOriginalStringValue (bool preferHex) const override   { return getAsString (originalValue, preferHex); }
        void setStringValue (const String& s) override                  { setFromString (value, s); }

        Type value, originalValue;

        JUCE_DECLARE_NON_COPYABLE (LiveValue)
    };

    //==============================================================================
    class JUCE_API ValueList  : private AsyncUpdater,
                                private DeletedAtShutdown
    {
    public:
        ValueList();
        ~ValueList();

        static ValueList& getInstance();

        template <typename Type>
        LiveValue<Type>& getValue (const char* file, int line, const Type& initialValue)
        {
            const ScopedLock sl (lock);
            typedef LiveValue<Type> ValueType;

            for (int i = 0; i < values.size(); ++i)
            {
                LiveValueBase* v = values.getUnchecked(i);

                if (v->sourceLine == line && v->sourceFile == file)
                    return *static_cast<ValueType*> (v);
            }

            ValueType* v = new ValueType (file, line, initialValue);
            addValue (v);
            return *v;
        }

    private:
        OwnedArray<LiveValueBase> values;
        OwnedArray<CodeDocument> documents;
        Array<File> documentFiles;
        class EditorWindow;
        friend class EditorWindow;
        friend struct ContainerDeletePolicy<EditorWindow>;
        Component::SafePointer<EditorWindow> editorWindow;
        CriticalSection lock;

        CodeDocument& getDocument (const File&);
        void addValue (LiveValueBase*);
        void handleAsyncUpdate() override;
    };

    template <typename Type>
    inline LiveValue<Type>& getValue (const char* file, int line, const Type& initialValue)
    {
        return ValueList::getInstance().getValue (file, line, initialValue);
    }

    inline LiveValue<String>& getValue (const char* file, int line, const char* initialValue)
    {
        return getValue (file, line, String (initialValue));
    }
}

#endif

//==============================================================================
#if JUCE_ENABLE_LIVE_CONSTANT_EDITOR || DOXYGEN
 /**
    This macro wraps a primitive constant value in some cunning boilerplate code that allows
    its value to be interactively tweaked in a popup window while your application is running.

    In a release build, this macro disappears and is replaced by only the constant that it
    wraps, but if JUCE_ENABLE_LIVE_CONSTANT_EDITOR is enabled, it injects a class wrapper
    that automatically pops-up a window containing an editor that allows the value to be
    tweaked at run-time. The editor window will also force all visible components to be
    resized and repainted whenever a value is changed, so that if you use this to wrap
    a colour or layout parameter, you'll be able to immediately see the effects of changing it.

    The editor will also load the original source-file that contains each JUCE_LIVE_CONSTANT
    macro, and will display a preview of the modified source code as you adjust the values.

    Things to note:

    - Only one of these per line! The __FILE__ and __LINE__ macros are used to identify
      the value, so things will get confused if you have more than one per line
    - Obviously because it needs to load the source code based on the __FILE__ macro,
      it'll only work if the source files are stored locally in the same location as they
      were when you compiled the program.
    - It's only designed to cope with simple types: primitives, string literals, and
      the Colour class, so if you try using it for other classes or complex expressions,
      good luck!
    - The editor window will get popped up whenever a new value is used for the first
      time. You can close the window, but there's no way to get it back without restarting
      the app!

    e.g. in this example the colours, font size, and text used in the paint method can
    all be adjusted live:
    @code
    void MyComp::paint (Graphics& g) override
    {
        g.fillAll (JUCE_LIVE_CONSTANT (Colour (0xffddddff)));

        Colour fontColour = JUCE_LIVE_CONSTANT (Colour (0xff005500));
        float fontSize = JUCE_LIVE_CONSTANT (16.0f);

        g.setColour (fontColour);
        g.setFont (fontSize);

        g.drawFittedText (JUCE_LIVE_CONSTANT ("Hello world!"),
                          getLocalBounds(), Justification::centred, 2);
    }
    @endcode
 */
 #define JUCE_LIVE_CONSTANT(initialValue) \
    (juce::LiveConstantEditor::getValue (__FILE__, __LINE__ - 1, initialValue))
#else
 #define JUCE_LIVE_CONSTANT(initialValue) \
    (initialValue)
#endif


#endif   // JUCE_LIVECONSTANTEDITOR_H_INCLUDED
