#!/usr/bin/env python3

"""Generates (or checks) the SPDX software bill of materials for the JUCE framework.

Usage:
    generate_sbom.py <path-to-juce-root>            # writes <juce-root>/JUCE.spdx.json
    generate_sbom.py <path-to-juce-root> --check    # fails if the committed file is stale
    generate_sbom.py <path-to-juce-root> --output <path>

This script uses the manifest of vendored third-party code located in
extras/SBOM/dependencies.json, which holds (amongst other information):

  "lastUpdated"         The SPDX creation timestamp. Bump this whenever the
                        manifest or any vendored code changes.
  "dependencies"        One entry per vendored third-party component:

    "name"         The upstream project name.
    "module"       The JUCE module (or component) containing it, which owns
                   its package in the SPDX document.
    "path"         The directory or single file where it is vendored,
                   relative to the repository root.
    "also"         Optional: additional covered files outside "path".
    "license"      An SPDX licence expression. Any LicenseRef- identifier
                   used must have an entry in EXTRACTED_LICENSES below.
    "licenseFile"  The licence text within the tree, relative to the
                   repository root. For a LicenseRef- licence this file
                   supplies the SPDX extractedText (see EXTRACTED_LICENSES).
    "homepage"     The upstream URL (or null), used as the SPDX
                   downloadLocation.
    "supplier"     "Organization: <name>" or "Person: <name>", normally the
                   upstream copyright holder. Required: every package must
                   name a supplier for the document to be NTIA-conformant.
    "purl"         Optional: the package URL without a version; the extracted
                   version is appended automatically.
    "cpe"          Optional: a CPE 2.3 prefix ending at the product field
                   (e.g. "cpe:2.3:a:zlib:zlib"); the extracted version
                   completes the identifier. Only add a CPE that exists in
                   the NVD CPE dictionary.
    "version"      A version rule, described below.

To add a new dependency: vendor the code, add a JUCE_UPSTREAM.txt file to its
directory (see below), add a manifest entry, bump "lastUpdated", and run this
script. The same applies when updating a dependency.

Version rules in the manifest are one of:
  {"file": <path>, "patterns": [<regex>...], "transform": <name, optional>}
      Each regex contributes its capture groups, joined with '.' or passed to
      the named transform function defined in this script.
  {"directory": <path>, "pattern": <regex>}
      The version is captured from the name of a directory entry.
  {"unversioned": <note>}
      Upstream publishes no version information; the note goes into the SPDX
      sourceInfo field.

Directories of vendored code may carry a JUCE_UPSTREAM.txt file documenting
the provenance of the code and any local modifications: a header block of "Key:
value" lines (Upstream: always; Version: or Commit: when the version cannot be
determined from the upstream source directly), a blank line, then free-form
notes. This script will note the presence of JUCE_UPSTREAM.txt in the SPDX
sourceInfo field.

The generated SPDX document is designed so that JUCE's build systems can reduce
it down to the modules a project actually uses without needing to understand its
contents:

  - Each JUCE module is a package with SPDXID "SPDXRef-<module>" where
    <module> is the module name with underscores replaced by hyphens
    (e.g. "SPDXRef-juce-core").
  - Each third-party dependency is a package with SPDXID
    "SPDXRef-<module>.<library>" (e.g. "SPDXRef-juce-core.zlib"). The dot
    cannot appear in a module ID, so ownership is unambiguous.

To remove an unused module M: delete every package whose SPDXID is exactly
"SPDXRef-<M>" or starts with "SPDXRef-<M>.", then delete every relationship
whose spdxElementId or relatedSpdxElement referred to a deleted package.
Nothing else in the document needs to change.

The examples, bundled applications, and build system also carry third-party
code. Their packages use the same scheme under the non-module owners listed in
the manifest's "components" object (e.g. "SPDXRef-examples.reaper-sdk").  None
of these are ever part of a user's product, so a project-level trimming utility
should remove those owners and their contents outright, using the same rule.

The SPDX document produced is NTIA-conformant and contains enough information to
be converted to the CycloneDX format using the CycloneDX CLI tool for security
SBOM analysis.
"""

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path


MANIFEST_PATH = 'extras/SBOM/dependencies.json'

JUCE_DOWNLOAD_LOCATION = 'git+https://github.com/juce-framework/JUCE.git'
JUCE_SUPPLIER = 'Organization: Raw Material Software Limited'

