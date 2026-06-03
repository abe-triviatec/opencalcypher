# Contributing

OpenCalcypher welcomes issues and pull requests.

## Scope

Good contributions:

- CLI usability improvements.
- FHE context, key, encrypt, decrypt, inspect, and verify workflows.
- Artifact schema documentation.
- Compatibility tests.
- Build and release automation.
- Examples that do not include real personal, customer, health, financial, or production data.

Out of scope for this repository:

- encrypted add or multiply;
- SQL or CQL;
- analytics;
- table storage;
- hosted services;
- production deployment automation;
- Calcypher Pro or Enterprise code.

## Source of Truth

This public repository is generated from Triviatec's private source of truth.

Public pull requests are still useful. Maintainers will review them, reproduce
accepted changes in the private source tree, run the private/export test suite,
and regenerate this repository.

This process prevents generated public repositories from drifting apart while
still allowing external contribution and review.

## Pull Request Checklist

Before opening a PR:

- Keep the change inside OpenCalcypher scope.
- Do not include secrets, credentials, private URLs, datasets, or generated keys.
- Do not add encrypted computation commands.
- Update README or docs when changing CLI behavior.
- Add tests for behavior changes.

## Artifact Policy

Do not commit release binaries, generated keys, generated ciphertexts, or large
build outputs unless a maintainer explicitly asks for a small deterministic test
fixture.
