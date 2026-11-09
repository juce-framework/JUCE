#!/usr/bin/env python3
"""Vendors libogg into the directory containing this script.

To update libogg, bump VERSION and run this script. It replaces everything in
this directory except the script itself with a fresh copy of the upstream
release, rewrites the sources' includes for the JUCE include model (no -I
directories), and generates the config_types.h that upstream's
configure/cmake run would produce. Needs git and Python 3.10+.
"""

import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

VERSION = "1.3.6"
REPO = "https://github.com/xiph/ogg.git"

# Copied verbatim from the upstream tree.
FILES = [
    "COPYING",
    "include/ogg/ogg.h",
    "include/ogg/os_types.h",
    "src/bitwise.c",
    "src/crctable.h",
    "src/framing.c",
]

# Substituted into upstream's include/ogg/config_types.h.in.
CONFIG_TYPES = {
    "INCLUDE_INTTYPES_H": "0",
    "INCLUDE_STDINT_H": "1",
    "INCLUDE_SYS_TYPES_H": "0",
    "SIZE16": "int16_t",
    "USIZE16": "uint16_t",
    "SIZE32": "int32_t",
    "USIZE32": "uint32_t",
    "SIZE64": "int64_t",
    "USIZE64": "uint64_t",
}

UPSTREAM_NOTE = """Upstream: {repo}
Version:  {version} (tag v{version}, commit {commit})

Vendored by vendor.py in this directory; rerun it to update. It copies the
files listed in it verbatim, rewrites their '#include <ogg/...>' lines as
paths relative to the including file (JUCE modules have no include
directories), and generates include/ogg/config_types.h from upstream's
config_types.h.in using the <stdint.h> types.
"""


# Interprets the provided path constituents relative to the location of this
# script, and returns an absolute Path to the resulting location.
#
# E.g. rel_to_py(".") returns an absolute path to the directory containing this
# script.
def rel_to_py(*paths) -> Path:
    return Path(
        os.path.realpath(
            os.path.join(os.path.realpath(os.path.dirname(__file__)), *paths)
        )
    )


def git(*args) -> str:
    return subprocess.run(
        ["git", *args], check=True, capture_output=True, text=True
    ).stdout.strip()


def write(path: Path, text: str):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, newline="\n")


def main():
    dest = rel_to_py(".")

    for stale in ("include", "src", "COPYING", "JUCE_UPSTREAM.txt"):
        path = dest / stale
        shutil.rmtree(path) if path.is_dir() else path.unlink(missing_ok=True)

    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp) / "ogg"
        git("clone", "--quiet", "--depth", "1", "--branch", f"v{VERSION}", REPO, str(src))
        commit = git("-C", str(src), "rev-parse", "HEAD")

        for rel in FILES:
            write(dest / rel, (src / rel).read_text())

        template = (src / "include/ogg/config_types.h.in").read_text()
        for key, value in CONFIG_TYPES.items():
            template = template.replace(f"@{key}@", value)
        write(dest / "include/ogg/config_types.h", template)

    # '#include <ogg/x.h>' -> '#include "<path to include/ogg/x.h relative to the including file>"'
    for path in list(dest.rglob("*.c")) + list(dest.rglob("*.h")):
        def relative(match):
            target = dest / "include/ogg" / match.group(2)
            return f'{match.group(1)}"{Path(os.path.relpath(target, path.parent)).as_posix()}"'

        write(path, re.sub(r"(#\s*include\s*)<ogg/([\w.]+)>", relative, path.read_text()))

    write(dest / "JUCE_UPSTREAM.txt", UPSTREAM_NOTE.format(repo=REPO, version=VERSION, commit=commit))


if __name__ == "__main__":
    main()
