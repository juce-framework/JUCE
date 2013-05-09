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

#ifndef __JUCER_TRANSLATIONTOOL_H_E4B7E119__
#define __JUCER_TRANSLATIONTOOL_H_E4B7E119__


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

        MemoryOutputStream s;
        for (int i = 0; i < strings.size(); ++i)
        {
            s << getMungingSeparator() << i << "." << newLine << strings[i];

            if (i < strings.size() - 1)
                s << newLine;
        }

        return s.toString();
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

        instructionsLabel.setText ("This tool scans your project for all TRANS() macros, and "
                                   "assembles them into a blob of text that can be automatically "
                                   "translated and disassembled into a JUCE translation file...", dontSendNotification);
        addAndMakeVisible (&instructionsLabel);

        label1.setText ("Copy-and-paste this text into Google Translate or some other translator:", dontSendNotification);
        addAndMakeVisible (&label1);

        label2.setText ("...then, take the translated result and paste it into this box:", dontSendNotification);
        addAndMakeVisible (&label2);

        label3.setText ("Finally: Click the 'Generate' button, and a valid translation file will be created below...", dontSendNotification);
        addAndMakeVisible (&label3);

        addAndMakeVisible (&editorPre);
        addAndMakeVisible (&editorPost);
        addAndMakeVisible (&editorResult);

        generateButton.setButtonText (TRANS("Generate"));
        addAndMakeVisible (&generateButton);
        generateButton.addListener (this);
    }

    void paint (Graphics& g)
    {
        IntrojucerLookAndFeel::fillWithBackgroundTexture (*this, g);
    }

    void resized()
    {
        Rectangle<int> r (getLocalBounds());

        r.removeFromTop (120);

        editorPre.setBounds (10, 95, getWidth() - 20, 130);
        editorPost.setBounds (10, 271, getWidth() - 20, 114);
        editorResult.setBounds (10, 470, getWidth() - 20, getHeight() - 480);

        generateButton.setBounds (getWidth() - 150, 413, 140, 30);
        label1.setBounds (6, 58, getWidth() - 12, 26);
        label2.setBounds (6, 238, getWidth() - 12, 25);
        label3.setBounds (6, 402, generateButton.getX() - 20, 54);
        instructionsLabel.setBounds (6, 10, getWidth() - 12, 44);
    }

    void initialiseForProject (Project& project)
    {
        documentPre.replaceAllContent (TranslationHelpers::getPreTranslationText (project));
        editorPre.selectAll();
    }

private:
    CodeDocument documentPre, documentPost, documentResult;
    CodeEditorComponent editorPre, editorPost, editorResult;
    juce::Label label1, label2, label3;
    juce::TextButton generateButton;
    juce::Label instructionsLabel;

    IntrojucerLookAndFeel lf;

    void buttonClicked (Button*)
    {
        StringArray mappings;

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

        for (int i = 0; i < preStrings.size(); ++i)
            mappings.add (TranslationHelpers::escapeString (preStrings[i]).quoted()
                           + " = "
                           + TranslationHelpers::escapeString (postStrings[i]).quoted());

        documentResult.replaceAllContent (mappings.joinIntoString (newLine));
    }
};


#endif  // __JUCER_TRANSLATIONTOOL_H_E4B7E119__
