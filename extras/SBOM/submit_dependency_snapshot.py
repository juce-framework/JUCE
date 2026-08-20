#!/usr/bin/env python3

"""Submits the JUCE software bill of materials from JUCE.spdx.json to GitHub's
dependency submission API, so that the vendored third-party code appears in the
repository's dependency graph and in GitHub's own SBOM export.

Usage:
    submit_dependency_snapshot.py <path-to-juce-root>            # submit
    submit_dependency_snapshot.py <path-to-juce-root> --dry-run  # print only
"""

import argparse
import datetime
import json
import os
import sys
import urllib.error
import urllib.request
from pathlib import Path


CORRELATOR = 'juce-sbom'
JUCE_SUPPLIER = 'Organization: Raw Material Software Limited'
MANIFEST_PATH = 'extras/SBOM/dependencies.json'


def fail (message):
    print (f'error: {message}', file=sys.stderr)
    sys.exit (1)


def github_environment (dry_run):
    def value_of (name, placeholder):
        value = os.environ.get (name, '')

        if not value:
            if dry_run:
                return placeholder

            fail (f'{name} is not set')

        return value

    return { 'repository': value_of ('GITHUB_REPOSITORY', 'juce-framework/JUCE'),
             'sha':        value_of ('GITHUB_SHA', '0' * 40),
             'ref':        value_of ('GITHUB_REF', 'refs/heads/develop'),
             'run_id':     value_of ('GITHUB_RUN_ID', '0'),
             'api_url':    os.environ.get ('GITHUB_API_URL', 'https://api.github.com') }


def package_purl (package):
    for reference in package.get ('externalRefs', []):
        if reference['referenceType'] == 'purl':
            return reference['referenceLocator']

    fail (f'{package["name"]} has no purl')


def build_snapshot (document, environment):
    development = { r['spdxElementId'] for r in document['relationships']
                    if r['relationshipType'] == 'DEV_DEPENDENCY_OF' }

    resolved = {}

    for package in document['packages']:
        if package.get ('supplier') == JUCE_SUPPLIER:
            continue

        resolved[package['name']] = {
            'package_url': package_purl (package),
            'relationship': 'direct',
            'scope': 'development' if package['SPDXID'] in development else 'runtime',
        }

    return { 'version': 0,
             'sha': environment['sha'],
             'ref': environment['ref'],
             'job': { 'id': environment['run_id'], 'correlator': CORRELATOR },
             'detector': { 'name': 'generate_sbom.py',
                           'version': document['name'].removeprefix ('JUCE-'),
                           'url': 'https://github.com/juce-framework/JUCE' },
             'scanned': datetime.datetime.now (datetime.timezone.utc).isoformat(),
             'manifests': { 'JUCE.spdx.json': { 'name': 'JUCE.spdx.json',
                                                'file': { 'source_location': MANIFEST_PATH },
                                                'resolved': resolved } } }


def submit (snapshot, environment):
    url = f'{environment["api_url"]}/repos/{environment["repository"]}/dependency-graph/snapshots'
    token = os.environ.get ('GITHUB_TOKEN', '')

    if not token:
        fail ('GITHUB_TOKEN is not set')

    request = urllib.request.Request (url,
                                      data=json.dumps (snapshot).encode(),
                                      headers={ 'Authorization': f'Bearer {token}',
                                                'Accept': 'application/vnd.github+json',
                                                'X-GitHub-Api-Version': '2022-11-28',
                                                'Content-Type': 'application/json' },
                                      method='POST')

    try:
        with urllib.request.urlopen (request) as response:
            print (f'{response.status}: {response.read().decode()}')
    except urllib.error.HTTPError as error:
        fail (f'submission failed with {error.code}: {error.read().decode()}')


def main():
    parser = argparse.ArgumentParser (description='Submit the JUCE SBOM to the '
                                                  'GitHub dependency submission API.')
    parser.add_argument ('juce_root', type=Path, help='path to the root of the JUCE repository')
    parser.add_argument ('--dry-run', action='store_true',
                         help='print the snapshot instead of submitting it')
    arguments = parser.parse_args()

    document_path = arguments.juce_root.resolve() / 'JUCE.spdx.json'

    if not document_path.exists():
        fail (f'{document_path} does not exist; run generate_sbom.py to create it')

    document = json.loads (document_path.read_text (encoding='utf-8'))
    environment = github_environment (arguments.dry_run)
    snapshot = build_snapshot (document, environment)
    resolved = snapshot['manifests']['JUCE.spdx.json']['resolved']
    development = sum (1 for p in resolved.values() if p['scope'] == 'development')

    print (f'{len (resolved)} dependencies ({development} development-only)', file=sys.stderr)

    if arguments.dry_run:
        print (json.dumps (snapshot, indent=2))
    else:
        submit (snapshot, environment)


if __name__ == '__main__':
    main()