EXCLUDED_DIRS = frozenset ({ '.git', 'build', 'Builds', 'JuceLibraryCode' })

SOURCE_EXTENSIONS = { '.h', '.hpp', '.hxx', '.c', '.cc', '.cpp', '.cxx',
                      '.m', '.mm', '.java', '.kt', '.ts', '.js' }

MODULE_LICENSES = {
    'AGPLv3/Commercial': 'AGPL-3.0-only OR LicenseRef-JUCE-Commercial',
    'ISC':               'ISC',
}

# Licences referenced by the manifest or the module declarations that have no
# identifier on the SPDX licence list. SPDX 2.3 requires the extractedText of
# each to be the actual licence text found in the package, so it is the full
# contents of a file in the tree: the "licenseFile" of the manifest dependency
# that uses the identifier, or the "file" named here for licences that the
# manifest does not introduce. "seeAlsos" lists the URLs of the full terms
# where the in-tree text refers out to them.
EXTRACTED_LICENSES = {
    'LicenseRef-JUCE-Commercial': { 'name':     'JUCE Commercial Licence',
                                    'file':     'LICENSE.md',
                                    'seeAlsos': [ 'https://juce.com/legal/juce-9-licence/' ] },
    'LicenseRef-Avid-AAX':        { 'name':     'Avid AAX SDK Licence',
                                    'seeAlsos': [ 'https://developer.avid.com/aax' ] },
    'LicenseRef-Steinberg-ASIO':  { 'name':     'Steinberg ASIO SDK Licence',
                                    'seeAlsos': [ 'https://www.steinberg.net/en/company/developers.html' ] },
    'LicenseRef-Public-Domain':   { 'name':     'Public Domain' },
}


def fail (message):
    print (f'error: {message}', file=sys.stderr)
    sys.exit (1)


def framework_version (juce_root):
    """The JUCE version string, parsed from the version macros in
    juce_StandardHeader.h."""
    header = Path (juce_root) / 'modules' / 'juce_core' / 'system' / 'juce_StandardHeader.h'
    text = header.read_text (encoding='utf-8', errors='replace')
    parts = []

    for macro in ('JUCE_MAJOR_VERSION', 'JUCE_MINOR_VERSION', 'JUCE_BUILDNUMBER'):
        if (match := re.search (rf'#define\s+{macro}\s+(\d+)', text)) is None:
            fail (f'{macro} not found in {header}')

        parts.append (match.group (1))

    return '.'.join (parts)


def load_manifest (juce_root):
    path = Path (juce_root) / MANIFEST_PATH

    if not path.exists():
        fail (f'no SBOM manifest found at {path}')

    return json.loads (path.read_text (encoding='utf-8'))


def third_party_paths (juce_root):
    """Repo-relative paths (directories or single files) of all vendored
    third-party code, for other tooling (e.g. check_copyright_banners.py) to
    exclude from checks that only apply to JUCE-authored files. First-party
    entries such as the webview-interop package are not included."""
    return tuple (d['path'] for d in load_manifest (juce_root)['dependencies']
                  if not d.get ('firstParty', False))


def aax_version (groups):
    value = int (groups[0], 16)
    return f'{value >> 8}.{value & 0xff}'


def lv2_version (groups):
    return f'1.{groups[0]}.{groups[1]}'


TRANSFORMS = { 'aax': aax_version, 'lv2': lv2_version }


def extract_version (root, spec, name):
    if 'unversioned' in spec:
        return None

    if 'directory' in spec:
        pattern = re.compile (spec['pattern'])

        for entry in sorted (p.name for p in (root / spec['directory']).iterdir()):
            if (match := pattern.match (entry)) is not None:
                return match.group (1)

        fail (f'{name}: no directory in {spec["directory"]} matches {spec["pattern"]}')

    if 'file' not in spec:
        fail (f'{name}: unrecognised version rule {spec}')

    text = (root / spec['file']).read_text (encoding='utf-8', errors='replace')
    groups = []

    for pattern in spec['patterns']:
        if (match := re.search (pattern, text)) is None:
            fail (f'{name}: pattern {pattern} not found in {spec["file"]}')

        groups.extend (match.groups())

    if 'transform' in spec:
        return TRANSFORMS[spec['transform']] (groups)

    return '.'.join (groups)


