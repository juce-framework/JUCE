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


struct OpenGLAppWizard   : public NewProjectWizard
{
    OpenGLAppWizard()  {}

    String getName() const override         { return TRANS("OpenGL Application"); }
    String getDescription() const override  { return TRANS("Creates a blank JUCE application with a single window component. This component supports openGL drawing features including 3D model import and GLSL shaders."); }
    const char* getIcon() const override    { return BinaryData::wizard_OpenGL_svg; }

    bool initialiseProject (Project& project) override
    {
        createSourceFolder();
        project.getProjectTypeValue() = ProjectType::getGUIAppTypeName();
        createSourceGroup (project);
        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        return true;
    }
};

