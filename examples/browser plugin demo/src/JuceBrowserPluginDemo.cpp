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

#include "JuceHeader.h"


//==============================================================================
/**
    This is our top-level component for our plugin..
*/
class JuceDemoBrowserPlugin  : public BrowserPluginComponent,
                               public Button::Listener
{
public:
    JuceDemoBrowserPlugin()
        : textBox (String::empty),
          button ("Send a message to the webpage")
    {
        addAndMakeVisible (textBox);
        textBox.setMultiLine (true);
        textBox.setBounds (8, 8, 300, 300);

        addAndMakeVisible (button);
        button.setBounds (320, 8, 180, 22);
        button.addListener (this);
        button.setEnabled (false);

        ourJavascriptObject = new DemoBrowserObject (this);

        textBox.setText (SystemStats::getJUCEVersion() + "\n\n"
                          + "Browser: " + getBrowserVersion());
    }

    var getJavascriptObject()
    {
        // The browser calls this to get the javascript object that represents our plugin..
        return ourJavascriptObject;
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::lightblue);
    }

    void setJavascriptObjectFromBrowser (var callbackObject)
    {
        javascriptObjectFromBrowser = callbackObject;

        button.setEnabled (javascriptObjectFromBrowser.isObject());
    }

    void buttonClicked (Button*)
    {
        javascriptObjectFromBrowser.call ("printmessage", "This is a message sent from the plugin...");
    }

    var ourJavascriptObject;
    var javascriptObjectFromBrowser;
    TextEditor textBox;
    TextButton button;

    //==============================================================================
    /** This is the javascript object that the browser uses when the webpage accesses
        methods or properties on our plugin object.
    */
    class DemoBrowserObject : public DynamicObject
    {
    public:
        DemoBrowserObject (JuceDemoBrowserPlugin* bp)  : owner (bp)
        {
            // Add a couple of methods to our object..
            setMethod ("printText", printText);
            setMethod ("popUpMessageBox", popUpMessageBox);
            setMethod ("registerCallbackObject", registerCallbackObject);

            // Add some value properties that the webpage can access
            setProperty ("property1", "testing testing...");
            setProperty ("property2", 12345678.0);
        }

        //==============================================================================
        // These methods are called by javascript in the webpage...
        static var printText (const var::NativeFunctionArgs& args)
        {
            if (DemoBrowserObject* b = dynamic_cast<DemoBrowserObject*> (args.thisObject.getObject()))
                if (args.numArguments > 0)
                    b->owner->textBox.setText (b->owner->textBox.getText() + "\n" + args.arguments[0].toString());

            return "text was printed ok!";
        }

        static var popUpMessageBox (const var::NativeFunctionArgs& args)
        {
            if (DemoBrowserObject* b = dynamic_cast<DemoBrowserObject*> (args.thisObject.getObject()))
                if (args.numArguments > 0)
                    AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                                 "A message from the webpage",
                                                 args.arguments[0].toString(),
                                                 String::empty, b->owner);
            return var();
        }

        static var registerCallbackObject (const var::NativeFunctionArgs& args)
        {
            if (DemoBrowserObject* b = dynamic_cast<DemoBrowserObject*> (args.thisObject.getObject()))
                if (args.numArguments > 0)
                    b->owner->setJavascriptObjectFromBrowser (args.arguments[0]);

            return var();
        }

        //==============================================================================
        JuceDemoBrowserPlugin* owner;
    };
};

BrowserPluginComponent* JUCE_CALLTYPE createBrowserPlugin()
{
    return new JuceDemoBrowserPlugin();
}
