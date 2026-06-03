#!/usr/bin/env python3
import json
import os
import subprocess
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_BIN = ROOT / "build-docker" / "opencalcypher"
BIN = Path(os.environ.get("OPENCALCYPHER_BIN", str(DEFAULT_BIN))).resolve()
WORKDIR = Path(os.environ.get("OPENCALCYPHER_WEB_WORKDIR", str(ROOT / ".opencalcypher-web"))).resolve()
MAX_BODY = 4096


HTML = """<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>OpenCalcypher Web</title>
  <style>
    :root { color-scheme: light; font-family: Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; }
    body { margin: 0; background: #f7f7f4; color: #171717; }
    main { max-width: 1120px; margin: 0 auto; padding: 48px 20px; }
    header { margin-bottom: 32px; }
    h1 { font-size: 40px; line-height: 1.05; margin: 0 0 12px; letter-spacing: 0; }
    h2 { font-size: 18px; margin: 0 0 16px; }
    p { margin: 0; color: #555; max-width: 720px; line-height: 1.5; }
    .grid { display: grid; grid-template-columns: minmax(0, 1fr) 360px; gap: 24px; align-items: start; }
    .panel, aside { background: #fff; border: 1px solid #deded8; border-radius: 8px; padding: 20px; }
    .steps { display: grid; gap: 16px; }
    .step { border-top: 1px solid #ecece6; padding-top: 16px; }
    .step:first-child { border-top: 0; padding-top: 0; }
    label { display: block; font-size: 13px; font-weight: 650; margin-bottom: 6px; }
    input, select { width: 100%; box-sizing: border-box; border: 1px solid #cfcfc8; border-radius: 6px; padding: 10px 11px; font: inherit; background: #fff; }
    .row { display: grid; grid-template-columns: repeat(3, 1fr); gap: 12px; margin-bottom: 12px; }
    button { border: 0; background: #1d4f45; color: #fff; border-radius: 6px; padding: 10px 13px; font-weight: 700; cursor: pointer; }
    button.secondary { background: #343434; }
    pre { white-space: pre-wrap; word-break: break-word; background: #1c1d1b; color: #f3f3ef; border-radius: 8px; padding: 16px; min-height: 160px; margin: 16px 0 0; font-size: 13px; }
    aside ul { padding-left: 18px; margin: 10px 0 0; color: #555; line-height: 1.5; }
    @media (max-width: 860px) { .grid, .row { grid-template-columns: 1fr; } h1 { font-size: 32px; } main { padding-top: 28px; } }
  </style>
</head>
<body>
  <main>
    <header>
      <h1>OpenCalcypher Web</h1>
      <p>Local screens for the same artifact workflow as the command line: context, keys, encryption, inspection, verification, and decryption. The page calls a local helper that runs the OpenCalcypher binary.</p>
    </header>
    <div class="grid">
      <section class="panel">
        <h2>Artifact Workflow</h2>
        <div class="steps">
          <div class="step">
            <div class="row">
              <div><label for="scheme">Scheme</label><select id="scheme"><option>bfv</option><option>bgv</option></select></div>
              <div><label for="depth">Depth</label><input id="depth" type="number" min="1" max="8" value="2"></div>
              <div><label for="batchSize">Batch size</label><input id="batchSize" type="number" min="1" max="4096" value="16"></div>
            </div>
            <button data-action="context">Create context</button>
          </div>
          <div class="step"><button data-action="keygen">Generate keys</button></div>
          <div class="step">
            <label for="value">Integer to encrypt</label>
            <input id="value" type="number" value="42">
            <div style="height:12px"></div>
            <button data-action="encrypt">Encrypt integer</button>
          </div>
          <div class="step">
            <button class="secondary" data-action="inspect">Inspect ciphertext</button>
            <button class="secondary" data-action="verify">Verify artifacts</button>
            <button class="secondary" data-action="decrypt">Decrypt integer</button>
          </div>
        </div>
        <pre id="output">Ready. Start with Create context.</pre>
      </section>
      <aside>
        <h2>Files Created Locally</h2>
        <ul>
          <li><code>context.json</code>: FHE parameters and serialized OpenFHE context.</li>
          <li><code>keys/public.key</code>: public key artifact.</li>
          <li><code>keys/secret.key</code>: secret key artifact kept in this local workspace.</li>
          <li><code>value.ct.json</code>: ciphertext artifact with <code>plaintextIncluded: false</code>.</li>
        </ul>
      </aside>
    </div>
  </main>
  <script>
    const output = document.querySelector('#output');
    async function run(action) {
      output.textContent = 'Running ' + action + '...';
      const payload = {
        scheme: document.querySelector('#scheme').value,
        depth: Number(document.querySelector('#depth').value),
        batchSize: Number(document.querySelector('#batchSize').value),
        value: Number(document.querySelector('#value').value)
      };
      const res = await fetch('/api/' + action, { method: 'POST', headers: { 'content-type': 'application/json' }, body: JSON.stringify(payload) });
      const data = await res.json();
      output.textContent = JSON.stringify(data, null, 2);
    }
    document.querySelectorAll('button[data-action]').forEach((button) => button.addEventListener('click', () => run(button.dataset.action)));
  </script>
</body>
</html>
"""


