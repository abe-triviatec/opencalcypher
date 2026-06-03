# FHE Artifact Format

OpenCalcypher artifacts are explicit JSON wrappers around FHE context, key, and
ciphertext material.

The first ciphertext artifact version is:

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

Only homomorphic ciphertexts created for the same context and key are
compatible. Generic encrypted blobs from SSH, GPG, TLS, age, or OpenSSL are not
homomorphic ciphertexts.
