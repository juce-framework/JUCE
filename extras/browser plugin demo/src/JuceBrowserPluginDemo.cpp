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
        addAndMakeVisible (&textBox);
        textBox.setMultiLine (true);
        textBox.setBounds (8, 8, 300, 300);

        addAndMakeVisible (&button);
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
        DemoBrowserObject (JuceDemoBrowserPlugin* owner_)
            : owner (owner_)
        {
            // Add a couple of methods to our object..
            setMethod ("printText", (var::MethodFunction) &DemoBrowserObject::printText);
            setMethod ("popUpMessageBox", (var::MethodFunction) &DemoBrowserObject::popUpMessageBox);
            setMethod ("registerCallbackObject", (var::MethodFunction) &DemoBrowserObject::registerCallbackObject);

            // Add some value properties that the webpage can access
            setProperty ("property1", "testing testing...");
            setProperty ("property2", 12345678.0);
        }

        //==============================================================================
        // These methods are called by javascript in the webpage...
        const var printText (const var* params, int numParams)
        {
            if (numParams > 0)
                owner->textBox.setText (owner->textBox.getText() + "\n" + params[0].toString());

            return "text was printed ok!";
        }

        const var popUpMessageBox (const var* params, int numParams)
        {
            if (numParams > 0)
                AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                             "A message from the webpage",
                                             params[0].toString(),
                                             String::empty, owner);
            return var::null;
        }

        const var registerCallbackObject (const var* params, int numParams)
        {
            if (numParams > 0)
                owner->setJavascriptObjectFromBrowser (params[0]);

            return var::null;
        }

        //==============================================================================
        JuceDemoBrowserPlugin* owner;
    };
};

BrowserPluginComponent* JUCE_CALLTYPE createBrowserPlugin()
{
    return new JuceDemoBrowserPlugin();
}
