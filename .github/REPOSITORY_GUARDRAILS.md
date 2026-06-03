# Repository Guardrails

This repository is public and generated from Triviatec's private source of
truth. Configure GitHub settings to preserve that boundary.

## Required GitHub Settings

- Disable direct pushes to `main` except maintainers.
- Require pull requests before merging.
- Require CODEOWNERS review.
- Require status checks to pass before merge.
- Require branches to be up to date before merge.
- Require signed commits when practical.
- Disable force pushes on `main`.
- Disable branch deletion on `main`.
- Use squash merge as the default merge method.
- Disable merge commits.
- Disable rebase merges unless explicitly needed.
- Enable Dependabot alerts.
- Enable secret scanning and push protection if available for the account.
- Enable private vulnerability reporting if available for the repository.

## Merge Rules

- Do not merge public-generated-file edits directly.
- Reproduce accepted changes in the private source tree.
- Run private/export checks.
- Regenerate this repository.
- Merge only the regenerated diff.

## Business Boundaries

Reject changes that expose or imply:

- proprietary Calcypher engine internals;
- hosted computation paths;
- SQL/CQL;
- analytics or production deployment features;
- private infrastructure;
- non-public customer context;
- unsupported performance, compliance, or certification claims.