def parse_module_declarations (root):
    """Returns {module_name: {key: value}} from each module's
    BEGIN_JUCE_MODULE_DECLARATION block."""
    modules = {}

    for module_dir in sorted ((root / 'modules').iterdir()):
        if not module_dir.is_dir() or not module_dir.name.startswith ('juce_'):
            continue

        header = module_dir / f'{module_dir.name}.h'

        if not header.exists():
            fail (f'no module header found for {module_dir.name}')

        text = header.read_text (encoding='utf-8', errors='replace')
        match = re.search (r'BEGIN_JUCE_MODULE_DECLARATION(.*?)END_JUCE_MODULE_DECLARATION',
                           text, re.DOTALL)

        if match is None:
            fail (f'no module declaration found in {header}')

        metadata = {}
        current_key = None

        for line in match.group (1).splitlines():
            if (key_value := re.match (r'^\s*([a-zA-Z]+):\s*(.*?)\s*$', line)) is not None:
                current_key = key_value.group (1)
                metadata[current_key] = key_value.group (2)
            elif current_key is not None and line.strip():
                metadata[current_key] += ' ' + line.strip()

        modules[module_dir.name] = metadata

    return modules


def scan_for_uncovered_files (root, manifest):
    """Every non-juce_-prefixed source file under modules/ must belong to a
    manifest dependency or match a first-party pattern."""
    dependencies = manifest['dependencies']
    first_party = [re.compile (p) for p in manifest.get ('firstPartyPatterns', [])]
    covered_dirs  = [d['path'] + '/' for d in dependencies]
    covered_files = { d['path'] for d in dependencies }
    covered_files.update (extra for d in dependencies for extra in d.get ('also', []))
    uncovered = []

    for path in sorted ((root / 'modules').rglob ('*')):
        if not path.is_file() or path.suffix not in SOURCE_EXTENSIONS:
            continue

        if path.name.startswith ('juce_'):
            continue

        relative = path.relative_to (root).as_posix()

        if any (part in EXCLUDED_DIRS for part in relative.split ('/')):
            continue

        if any (pattern.match (relative) for pattern in first_party):
            continue

        if relative in covered_files or any (relative.startswith (d) for d in covered_dirs):
            continue

        uncovered.append (relative)

    return uncovered


def spdx_id_for_module (module_name):
    return 'SPDXRef-' + module_name.replace ('_', '-')


def dependency_slug (dependency):
    return re.sub (r'[^A-Za-z0-9.-]+', '-', dependency['name'].lower()).strip ('-')


def spdx_id_for_dependency (dependency):
    return spdx_id_for_module (dependency['module']) + '.' + dependency_slug (dependency)


def normalised_text (text):
    """Licence text with line endings and trailing whitespace normalised, so
    that the document does not depend on how the file happens to be stored."""
    return '\n'.join (line.rstrip() for line in text.splitlines()).strip()


def license_refs (expression):
    return re.findall (r'LicenseRef-[A-Za-z0-9.-]+', expression)


def extracted_licensing_info (root, manifest, packages):
    """The hasExtractedLicensingInfos entries for every LicenseRef- identifier
    used by a package, with the text read from the tree as described at
    EXTRACTED_LICENSES."""
    used = sorted ({ ref for p in packages for ref in license_refs (p['licenseDeclared']) })
    extracted = []

    for license_id in used:
        if (info := EXTRACTED_LICENSES.get (license_id)) is None:
            fail (f'{license_id} is used but has no entry in EXTRACTED_LICENSES')

        files = [info['file']] if 'file' in info else [d['licenseFile'] for d in manifest['dependencies']
                                                                         if license_id in license_refs (d['license'])]

        for file in files:
            if not (root / file).is_file():
                fail (f'{license_id}: licence file {file} does not exist')

        texts = { normalised_text ((root / file).read_text (encoding='utf-8', errors='replace')) for file in files }

        if len (texts) != 1:
            fail (f'{license_id}: expected exactly one licence text but found {len (texts)} '
                  f'in {", ".join (files) or "no files"}')

        entry = { 'licenseId': license_id, 'name': info['name'], 'extractedText': texts.pop() }

        if 'seeAlsos' in info:
            entry['seeAlsos'] = info['seeAlsos']

        extracted.append (entry)

    return extracted


