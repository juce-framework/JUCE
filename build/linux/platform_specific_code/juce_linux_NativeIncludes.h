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

#ifndef __JUCE_LINUX_NATIVEINCLUDES_JUCEHEADER__
#define __JUCE_LINUX_NATIVEINCLUDES_JUCEHEADER__

/*
    This file wraps together all the linux-specific headers, so
    that we can include them all just once, and compile all our
    platform-specific stuff in one big lump, keeping it out of the
    way of the rest of the codebase.
*/

#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#include <sched.h>
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/ptrace.h>
#include <sys/vfs.h>
#include <sys/wait.h>
#include <fnmatch.h>
#include <utime.h>
#include <pwd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <sys/sysinfo.h>
#include <sys/file.h>
#include <signal.h>

/* Got a build error here? You'll need to install the freetype library...
   The name of the package to install is "libfreetype6-dev".
*/
#include <ft2build.h>
#include FT_FREETYPE_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#if JUCE_USE_XINERAMA
 /* If you're trying to use Xinerama, you'll need to install the "libxinerama-dev" package..  */
 #include <X11/extensions/Xinerama.h>
#endif

#if JUCE_USE_XSHM
 #include <X11/extensions/XShm.h>
 #include <sys/shm.h>
 #include <sys/ipc.h>
#endif

#if JUCE_OPENGL
 /* Got an include error here?

    If you want to install OpenGL support, the packages to get are "mesa-common-dev"
    and "freeglut3-dev".

    Alternatively, you can turn off the JUCE_OPENGL flag in juce_Config.h if you
    want to disable it.
 */
 #include <GL/glx.h>
#endif

#undef KeyPress

#if JUCE_ALSA
 /* Got an include error here? If so, you've either not got ALSA installed, or you've
    not got your paths set up correctly to find its header files.

    The package you need to install to get ASLA support is "libasound2-dev".

    If you don't have the ALSA library and don't want to build Juce with audio support,
    just disable the JUCE_ALSA flag in juce_Config.h
 */
 #include <alsa/asoundlib.h>
#endif

#undef SIZEOF

#endif   // __JUCE_LINUX_NATIVEINCLUDES_JUCEHEADER__
