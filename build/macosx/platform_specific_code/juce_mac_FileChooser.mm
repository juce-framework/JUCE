/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

//==============================================================================
END_JUCE_NAMESPACE
using namespace JUCE_NAMESPACE;

#define JuceFileChooserDelegate MakeObjCClassName(JuceFileChooserDelegate)

@interface JuceFileChooserDelegate   : NSObject
{
    StringArray* filters;
}

- (JuceFileChooserDelegate*) initWithFilters: (StringArray*) filters_;
- (void) dealloc;
- (BOOL) panel: (id) sender shouldShowFilename: (NSString*) filename;

@end

@implementation JuceFileChooserDelegate
- (JuceFileChooserDelegate*) initWithFilters: (StringArray*) filters_
{
    [super init];
    filters = filters_;
    return self;
}

- (void) dealloc
{
    delete filters;
    [super dealloc];
}

- (BOOL) panel: (id) sender shouldShowFilename: (NSString*) filename
{
    const String fname (nsStringToJuce (filename));

    for (int i = filters->size(); --i >= 0;)
        if (fname.matchesWildcard ((*filters)[i], true))
            return true;

    return File (fname).isDirectory();
}
@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
void FileChooser::showPlatformDialog (OwnedArray<File>& results,
                                      const String& title,
                                      const File& currentFileOrDirectory,
                                      const String& filter,
                                      bool selectsDirectory,
                                      bool isSaveDialogue,
                                      bool warnAboutOverwritingExistingFiles,
                                      bool selectMultipleFiles,
                                      FilePreviewComponent* extraInfoComponent)
{
    const ScopedAutoReleasePool pool;

    StringArray* filters = new StringArray();
    filters->addTokens (filter.replaceCharacters (T(",:"), T(";;")), T(";"), 0);
    filters->trim();
    filters->removeEmptyStrings();

    JuceFileChooserDelegate* delegate = [[JuceFileChooserDelegate alloc] initWithFilters: filters];
    [delegate autorelease];

    NSSavePanel* panel = isSaveDialogue ? [NSSavePanel savePanel]
                                        : [NSOpenPanel openPanel];

    [panel setTitle: juceStringToNS (title)];

    if (! isSaveDialogue)
    {
        NSOpenPanel* openPanel = (NSOpenPanel*) panel;
        [openPanel setCanChooseDirectories: selectsDirectory];
        [openPanel setCanChooseFiles: ! selectsDirectory];
        [openPanel setAllowsMultipleSelection: selectMultipleFiles];
    }

    [panel setDelegate: delegate];

    String directory, filename;

    if (currentFileOrDirectory.isDirectory())
    {
        directory = currentFileOrDirectory.getFullPathName();
    }
    else
    {
        directory = currentFileOrDirectory.getParentDirectory().getFullPathName();
        filename = currentFileOrDirectory.getFileName();
    }

    if ([panel runModalForDirectory: juceStringToNS (directory)
                               file: juceStringToNS (filename)]
           == NSOKButton)
    {
        if (isSaveDialogue)
        {
            results.add (new File (nsStringToJuce ([panel filename])));
        }
        else
        {
            NSOpenPanel* openPanel = (NSOpenPanel*) panel;
            NSArray* urls = [openPanel filenames];
            for (unsigned int i = 0; i < [urls count]; ++i)
            {
                NSString* f = [urls objectAtIndex: i];
                results.add (new File (nsStringToJuce (f)));
            }
        }
    }

    [panel setDelegate: nil];
}

#endif