def build_document (root, modules, manifest):
    juce_version = framework_version (root)
    packages = []
    relationships = [ { 'spdxElementId': 'SPDXRef-DOCUMENT',
                        'relationshipType': 'DESCRIBES',
                        'relatedSpdxElement': 'SPDXRef-JUCE' } ]

    packages.append ({ 'name': 'JUCE',
                       'SPDXID': 'SPDXRef-JUCE',
                       'versionInfo': juce_version,
                       'supplier': JUCE_SUPPLIER,
                       'downloadLocation': JUCE_DOWNLOAD_LOCATION,
                       'filesAnalyzed': False,
                       'licenseConcluded': 'AGPL-3.0-only OR LicenseRef-JUCE-Commercial',
                       'licenseDeclared': 'AGPL-3.0-only OR LicenseRef-JUCE-Commercial',
                       'copyrightText': 'Copyright (c) Raw Material Software Limited' })

    for module_name, metadata in modules.items():
        declared = metadata.get ('license', '')
        license_expression = MODULE_LICENSES.get (declared)

        if license_expression is None:
            print (f'warning: module {module_name} has unrecognised licence "{declared}"; '
                   f'add it to MODULE_LICENSES', file=sys.stderr)
            license_expression = 'NOASSERTION'

        packages.append ({ 'name': module_name,
                           'SPDXID': spdx_id_for_module (module_name),
                           'versionInfo': metadata.get ('version', juce_version),
                           'supplier': JUCE_SUPPLIER,
                           'downloadLocation': JUCE_DOWNLOAD_LOCATION,
                           'filesAnalyzed': False,
                           'licenseConcluded': license_expression,
                           'licenseDeclared': license_expression,
                           'copyrightText': 'Copyright (c) Raw Material Software Limited',
                           'sourceInfo': f'modules/{module_name} in the JUCE source tree' })

        relationships.append ({ 'spdxElementId': 'SPDXRef-JUCE',
                                'relationshipType': 'CONTAINS',
                                'relatedSpdxElement': spdx_id_for_module (module_name) })

        for dependency in re.split (r'[,\s]+', metadata.get ('dependencies', '')):
            if dependency in modules:
                relationships.append ({ 'spdxElementId': spdx_id_for_module (module_name),
                                        'relationshipType': 'DEPENDS_ON',
                                        'relatedSpdxElement': spdx_id_for_module (dependency) })

    for component, info in manifest['components'].items():
        packages.append ({ 'name': component,
                           'SPDXID': spdx_id_for_module (component),
                           'versionInfo': juce_version,
                           'supplier': JUCE_SUPPLIER,
                           'downloadLocation': JUCE_DOWNLOAD_LOCATION,
                           'filesAnalyzed': False,
                           'licenseConcluded': info['license'],
                           'licenseDeclared': info['license'],
                           'copyrightText': 'Copyright (c) Raw Material Software Limited',
                           'sourceInfo': f'{info["description"]} in the JUCE source tree' })

        relationships.append ({ 'spdxElementId': 'SPDXRef-JUCE',
                                'relationshipType': 'CONTAINS',
                                'relatedSpdxElement': spdx_id_for_module (component) })

    for dependency in manifest['dependencies']:
        spec = dependency['version']
        version = extract_version (root, spec, dependency['name'])

        # Every package must name a supplier for the document to meet the
        # NTIA SBOM minimum elements, so a manifest entry without one is an
        # error.
        supplier = dependency.get ('supplier', '')

        if not supplier.startswith (('Organization: ', 'Person: ')):
            fail (f'{dependency["name"]}: NTIA conformance requires a supplier of the form '
                  f'"Organization: <name>" or "Person: <name>"')

        source_info = f'Vendored at {dependency["path"]} in the JUCE source tree'

        if 'unversioned' in spec:
            source_info += f'; {spec["unversioned"]}'

        if (root / dependency['path'] / 'JUCE_UPSTREAM.txt').exists():
            source_info += '; provenance and local modifications are documented in JUCE_UPSTREAM.txt'

        package = { 'name': dependency['name'],
                    'SPDXID': spdx_id_for_dependency (dependency),
                    'versionInfo': version if version is not None else 'NOASSERTION',
                    'supplier': supplier,
                    'downloadLocation': dependency['homepage'] or 'NOASSERTION',
                    'filesAnalyzed': False,
                    'licenseConcluded': dependency['license'],
                    'licenseDeclared': dependency['license'],
                    'copyrightText': 'NOASSERTION',
                    'sourceInfo': source_info }

        external_refs = []

        # Every dependency carries a purl so that purl-based consumers (e.g.
        # the GitHub dependency snapshot) never need to synthesise an
        # identifier; packages with no upstream package URL get a generic
        # one. The version is appended so that SBOM consumers such as
        # vulnerability scanners can match the exact package.
        locator = dependency.get ('purl', 'pkg:generic/' + dependency_slug (dependency))
        external_refs.append ({ 'referenceCategory': 'PACKAGE-MANAGER',
                                'referenceType': 'purl',
                                'referenceLocator': locator + (f'@{version}' if version is not None else '') })

        # A CPE identifier lets NVD-based vulnerability scanners match the
        # package. The manifest holds the vendor and product fields, and the
        # extracted version completes the identifier.
        if 'cpe' in dependency and version is not None:
            external_refs.append ({ 'referenceCategory': 'SECURITY',
                                    'referenceType': 'cpe23Type',
                                    'referenceLocator': f'{dependency["cpe"]}:{version}:*:*:*:*:*:*:*' })

        if external_refs:
            package['externalRefs'] = external_refs

        packages.append (package)
        relationships.append ({ 'spdxElementId': spdx_id_for_module (dependency['module']),
                                'relationshipType': 'CONTAINS',
                                'relatedSpdxElement': spdx_id_for_dependency (dependency) })

        # Dependencies of the non-module components are never part of a JUCE
        # user's product, so mark them development-only for SBOM consumers.
        if dependency['module'] in manifest['components']:
            relationships.append ({ 'spdxElementId': spdx_id_for_dependency (dependency),
                                    'relationshipType': 'DEV_DEPENDENCY_OF',
                                    'relatedSpdxElement': 'SPDXRef-JUCE' })

    extracted = extracted_licensing_info (root, manifest, packages)

    document = { 'spdxVersion': 'SPDX-2.3',
                 'dataLicense': 'CC0-1.0',
                 'SPDXID': 'SPDXRef-DOCUMENT',
                 'name': f'JUCE-{juce_version}',
                 'documentNamespace': None,
                 'creationInfo': { 'created': manifest['lastUpdated'],
                                   'creators': [ 'Tool: generate_sbom.py',
                                                 JUCE_SUPPLIER ] },
                 'packages': packages,
                 'relationships': relationships,
                 'hasExtractedLicensingInfos': extracted }

    # SPDX requires every revision of a document to have a unique namespace
    # URI, so derive it from the whole of the rest of the document: any change
    # at all, including to the creation timestamp alone, produces a new URI.
    content_hash = hashlib.sha256 (json.dumps (document, sort_keys=True).encode()).hexdigest()[:12]
    document['documentNamespace'] = f'https://juce.com/spdx/JUCE-{juce_version}-{content_hash}'

    return document


