import * as esbuild from "esbuild";
import { cp, mkdir, readFile, rm, writeFile } from "node:fs/promises";
import path from "node:path";

const isServe = process.argv.includes("--serve");
const outdir = "build";

const liveReloadScript = `
    <script>
      new EventSource("/esbuild").addEventListener("change", () => location.reload());
    </script>`;

async function writeIndexHtml() {
  let html = await readFile("public/index.html", "utf8");

  if (isServe) html = html.replace("</body>", `${liveReloadScript}\n  </body>`);

  await writeFile(path.join(outdir, "index.html"), html);
}

async function copyPublicAssets() {
  await cp("public", outdir, {
    recursive: true,
    filter: (source) => path.basename(source) !== "index.html",
  });
}

const buildOptions = {
  entryPoints: ["src/index.tsx"],
  outdir,
  bundle: true,
  format: "esm",
  target: "es2015",
  sourcemap: true,
  minify: !isServe,
  entryNames: "[name]",
  assetNames: "assets/[name]-[hash]",
  loader: {
    ".woff": "file",
    ".woff2": "file",
  },
  jsx: "automatic",
  define: {
    "process.env.NODE_ENV": isServe ? '"development"' : '"production"',
  },
  logLevel: "info",
};

await rm(outdir, { recursive: true, force: true });
await mkdir(outdir, { recursive: true });
await copyPublicAssets();
await writeIndexHtml();

if (isServe) {
  const ctx = await esbuild.context(buildOptions);
  await ctx.watch();

  await ctx.serve({ servedir: outdir, port: 3000 });
} else {
  await esbuild.build(buildOptions);
}
