This list details modifications made to the VST3 SDK in order to facilitate
inclusion in JUCE.

- The main.cpp of moduleinfotool was updated to include information exported
  by the plugin's IPluginCompatibility object, if present.
