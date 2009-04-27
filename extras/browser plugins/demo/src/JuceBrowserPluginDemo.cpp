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

#include "../../wrapper/juce_BrowserPluginComponent.h"


//==============================================================================
/**
    This is our top-level component for our plugin..
*/
class JuceDemoBrowserPlugin  : public BrowserPluginComponent, 
                               public ButtonListener
{
public:
    JuceDemoBrowserPlugin()
    {
        addAndMakeVisible (textBox = new TextEditor (String::empty));
        textBox->setMultiLine (true);
        textBox->setBounds (8, 8, 300, 300);

        addAndMakeVisible (button = new TextButton ("Send a message to the webpage"));
        button->setBounds (320, 8, 180, 22);
        button->addButtonListener (this);
        button->setEnabled (false);

        ourJavascriptObject = new DemoBrowserObject (this);

        textBox->setText ("Browser version info: " + getBrowserVersion());
    }

    ~JuceDemoBrowserPlugin()
    {
        deleteAllChildren();
    }

    const var getJavascriptObject()
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
        
        button->setEnabled (javascriptObjectFromBrowser.isObject());
    }

    void buttonClicked (Button*)
    {
        javascriptObjectFromBrowser.call ("printmessage", "This is a message sent from the plugin...");
    }

    var ourJavascriptObject;
    var javascriptObjectFromBrowser;
    TextEditor* textBox;
    TextButton* button;

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

        DemoBrowserObject()
        {
        }

        //==============================================================================
        // These methods are called by javascript in the webpage...
        const var printText (const var* params, int numParams)
        {
            if (numParams > 0)
                owner->textBox->setText (owner->textBox->getText() + "\n" + params[0].toString());

            return var();
        }

        const var popUpMessageBox (const var* params, int numParams)
        {
            if (numParams > 0)
                AlertWindow::showMessageBox (AlertWindow::InfoIcon, 
                                             "A message from the webpage", 
                                             params[0].toString(), 
                                             String::empty, owner);
            return var();
        }

        const var registerCallbackObject (const var* params, int numParams)
        {
            if (numParams > 0)
                owner->setJavascriptObjectFromBrowser (params[0]);

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
