# Third-Party Notices

This distribution includes or interoperates with third-party open source
software. License texts below are provided for attribution and redistribution
compliance. They do not change the license of Calcypher Community or
OpenCalcypher source files.

## OpenFHE

OpenCalcypher is built on OpenFHE for fully homomorphic encryption primitives.
Calcypher Community local demos are designed to interoperate with compatible
OpenFHE/Calcypher ciphertext artifacts.

Project: https://github.com/openfheorg/openfhe-development

License: BSD 2-Clause License

```text
BSD 2-Clause License

Copyright (c) 2014-2022, NJIT, Duality Technologies Inc. and other
contributors

All rights reserved.

Author TPOC: contact@openfhe.org

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

## OpenSSL

OpenCalcypher uses OpenSSL libcrypto for SHA-256 hashing.

Project: https://www.openssl.org/

License: Apache License 2.0 for OpenSSL 3.x.

## CMake

OpenCalcypher source builds are driven by CMake.

Project: https://cmake.org/

License: BSD 3-Clause License.

## Python

OpenCalcypher Web uses the Python standard library for its local web helper.

Project: https://www.python.org/

License: Python Software Foundation License.

## Node.js Runtime Dependencies

Calcypher Community CLI and local runtime use Node.js packages, including
Express, `@grpc/grpc-js`, and `@grpc/proto-loader`. Those packages are not
vendored into this generated source tree. When building or distributing a
runtime image, keep the dependency license metadata from `package-lock.json`
and the generated container SBOM alongside the image release.

Primary projects:

- Express: https://expressjs.com/
- grpc-node: https://github.com/grpc/grpc-node
- Node.js: https://nodejs.org/

Common licenses in the runtime dependency tree include MIT, Apache-2.0, BSD,
and ISC.