def main():
    parser = argparse.ArgumentParser (description='Generate or check the JUCE SPDX SBOM.')
    parser.add_argument ('juce_root', type=Path, help='path to the root of the JUCE repository')
    parser.add_argument ('--output', type=Path, default=None,
                         help='output path (default: <juce_root>/JUCE.spdx.json)')
    parser.add_argument ('--check', action='store_true',
                         help='verify that the existing document is up to date instead of writing')
    arguments = parser.parse_args()

    root = arguments.juce_root.resolve()

    if not (root / 'modules' / 'juce_core').is_dir():
        fail (f'{root} does not look like a JUCE repository')

    manifest = load_manifest (root)
    uncovered = scan_for_uncovered_files (root, manifest)

    if uncovered:
        for path in uncovered:
            print (f'error: {path} looks like third-party code but has no entry in {MANIFEST_PATH}',
                   file=sys.stderr)

        fail (f'{len (uncovered)} file(s) are not covered by the manifest at {root / MANIFEST_PATH}')

    modules = parse_module_declarations (root)
    document = build_document (root, modules, manifest)

    output_path = arguments.output or (root / 'JUCE.spdx.json')
    serialized = json.dumps (document, indent=2) + '\n'

    if arguments.check:
        if not output_path.exists():
            fail (f'{output_path} does not exist; run generate_sbom.py to create it')

        if output_path.read_text (encoding='utf-8') != serialized:
            fail (f'{output_path} is out of date; re-run generate_sbom.py and commit the result')

        print (f'{output_path} is up to date ({len (document["packages"])} packages)')
    else:
        output_path.write_text (serialized, encoding='utf-8')
        print (f'wrote {output_path} ({len (document["packages"])} packages)')


if __name__ == '__main__':
    main()
