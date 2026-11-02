# JUCE BinaryBuilder

A command-line utility that encodes binary files into C++ source files, compiled directly into your application — no file-system access required at runtime.

---

## Usage

```
BinaryBuilder <inputDir> <outputDir> <namespace> [--split]
```

| Argument | Description |
|---|---|
| `<inputDir>` | Directory containing the files to encode |
| `<outputDir>` | Directory where the generated C++ files will be written |
| `<namespace>` | C++ namespace name (e.g. `BinaryData`) |
| `--split` | *(Optional)* Write each resource into its own `.cpp` file |

---

## Default

```bash
"/JUCE/extras/BinaryBuilder/Builds/MacOSX/build/Debug/BinaryBuilder" \
"/Users/Desktop/DATA" \
"/Users/Desktop/OUTPUT" \
"BinaryData"
```

Outputs a single `BinaryData.h` / `BinaryData.cpp` pair.

---

## Split Mode

```bash
"/JUCE/extras/BinaryBuilder/Builds/MacOSX/build/Debug/BinaryBuilder" \
"/Users/Desktop/DATA" \
"/Users/Desktop/OUTPUT" \
"BinaryData" \
--split
```

Outputs a separate `.cpp` file per resource. Useful for large asset sets where a single `.cpp` would cause slow compile times.

```
OUTPUT/
├── BinaryData.h
├── BinaryData1.cpp    ← lookup table
├── BinaryData2.cpp    ← resource: file_001
├── BinaryData3.cpp    ← resource: file_002
└── ...
```

> All generated `.cpp` files must be added to your project.

---

## Runtime Access

```cpp
#include "BinaryData.h"

// By name
int   size = 0;
auto* data = BinaryData::getNamedResource ("mySound_wav", size);

// By symbol
auto* raw     = BinaryData::mySound_wav;
int   rawSize = BinaryData::mySound_wavSize;
```

Filenames are converted to valid C++ identifiers — non-alphanumeric characters become underscores:

| File | Symbol |
|---|---|
| `click.wav` | `BinaryData::click_wav` |
| `background.png` | `BinaryData::background_png` |
| `my-font.ttf` | `BinaryData::my_font_ttf` |