def run_cli(args):
    if not BIN.exists():
        raise RuntimeError(f"OpenCalcypher binary not found: {BIN}")
    WORKDIR.mkdir(parents=True, exist_ok=True)
    result = subprocess.run([str(BIN), *args], cwd=WORKDIR, text=True, capture_output=True, timeout=120)
    if result.returncode != 0:
        raise RuntimeError(result.stderr.strip() or result.stdout.strip() or f"command failed: {args[0]}")
    return result.stdout.strip()


def require_int(value, name, minimum, maximum):
    if not isinstance(value, int) or value < minimum or value > maximum:
        raise ValueError(f"{name} must be an integer between {minimum} and {maximum}")
    return str(value)


def handle_action(action, body):
    if action == "context":
        scheme = str(body.get("scheme", "bfv")).lower()
        if scheme not in ("bfv", "bgv"):
            raise ValueError("scheme must be bfv or bgv for integer artifacts")
        depth = require_int(body.get("depth", 2), "depth", 1, 8)
        batch_size = require_int(body.get("batchSize", 16), "batchSize", 1, 4096)
        run_cli(["context", "--scheme", scheme, "--security", "128", "--depth", depth, "--batch-size", batch_size, "--out", "context.json"])
        return {"ok": True, "created": "context.json"}
    if action == "keygen":
        run_cli(["keygen", "--context", "context.json", "--out", "keys"])
        return {"ok": True, "created": ["keys/public.key", "keys/secret.key"]}
    if action == "encrypt":
        value = require_int(body.get("value", 42), "value", -1000000, 1000000)
        run_cli(["encrypt-int", "--context", "context.json", "--public-key", "keys/public.key", "--value", value, "--out", "value.ct.json"])
        return {"ok": True, "created": "value.ct.json"}
    if action == "inspect":
        return {"ok": True, "inspect": json.loads(run_cli(["inspect", "value.ct.json"]))}
    if action == "verify":
        return {"ok": True, "verify": run_cli(["verify", "--context", "context.json", "--public-key", "keys/public.key", "--in", "value.ct.json"])}
    if action == "decrypt":
        return {"ok": True, "value": int(run_cli(["decrypt-int", "--context", "context.json", "--secret-key", "keys/secret.key", "--in", "value.ct.json"]))}
    raise ValueError(f"unknown action: {action}")


class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        return

    def send_json(self, status, payload):
        data = json.dumps(payload).encode()
        self.send_response(status)
        self.send_header("content-type", "application/json")
        self.send_header("content-length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_GET(self):
        if urlparse(self.path).path != "/":
            self.send_error(404)
            return
        data = HTML.encode()
        self.send_response(200)
        self.send_header("content-type", "text/html; charset=utf-8")
        self.send_header("content-length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_POST(self):
        path = urlparse(self.path).path
        if not path.startswith("/api/"):
            self.send_error(404)
            return
        length = int(self.headers.get("content-length", "0"))
        if length > MAX_BODY:
            self.send_json(413, {"ok": False, "error": "request body too large"})
            return
        try:
            body = json.loads(self.rfile.read(length) or b"{}")
            payload = handle_action(path.rsplit("/", 1)[-1], body)
            self.send_json(200, payload)
        except Exception as exc:
            self.send_json(400, {"ok": False, "error": str(exc)})


if __name__ == "__main__":
    host = os.environ.get("OPENCALCYPHER_WEB_HOST", "127.0.0.1")
    port = int(os.environ.get("OPENCALCYPHER_WEB_PORT", "8089"))
    print(f"OpenCalcypher Web listening on http://{host}:{port}")
    ThreadingHTTPServer((host, port), Handler).serve_forever()
