# OpenCalcypher

[![pipeline](https://gitlab.com/triviatec/opencalcypher/badges/main/pipeline.svg)](https://gitlab.com/triviatec/opencalcypher/-/pipelines)
[![coverage](https://gitlab.com/triviatec/opencalcypher/badges/main/coverage.svg)](https://gitlab.com/triviatec/opencalcypher/-/jobs)
[![GitHub tests](https://github.com/abe-triviatec/opencalcypher/actions/workflows/ci.yml/badge.svg)](https://github.com/abe-triviatec/opencalcypher/actions/workflows/ci.yml)
![public tools coverage](coverage/badges/public-tools-coverage.svg)
![OpenCalcypher coverage](coverage/badges/opencalcypher-coverage.svg)

OpenCalcypher is an open source command-line tool for creating, decrypting,
inspecting, and validating fully homomorphic encryption artifacts.

It is designed for developers who want a terminal-first way to work with FHE
contexts, keys, and ciphertexts before sending ciphertexts to applications such
as Calcypher Community.

## Project Metadata

- Website: https://triviatec.com
- GitHub repository: https://github.com/abe-triviatec/opencalcypher
- Topics: `fhe`, `homomorphic-encryption`, `openfhe`, `privacy`, `encryption`, `cryptography`, `cli`, `developer-tools`, `calcypher`, `triviatec`

## What It Does

- Creates explicit FHE contexts.
- Generates public and secret keys for that context.
- Encrypts integer values into portable ciphertext artifacts.
- Decrypts ciphertext artifacts locally with the matching secret key.
- Inspects artifact metadata while keeping plaintext out of inspection output.
- Validates that artifacts match a context and key.

## What It Does Not Do

OpenCalcypher does not run encrypted computation.

It does not provide:

- encrypted add or multiply;
- SQL or CQL;
- analytics;
- table storage;
- hosted APIs;
- production deployment.

Use Calcypher Community to run local encrypted computation demos on ciphertexts.
Use Calcypher Pro or Enterprise for managed and production use cases.

## Quickstart

Build from source when using a local checkout:

```bash
cmake -S . -B build
cmake --build build --target opencalcypher -j2
```

```bash
opencalcypher context --scheme bfv --security 128 --depth 2 --batch-size 4096 --out context.json
opencalcypher keygen --context context.json --out keys/
opencalcypher encrypt-int --context context.json --public-key keys/public.key --value 42 --out value.ct.json
opencalcypher inspect value.ct.json
opencalcypher decrypt-int --context context.json --secret-key keys/secret.key --in value.ct.json
```

Then use the ciphertext with Calcypher Community:

```bash
calcypher demo arithmetic --left-ciphertext value.ct.json --right-ciphertext other.ct.json --proof
```

If you want a screen-based path for the same downstream demo flow, use the
Community web companion (`cli/web-companion.html`) from the Calcypher Community
public export. It is a small, static UI that calls only local runtime endpoints
and returns only local runtime crypto outputs.

## Web Companion

OpenCalcypher also includes a local web companion for the same artifact
workflow:

```bash
OPENCALCYPHER_BIN="$PWD/build/opencalcypher" python3 web/server.py
```

Open `http://127.0.0.1:8089` and run the guided screens for context creation,
key generation, integer encryption, artifact inspection, verification, and
decryption. The web companion shells out to the local OpenCalcypher binary with
fixed argument lists; it does not simulate cryptography in the browser.

## Artifact Format

Ciphertext artifacts are JSON files with metadata that makes compatibility
auditable:

```json
{
  "format": "opencalcypher-ciphertext-v1",
  "scheme": "BFV",
  "contextHash": "sha256:...",
  "keyHash": "sha256:...",
  "encoding": "integer",
  "ciphertextSha256": "...",
  "ciphertextBytes": 12345,
  "ciphertext": "...",
  "plaintextIncluded": false
}
```

The ciphertext field contains a serialized homomorphic ciphertext, not a
generic encrypted blob. SSH, GPG, TLS, age, and OpenSSL ciphertexts are not
homomorphic ciphertexts and cannot be evaluated by Calcypher.

## Relationship With Calcypher

OpenCalcypher creates and verifies FHE artifacts.

Calcypher Community runs local encrypted computation demos using compatible
ciphertexts.

Calcypher Pro and Enterprise add managed APIs, larger workloads, support,
deployment options, and integration paths.

## License and Credits

OpenCalcypher is licensed under the Apache License 2.0. See `LICENSE`.

OpenCalcypher uses OpenFHE for homomorphic encryption primitives and OpenSSL
libcrypto for SHA-256 hashing. See `NOTICE` and `THIRD_PARTY_NOTICES.md` for
third-party credits and license notices.

## Coverage Gate

Public-tool coverage is calculated locally and enforced in GitLab CI and GitHub
Actions. The release gate fails unless both OpenCalcypher and Calcypher
Community runtime line coverage are strictly above 90%. Generated coverage SVG
badges are stored under `coverage/badges/` for README display.

## Releases

Standard release artifacts are planned for:

- Linux x64
- Linux arm64
- macOS arm64
- macOS x64
- Windows x64

Release binaries should be downloaded from GitLab or GitHub Releases. Large generated
binary artifacts are not committed to this repository.
