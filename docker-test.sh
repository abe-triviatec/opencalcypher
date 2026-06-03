#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ "$(basename "$(dirname "${SCRIPT_DIR}")")" == "tools" && "$(basename "${SCRIPT_DIR}")" == "opencalcypher" ]]; then
  REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
  SOURCE_DIR="tools/opencalcypher"
  BUILD_DIR="tools/opencalcypher/build-docker"
  WEB_SERVER="tools/opencalcypher/web/server.py"
else
  REPO_ROOT="${SCRIPT_DIR}"
  SOURCE_DIR="."
  BUILD_DIR="build-docker"
  WEB_SERVER="web/server.py"
fi
IMAGE="${OPENCALCYPHER_TEST_IMAGE:-he-engine-base:grpc1.50-openfhe1.5.0}"

docker run --rm \
  --user "$(id -u):$(id -g)" \
  -e OPC_SOURCE_DIR="${SOURCE_DIR}" \
  -e OPC_BUILD_DIR="${BUILD_DIR}" \
  -e OPC_WEB_SERVER="${WEB_SERVER}" \
  -v "${REPO_ROOT}:/workspace" \
  -w /workspace \
  "${IMAGE}" \
  bash -lc '
    set -euo pipefail
    cmake -S "$OPC_SOURCE_DIR" -B "$OPC_BUILD_DIR"
    cmake --build "$OPC_BUILD_DIR" --target opencalcypher -j2
    tmp="$(mktemp -d)"
    trap "rm -rf \"$tmp\"" EXIT
    bin="$OPC_BUILD_DIR/opencalcypher"
    "$bin" context --scheme bfv --security 128 --depth 2 --batch-size 16 --out "$tmp/context.json"
    "$bin" keygen --context "$tmp/context.json" --out "$tmp/keys"
    "$bin" encrypt-int --context "$tmp/context.json" --public-key "$tmp/keys/public.key" --value 42 --out "$tmp/value.ct.json"
    "$bin" inspect "$tmp/value.ct.json" | tee "$tmp/inspect.json"
    "$bin" verify --context "$tmp/context.json" --public-key "$tmp/keys/public.key" --in "$tmp/value.ct.json" | tee "$tmp/verify.txt"
    "$bin" decrypt-int --context "$tmp/context.json" --secret-key "$tmp/keys/secret.key" --in "$tmp/value.ct.json" | tee "$tmp/decrypt.txt"
    grep -q "\"format\": \"opencalcypher-ciphertext-v1\"" "$tmp/inspect.json"
    grep -q "\"plaintextIncluded\": false" "$tmp/inspect.json"
    grep -q "^ok$" "$tmp/verify.txt"
    grep -q "^42$" "$tmp/decrypt.txt"
    if "$bin" foo 2>"$tmp/invalid.txt"; then exit 1; fi
    grep -q "unknown command" "$tmp/invalid.txt"
    OPENCALCYPHER_BIN="/workspace/$bin" \
    OPENCALCYPHER_WEB_WORKDIR="$tmp/web-workdir" \
    OPENCALCYPHER_WEB_HOST=127.0.0.1 \
    OPENCALCYPHER_WEB_PORT=18089 \
      python3 "$OPC_WEB_SERVER" >"$tmp/web.log" 2>&1 &
    web_pid="$!"
    trap "kill $web_pid 2>/dev/null || true; rm -rf \"$tmp\"" EXIT
    python3 - <<'"'"'PY'"'"'
import json
import time
from urllib.request import Request, urlopen

base = "http://127.0.0.1:18089"
for _ in range(50):
    try:
        with urlopen(base, timeout=1) as response:
            assert response.status == 200
            break
    except Exception:
        time.sleep(0.1)
else:
    raise SystemExit("web companion did not start")

def post(action, payload):
    request = Request(
        base + "/api/" + action,
        data=json.dumps(payload).encode(),
        headers={"content-type": "application/json"},
        method="POST",
    )
    with urlopen(request, timeout=120) as response:
        data = json.loads(response.read().decode())
    if not data.get("ok"):
        raise SystemExit(f"{action} failed: {data}")
    return data

payload = {"scheme": "bfv", "depth": 2, "batchSize": 16, "value": 42}
post("context", payload)
post("keygen", payload)
post("encrypt", payload)
inspect = post("inspect", payload)["inspect"]
assert inspect["format"] == "opencalcypher-ciphertext-v1"
assert inspect["plaintextIncluded"] is False
assert post("verify", payload)["verify"] == "ok"
assert post("decrypt", payload)["value"] == 42
PY
  '
