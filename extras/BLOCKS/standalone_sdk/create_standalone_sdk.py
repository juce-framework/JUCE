import os
import shutil

script_dir = os.path.dirname(os.path.realpath(__file__))

# Get the list of JUCE modules to include.
juce_modules = []
with open(os.path.join(script_dir, "..", "juce_modules.txt"), "r") as f:
    for line in f:
        juce_modules.append(line.strip())

# Make sure we are starting afresh.
out_directory = "BLOCKS-SDK"
try:
    shutil.rmtree(out_directory)
except OSError as e:
    if e.errno != 2:
        # An errno of 2 indicates that the directory does not exist, which is
        # fine!
        raise e

# Copy the required modules into the SDK dir.
sdk_dir = os.path.join(out_directory, "SDK")
shutil.copytree(os.path.join(script_dir, "SDK"), sdk_dir)
for module_name in juce_modules:
    shutil.copytree(os.path.join(script_dir, "..", "..", "..", "modules",
                                 module_name),
                    os.path.join(sdk_dir, module_name))

# Copy the examples.
shutil.copytree(os.path.join(script_dir, "examples"),
                os.path.join(out_directory, "examples"))

# Copy the README.
shutil.copyfile(os.path.join(script_dir, "README.md"),
                os.path.join(out_directory, "README.md"))
