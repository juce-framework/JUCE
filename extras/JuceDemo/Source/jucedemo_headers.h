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

#ifndef __JUCEDEMO_HEADERS_JUCEHEADER__
#define __JUCEDEMO_HEADERS_JUCEHEADER__

// include the JUCE headers..
#include "../JuceLibraryCode/JuceHeader.h"

#if JUCE_IOS || JUCE_LINUX || JUCE_ANDROID
 #undef JUCE_USE_CAMERA
#endif

// Pre-declare the functions that create each of the demo components..
Component* createFontsAndTextDemo();
Component* createRenderingDemo();
Component* createWidgetsDemo();
Component* createThreadingDemo();
Component* createTreeViewDemo();
Component* createTableDemo();
Component* createAudioDemo();
Component* createDragAndDropDemo();
Component* createInterprocessCommsDemo();
Component* createCodeEditorDemo();

#if JUCE_QUICKTIME && ! JUCE_LINUX
 Component* createQuickTimeDemo();
#endif

#if JUCE_OPENGL
 Component* createOpenGLDemo();
#endif

#if JUCE_WEB_BROWSER
  Component* createWebBrowserDemo();
#endif

#if JUCE_USE_CAMERA
  Component* createCameraDemo();
#endif

#if JUCE_DIRECTSHOW
  Component* createDirectShowDemo();
#endif

#endif   // __JUCEDEMO_HEADERS_JUCEHEADER__
