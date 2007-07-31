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

#ifndef __JUCEDEMO_HEADERS_JUCEHEADER__
#define __JUCEDEMO_HEADERS_JUCEHEADER__


// include the JUCE headers..
#include "../../../juce.h"

// this declares the binary resources that we're building into the
// application - i.e. images, sounds, etc that the demos use
#include "BinaryData.h"

// Pre-declare the functions that create each of the demo components..
Component* createFontsAndTextDemo();
Component* createPathsAndTransformsDemo();
Component* createWidgetsDemo (ApplicationCommandManager* commandManager);
Component* createThreadingDemo();
Component* createTreeViewDemo();
Component* createTableDemo();
Component* createAudioDemo();
Component* createDragAndDropDemo();
Component* createInterprocessCommsDemo();

#if JUCE_QUICKTIME && ! JUCE_LINUX
 Component* createQuickTimeDemo();
#endif

#if JUCE_OPENGL
 Component* createOpenGLDemo();
#endif

#endif   // __JUCEDEMO_HEADERS_JUCEHEADER__
