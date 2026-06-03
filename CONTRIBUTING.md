# Contributing

OpenCalcypher welcomes focused issues and pull requests inside the public
artifact-tooling scope.

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

Direct edits to generated files must not be merged unless the same change has
been applied to the private source of truth and regenerated through the export
pipeline.

## Pull Request Checklist

Before opening a PR:

- Keep the change inside OpenCalcypher scope.
- Do not include secrets, credentials, private URLs, datasets, or generated keys.
- Do not add encrypted computation commands.
- Update README or docs when changing CLI behavior.
- Add tests for behavior changes.
- Keep marketing claims factual and limited to behavior available in this public
  repository.
- Accept that maintainers may close PRs that conflict with product boundaries,
  IP protection, security posture, or repository-generation rules.

## Artifact Policy

Do not commit release binaries, generated keys, generated ciphertexts, or large
build outputs unless a maintainer explicitly asks for a small deterministic test
fixture.

## Maintainer Merge Policy

- Only Triviatec maintainers may merge into `main`.
- Require a passing CI run before merge.
- Require CODEOWNERS review for every pull request.
- Prefer squash merge for accepted external contributions.
- Do not merge direct public-repo fixes that have not been reproduced in the
  private source of truth.
- Do not merge changes that expose proprietary engine internals, private
  infrastructure, non-public customer context, or unsupported product claims.
