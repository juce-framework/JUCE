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

#ifndef __JUCER_TRANSLATIONTOOL_JUCEHEADER__
#define __JUCER_TRANSLATIONTOOL_JUCEHEADER__


struct TranslationHelpers
{
    static void addString (StringArray& strings, const String& s)
    {
        if (s.isNotEmpty() && ! strings.contains (s))
            strings.add (s);
    }

    static void scanFileForTranslations (StringArray& strings, const File& file)
    {
        const String content (file.loadFileAsString());

        String::CharPointerType p (content.getCharPointer());

        for (;;)
        {
            p = CharacterFunctions::find (p, CharPointer_ASCII ("TRANS"));

            if (p.isEmpty())
                break;

            p += 5;
            p = p.findEndOfWhitespace();

            if (*p == '(')
            {
                ++p;
                MemoryOutputStream text;
                parseStringLiteral (p, text);

                addString (strings, text.toString());
            }
        }
    }

    static void parseStringLiteral (String::CharPointerType& p, MemoryOutputStream& out) noexcept
    {
        p = p.findEndOfWhitespace();

        if (p.getAndAdvance() == '"')
        {
            String::CharPointerType start (p);

            for (;;)
            {
                juce_wchar c = *p;

                if (c == '"')
                {
                    out << String (start, p);
                    parseStringLiteral (++p, out);
                    return;
                }

                if (c == 0)
                    break;

                if (c == '\\')
                {
                    out << String (start, p);
                    ++p;
                    out << String::charToString (readEscapedChar (p));
                    start = p + 1;
                }

                ++p;
            }
        }
    }

    static juce_wchar readEscapedChar (String::CharPointerType& p)
    {
        juce_wchar c = *p;

        switch (c)
        {
            case '"':
            case '\\':
            case '/':  break;

            case 'b':  c = '\b'; break;
            case 'f':  c = '\f'; break;
            case 'n':  c = '\n'; break;
            case 'r':  c = '\r'; break;
            case 't':  c = '\t'; break;

            case 'x':
                ++p;
                c = 0;

                for (int i = 4; --i >= 0;)
                {
                    const int digitValue = CharacterFunctions::getHexDigitValue (*p);
                    if (digitValue < 0)
                        break;

                    ++p;
                    c = (juce_wchar) ((c << 4) + digitValue);
                }

                break;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                c = 0;

                for (int i = 4; --i >= 0;)
                {
                    const int digitValue = *p - '0';
                    if (digitValue < 0 || digitValue > 7)
                        break;

                    ++p;
                    c = (juce_wchar) ((c << 3) + digitValue);
                }

                break;

            default:
                break;
        }

        return c;
    }

    static void scanFilesForTranslations (StringArray& strings, const Project::Item& p)
    {
        if (p.isFile())
        {
            const File file (p.getFile());

            if (file.hasFileExtension (sourceOrHeaderFileExtensions))
                scanFileForTranslations (strings, file);
        }

        for (int i = 0; i < p.getNumChildren(); ++i)
            scanFilesForTranslations (strings, p.getChild (i));
    }

    static void scanProject (StringArray& strings, Project& project)
    {
        scanFilesForTranslations (strings, project.getMainGroup());

        const File modulesFolder (ModuleList::getDefaultModulesFolder (&project));

        OwnedArray<LibraryModule> modules;
        ModuleList moduleList;
        moduleList.rescan (modulesFolder);
        project.createRequiredModules (moduleList, modules);

        for (int j = 0; j < modules.size(); ++j)
        {
            const File localFolder (modules.getUnchecked(j)->getLocalFolderFor (project));

            Array<File> files;
            modules.getUnchecked(j)->findBrowseableFiles (localFolder, files);

            for (int i = 0; i < files.size(); ++i)
                scanFileForTranslations (strings, files.getReference(i));
        }
    }

    static const char* getMungingSeparator()  { return "JCTRIDX"; }

    static StringArray breakApart (const String& munged)
    {
        StringArray lines, result;
        lines.addLines (munged);

        String currentItem;

        for (int i = 0; i < lines.size(); ++i)
        {
            if (lines[i].contains (getMungingSeparator()))
            {
                if (currentItem.isNotEmpty())
                    result.add (currentItem);

                currentItem = String::empty;
            }
            else
            {
                if (currentItem.isNotEmpty())
                    currentItem << newLine;

                currentItem << lines[i];
            }
        }

        if (currentItem.isNotEmpty())
            result.add (currentItem);

        return result;
    }

    static String escapeString (const String& s)
    {
        return s.replace ("\"", "\\\"")
                .replace ("\'", "\\\'")
                .replace ("\t", "\\t")
                .replace ("\r", "\\r")
                .replace ("\n", "\\n");
    }

    static String getPreTranslationText (Project& project)
    {
        StringArray strings;
        scanProject (strings, project);
        return mungeStrings (strings);
    }

    static String getPreTranslationText (const LocalisedStrings& strings)
    {
        return mungeStrings (strings.getMappings().getAllKeys());
    }

    static String mungeStrings (const StringArray& strings)
    {
        MemoryOutputStream s;

        for (int i = 0; i < strings.size(); ++i)
        {
            s << getMungingSeparator() << i << "." << newLine << strings[i];

            if (i < strings.size() - 1)
                s << newLine;
        }

        return s.toString();
    }

