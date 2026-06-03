# Security Policy

OpenCalcypher is a public developer tool for local fully homomorphic encryption
artifact workflows. Security reports are handled privately before public
discussion.

## Supported Versions

The `main` branch is the only supported public version until tagged releases are
published.

## Reporting a Vulnerability

Do not open a public GitHub issue for suspected vulnerabilities.

Send a private report to:

- contact@triviatec.com

Include:

- affected repository and commit SHA;
- reproducible steps;
- expected impact;
- logs or proof-of-concept files that do not contain real secrets or real client
  data.

Triviatec will acknowledge legitimate reports as soon as practical, triage the
impact, and coordinate disclosure before publishing details.

## Public Data Rules

- Do not upload production secrets, generated private keys, tokens, certificates,
  or customer credentials.
- Do not upload real personal, health, financial, customer, or production data.
- Do not send plaintext client business data to hosted Calcypher computation
  endpoints.
- Use synthetic examples only.

## Cryptography Scope

OpenCalcypher exposes local artifact workflows: context creation, key
generation, encryption, decryption, inspection, and validation.

Reports or pull requests that attempt to add hosted computation, SQL/CQL,
enterprise deployment code, private infrastructure, or proprietary Calcypher
engine internals are out of scope for this public repository.
