import * as esbuild from 'esbuild';
import { readFileSync } from 'node:fs';

// esbuild always prints its own lowering helpers (needed to target es2015) before anything
// else in the bundle, so `--legal-comments=inline` can't keep the license header at the very
// top. Strip the per-file headers instead and re-add a single copy via `banner`, which esbuild
// does place before everything else.
const entryPoint = 'src/index.ts';
const licenseHeader = readFileSync(entryPoint, 'utf8').match(
  /^\/\*![\s\S]*?\*\//,
)[0];

const { version } = JSON.parse(readFileSync('package.json', 'utf8'));

await esbuild.build({
  entryPoints: [entryPoint],
  outfile: 'dist/index.js',
  bundle: true,
  format: 'esm',
  target: 'es2015',
  sourcemap: true,
  legalComments: 'none',
  banner: { js: licenseHeader },
  define: { __JUCE_FRAMEWORK_WEBVIEW_VERSION__: JSON.stringify(version) },
});