    static String createFinishedTranslationFile (const StringArray& preStrings,
                                                 const StringArray& postStrings)
    {
        StringArray lines;

        lines.add ("language: [enter full name of the language here!]");
        lines.add ("countries: [enter list of 2-character country codes here!]");
        lines.add (String::empty);

        for (int i = 0; i < preStrings.size(); ++i)
            lines.add ("\"" + escapeString (preStrings[i])
                         + "\" = \""
                         + escapeString (postStrings[i]) + "\"");

        return lines.joinIntoString (newLine);
    }
};

//==============================================================================
class TranslationToolComponent  : public Component,
                                  public ButtonListener
{
public:
    TranslationToolComponent()
        : editorPre (documentPre, nullptr),
          editorPost (documentPost, nullptr),
          editorResult (documentResult, nullptr)
    {
        setLookAndFeel (&lf);

        instructionsLabel.setText (
            "This utility converts translation files to/from a format that can be passed to automatic translation tools."
            "\n\n"
            "First, choose whether to scan the current project for all TRANS() macros, or "
            "pick an existing translation file to load:", dontSendNotification);
        addAndMakeVisible (&instructionsLabel);

        label1.setText ("..then copy-and-paste this annotated text into Google Translate or some other translator:", dontSendNotification);
        addAndMakeVisible (&label1);

        label2.setText ("...then, take the translated result and paste it into the box below:", dontSendNotification);
        addAndMakeVisible (&label2);

        label3.setText ("Finally, click the 'Generate' button, and a translation file will be created below. "
                        "Remember to update its language code at the top!", dontSendNotification);
        addAndMakeVisible (&label3);

        addAndMakeVisible (&editorPre);
        addAndMakeVisible (&editorPost);
        addAndMakeVisible (&editorResult);

        generateButton.setButtonText (TRANS("Generate"));
        addAndMakeVisible (&generateButton);
        scanButton.setButtonText ("Scan Project for TRANS macros");
        addAndMakeVisible (&scanButton);
        loadButton.setButtonText ("Load existing translation File...");
        addAndMakeVisible (&loadButton);
        generateButton.addListener (this);

        scanButton.addListener (this);
        loadButton.addListener (this);
    }

    void paint (Graphics& g)
    {
        IntrojucerLookAndFeel::fillWithBackgroundTexture (*this, g);
    }

    void resized()
    {
        Rectangle<int> r (getLocalBounds());

        r.removeFromTop (120);

        editorPre.setBounds (10, 165, getWidth() - 20, 130);
        editorPost.setBounds (10, 338, getWidth() - 20, 114);
        editorResult.setBounds (10, 503, getWidth() - 20, getHeight() - 510);

        generateButton.setBounds (getWidth() - 152, 462, 140, 30);
        label1.setBounds (10, 128, getWidth() - 20, 26);
        label2.setBounds (10, 303, getWidth() - 20, 25);
        label3.setBounds (10, 459, generateButton.getX() - 20, 38);
        instructionsLabel.setBounds (6, 10, getWidth() - 14, 70);
        scanButton.setBounds (27, 86, 257, 30);
        loadButton.setBounds (304, 86, 260, 30);
    }

private:
    CodeDocument documentPre, documentPost, documentResult;
    CodeEditorComponent editorPre, editorPost, editorResult;
    juce::Label label1, label2, label3;
    juce::TextButton generateButton;
    juce::Label instructionsLabel;
    juce::TextButton scanButton;
    juce::TextButton loadButton;

    IntrojucerLookAndFeel lf;

    void buttonClicked (Button* b)
    {
        if (b == &generateButton)   generate();
        else if (b == &loadButton)  loadFile();
        else if (b == &scanButton)  scanProject();
    }

    void generate()
    {
        StringArray preStrings  (TranslationHelpers::breakApart (documentPre.getAllContent()));
        StringArray postStrings (TranslationHelpers::breakApart (documentPost.getAllContent()));

        if (postStrings.size() != preStrings.size())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              TRANS("Error"),
                                              TRANS("The pre- and post-translation text doesn't match!\n\n"
                                                    "Perhaps it got mangled by the translator?"));
            return;
        }

        documentResult.replaceAllContent (TranslationHelpers::createFinishedTranslationFile (preStrings, postStrings));
    }

    void loadFile()
    {
        FileChooser fc ("Choose a translation file to load",
                        File::nonexistent,
                        "*");

        if (fc.browseForFileToOpen())
            setPreTranslationText (TranslationHelpers::getPreTranslationText (LocalisedStrings (fc.getResult(), false)));
    }

    void scanProject()
    {
        if (Project* project = IntrojucerApp::getApp().mainWindowList.getFrontmostProject())
            setPreTranslationText (TranslationHelpers::getPreTranslationText (*project));
        else
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Translation Tool",
                                              "This will only work when you have a project open!");
    }

    void setPreTranslationText (const String& text)
    {
        documentPre.replaceAllContent (text);
        editorPre.grabKeyboardFocus();
        editorPre.selectAll();
    }
};


#endif   // __JUCER_TRANSLATIONTOOL_JUCEHEADER__
