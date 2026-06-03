#!/usr/bin/env bash
set -euo pipefail

opencalcypher context --scheme bfv --security 128 --depth 2 --batch-size 4096 --out context.json
opencalcypher keygen --context context.json --out keys/
opencalcypher encrypt-int --context context.json --public-key keys/public.key --value 42 --out value.ct.json
opencalcypher inspect value.ct.json
opencalcypher decrypt-int --context context.json --secret-key keys/secret.key --in value.ct.json
