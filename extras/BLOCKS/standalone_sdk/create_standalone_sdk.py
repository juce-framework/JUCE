# Run this script in an empty directory to create the contents of the
# standalone SDK repository.

import os
import shutil

script_dir = os.path.dirname(os.path.realpath(__file__))

# Get the list of JUCE modules to include.
juce_modules = []
with open(os.path.join(script_dir, "..", "juce_modules.txt"), "r") as f:
    for line in f:
        juce_modules.append(line.strip())

# Copy the required modules into the SDK dir.
sdk_dir = "SDK"
shutil.copytree(os.path.join(script_dir, sdk_dir), sdk_dir)
for module_name in juce_modules:
    shutil.copytree(os.path.join(script_dir, "..", "..", "..", "modules",
                                 module_name),
                    os.path.join(sdk_dir, module_name))

# Copy the examples.
shutil.copytree(os.path.join(script_dir, "examples"), "examples")

# Copy the README.
shutil.copyfile(os.path.join(script_dir, "README.md"), "README.md")
