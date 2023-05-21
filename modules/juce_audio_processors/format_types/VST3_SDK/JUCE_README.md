This list details modifications made to the VST3 SDK in order to facilitate
inclusion in JUCE.

- `#warning` directives were removed from fstring.cpp, as these cannot be
  silenced with a `pragma GCC diagnostic ignored "-Wcpp"` when building with
  g++.

- The version check in module_linux.cpp was changed to match the number
  corresponding to the C++17 standard.

- The <limits> header was included in moduleinfoparser.cpp in order to make
  std::numeric_limits visible when building with the GNU stdlib.

- Loop variable types were adjusted in moduleinfoparser.cpp to avoid forming
  references to temporary values, which produced -Wrange-loop-bind-reference
  warnings when building with Xcode.

- The <cstdint> header was included in moduleinfo.h in order to make uint32_t
  visible when building with the GNU stdlib.

- helper.manifest was added, to be included in the moduleinfo tool in order to
  force UTF-8 mode on Windows.

- std:: qualification was added to std::move call in module.cpp to silence
  a -Wunqualified-std-cast-call warning.

- The main.cpp of moduleinfotool was updated to include information exported
  by the plugin's IPluginCompatibility object, if present.

- Preprocessor definitions that expanded to include the keyword 'defined' were
  removed in fplatform.h to silence -Wexpansion-to-defined warnings.

- Pragma warning was guarded in falignpush.h to silence -Wunknown-pragma
  warnings.
